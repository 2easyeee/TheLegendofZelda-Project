#include "Shader.h"

CShader::CShader(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CComponent { _pDevice, _pDeviceContext }
{
}

CShader::CShader(const CShader& _Prototype)
    :CComponent ( _Prototype )
    , m_pEffect { _Prototype.m_pEffect }
    , m_iNumPasses { _Prototype.m_iNumPasses }
    , m_vecInputLayouts { _Prototype.m_vecInputLayouts }
{
    for (auto& pInputLayout : m_vecInputLayouts)
        Safe_AddRef(pInputLayout);

    Safe_AddRef(m_pEffect);
}

HRESULT CShader::Initialize_Prototype(const _tchar* _pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* _pElemnets, _uint _iNumElements)
{
    /* 0. Select a HLSL Compile Option */
    _uint iHLSLFlag = {};
#ifdef _DEBUG
    iHLSLFlag |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    iHLSLFlag |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif // _DEBUG

    /* 1. Effect 객체를 compile 한다 */
    ID3DBlob* pBuffer = {};
    if (FAILED(D3DX11CompileEffectFromFile(_pShaderFilePath, nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE, iHLSLFlag, 0,
        m_pDevice, &m_pEffect, nullptr)))
        return E_FAIL;

    /* 2. .hlsl 안에 있는 0번째의 Technique 가져오기 (pass0) */
    ID3DX11EffectTechnique* pTechnique = m_pEffect->GetTechniqueByIndex(0);
    if (nullptr == pTechnique)
        return E_FAIL;

    /* 3. Pass 별, InputLayout 생성하기 */
    D3DX11_TECHNIQUE_DESC TechniqueDesc = {};
    pTechnique->GetDesc(&TechniqueDesc);
    m_iNumPasses = TechniqueDesc.Passes;

    m_vecInputLayouts.reserve(m_iNumPasses);
    for (size_t i = 0; i < m_iNumPasses; i++)
    {
        ID3DX11EffectPass* pPass = pTechnique->GetPassByIndex(i);
        ID3D11InputLayout* pInputLayout = { nullptr };

        D3DX11_PASS_DESC PassDesc = {};
        pPass->GetDesc(&PassDesc);
        if (FAILED(m_pDevice->CreateInputLayout(_pElemnets, _iNumElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayout)))
            return E_FAIL;

        m_vecInputLayouts.push_back(pInputLayout);
    }

    return S_OK;
}

HRESULT CShader::Initialize(void* _pArg)
{
    return S_OK;
}

HRESULT CShader::Begin(_uint _iPassIndex)
{
    /* 0. Shader 에 포함된 각 Pass 가 사용할 InputLayout 개수 확인 */
    if (_iPassIndex >= m_vecInputLayouts.size())
        return E_FAIL;

    /* 1. InputLayout 바인딩 */
    m_pDeviceContext->IASetInputLayout(m_vecInputLayouts[_iPassIndex]);

    /* 2. Effect 에서 Pass 얻어오기 */
    ID3DX11EffectPass* pPass = m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(_iPassIndex);
    if (nullptr == pPass)
        return E_FAIL;

    /* 3. Pass 적용*/
    pPass->Apply(0, m_pDeviceContext);

    return S_OK;
}

HRESULT CShader::Bind_RawValue(const _char* _pConstantName, const void* _pValue, _uint _iLength)
{
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(_pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    return pVariable->SetRawValue(_pValue, 0, _iLength);
}

HRESULT CShader::Bind_Matrix(const _char* _pConstantName, const _float4x4* _pMatrix)
{
    /* 0. HLSL 의 변수(ex. g_WorldMatrix, g_ViewMatrix, g_ProjMatrix..) 찾기 */
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(_pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    /* 1. Matrix 타입으로 캐스팅 */
    // ID3DX11EffectMatrixVariable : 모든 타입(ex. Vector, Matrix, Struct, SRV 등) 의 부모 인터페이스
    ID3DX11EffectMatrixVariable* pMatrixVariable = pVariable->AsMatrix();
    if (nullptr == pMatrixVariable)
        return E_FAIL;

    /* 2. 행렬 -> GPU 에게 전달 */
    return pMatrixVariable->SetMatrix(reinterpret_cast<const _float*>(_pMatrix));
}

HRESULT CShader::Bind_Matrices(const _char* _pConstantName, const _float4x4* _pMatrix, _uint _iNumMatrices)
{
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(_pConstantName);
    if (nullptr == pVariable) 
        return E_FAIL;

    ID3DX11EffectMatrixVariable* pMatrixVariable = pVariable->AsMatrix(); 
    if (nullptr == pMatrixVariable) 
        return E_FAIL;

    return pMatrixVariable->SetMatrixArray(reinterpret_cast<const _float*>(_pMatrix), 0, _iNumMatrices);
}

HRESULT CShader::Bind_ShaderResourceView(const _char* _pConstantName, ID3D11ShaderResourceView* _pSRV)
{
    /* 0. HLSL 의 텍스쳐 변수(ex. g_DiffuseTexture ..) 찾기 */
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(_pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    /* 1. ShaderResource 타입으로 캐스팅 */
    ID3DX11EffectShaderResourceVariable* pSRVVariable = pVariable->AsShaderResource();
    if (nullptr == pSRVVariable)
        return E_FAIL;

    /* 2. SRV 바인딩 */
    return pSRVVariable->SetResource(_pSRV);
}

CShader* CShader::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _tchar* _pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* _pElements, _uint _iNumElements)
{
    CShader* pInstance = new CShader(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype(_pShaderFilePath, _pElements, _iNumElements)))
    {
        MSG_BOX("FAILED TO CREATED : CShader");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CShader::Clone(void* _pArg)
{
    CShader* pInstance = new CShader(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CShader");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CShader::Free()
{
    __super::Free();

    /* vector release */
    for (auto& pInputLayout : m_vecInputLayouts)
        Safe_Release(pInputLayout);
    m_vecInputLayouts.clear();

    Safe_Release(m_pEffect);
}
