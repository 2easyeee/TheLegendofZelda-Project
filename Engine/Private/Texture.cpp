#include "Texture.h"
#include "GameInstance.h"

CTexture::CTexture(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CComponent { _pDevice, _pDeviceContext }
{
}

CTexture::CTexture(const CTexture& _Prototype)
    : CComponent (_Prototype)
    , m_iNumTexture { _Prototype.m_iNumTexture }
    , m_vecTextures { _Prototype.m_vecTextures }
{
    for (auto& pSRV : m_vecTextures)
        Safe_AddRef(pSRV);
}

HRESULT CTexture::Initialize_Prototype(const _tchar* _pTextureFilePath, _uint _iNumTextures)
{
    m_iNumTexture = _iNumTextures;

    for (size_t i = 0; i < m_iNumTexture; i++)
    {
        ID3D11ShaderResourceView* pSRV = { nullptr };
        _tchar szEXT[MAX_PATH] = {};

        _wsplitpath_s(_pTextureFilePath, 
            nullptr, 0, nullptr, 0,
            nullptr, 0, szEXT, MAX_PATH);

        _tchar szFullPath[MAX_PATH] = {};
        wsprintf(szFullPath, _pTextureFilePath, i);

        HRESULT hr = {};

        if (false == lstrcmp(szEXT, TEXT(".dds")))
        {
            hr = CreateDDSTextureFromFile(m_pDevice, szFullPath, nullptr, &pSRV);
        }
        else if (false == lstrcmp(szEXT, TEXT(".tga")))
        {
            return E_FAIL;
        }
        else
        {
            hr = CreateWICTextureFromFile(m_pDevice, szFullPath, nullptr, &pSRV);
        }

        if (FAILED(hr))
            return E_FAIL;

        m_vecTextures.push_back(pSRV);
    }
    return S_OK;
}

HRESULT CTexture::Initialize(void* _pArg)
{
    return S_OK;
}

HRESULT CTexture::Bind_ShaderResourceView(CShader* _pShader, const _char* _pConstantName, _uint _iTextureIndex)
{
    if (_iTextureIndex >= m_iNumTexture)
        return E_FAIL;

    return _pShader->Bind_ShaderResourceView(_pConstantName, m_vecTextures[_iTextureIndex]);
}

CTexture* CTexture::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _tchar* _pTextureFilePath, _uint _iNumTextures)
{
    CTexture* pInstance = new CTexture(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype(_pTextureFilePath, _iNumTextures)))
    {
        MSG_BOX("FAILED TO CREATED : CTexture");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CTexture::Clone(void* _pArg)
{
    CTexture* pInstanace = new CTexture(*this);
    if (FAILED(pInstanace->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CTexture");
        Safe_Release(pInstanace);
    }
    return pInstanace;
}

void CTexture::Free()
{
    __super::Free();

    /* vector release */
    for (auto& pSRV : m_vecTextures)
        Safe_Release(pSRV);
    m_vecTextures.clear();
}
