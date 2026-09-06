#include "RenderTarget.h"
#include "GameInstance.h"

CRenderTarget::CRenderTarget(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : m_pDevice { _pDevice }
    , m_pDeviceContext { _pDeviceContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pDeviceContext);
}

HRESULT CRenderTarget::Initialize(_uint _iWidth, _uint _iHeight, DXGI_FORMAT _eFormat, const _float4& _vClearColor)
{
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = _iWidth;
    TextureDesc.Height = _iHeight;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = _eFormat;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;

    m_vClearColor = _vClearColor;

    if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
        return E_FAIL;

    if (FAILED(m_pDevice->CreateRenderTargetView(m_pTexture2D, nullptr, &m_pRTV)))
        return E_FAIL;

    if (FAILED(m_pDevice->CreateShaderResourceView(m_pTexture2D, nullptr, &m_pSRV)))
        return E_FAIL;

    return S_OK;
}

HRESULT CRenderTarget::Bind_SRV(CShader* _pShader, const _char* _pConstantName)
{
    return _pShader->Bind_ShaderResourceView(_pConstantName, m_pSRV);
}

void CRenderTarget::Clear()
{
    m_pDeviceContext->ClearRenderTargetView(m_pRTV, reinterpret_cast<const _float*>(&m_vClearColor));
}

ID3D11RenderTargetView* CRenderTarget::Get_RTV() const
{
    return m_pRTV;
}

CRenderTarget* CRenderTarget::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, _uint _iWidth, _uint _iHeight, DXGI_FORMAT _eFormat, const _float4& _vClearColor)
{
    CRenderTarget* pInstance = new CRenderTarget(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize(_iWidth, _iHeight, _eFormat, _vClearColor)))
    {
        MSG_BOX("FAILED TO CREATED : CRenderTarget");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRenderTarget::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pDeviceContext);

    Safe_Release(m_pSRV);
    Safe_Release(m_pRTV);
    Safe_Release(m_pTexture2D);
}

#ifdef _DEBUG
HRESULT CRenderTarget::Ready_RT_Debug(_float _fX, _float _fY, _float _fSizeX, _float _fSizeY)
{
    _uint iNumViewports = { 1 };
    D3D11_VIEWPORT ViewportDesc = {};

    m_pDeviceContext->RSGetViewports(&iNumViewports, &ViewportDesc);

    XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(_fSizeX, _fSizeY, 1.f));
    m_WorldMatrix._41 = _fX - ViewportDesc.Width * 0.5f;
    m_WorldMatrix._42 = -_fY + ViewportDesc.Height * 0.5f;

    return S_OK;
}

HRESULT CRenderTarget::Render(CShader* _pShader, CVIBuffer_Rect* _pVIBuffer)
{
    if (FAILED(_pShader->Bind_ShaderResourceView("g_Texture", m_pSRV)))
        return E_FAIL;

    if (FAILED(_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;

    if (FAILED(_pShader->Begin(0)))
        return E_FAIL;

    if (FAILED(_pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
}
#endif // _DEBUG