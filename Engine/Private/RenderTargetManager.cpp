#include "RenderTargetManager.h"
#include "RenderTarget.h"
#include "GameInstance.h"

CRenderTargetManager::CRenderTargetManager(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : m_pDevice { _pDevice }
    , m_pDeviceContext { _pDeviceContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pDeviceContext);
}

HRESULT CRenderTargetManager::Add_RenderTarget(const _wstring& _TargetTag, _uint _iWidth, _uint _iHeight, DXGI_FORMAT _eFormat, const _float4& _vClearColor)
{
    if (nullptr != Find_RenderTarget(_TargetTag))
        return E_FAIL;

    CRenderTarget* pRenderTarget = CRenderTarget::Create(m_pDevice, m_pDeviceContext, _iWidth, _iHeight, _eFormat, _vClearColor);
    if (nullptr == pRenderTarget)
        return E_FAIL;

    m_RenderTargets.emplace(_TargetTag, pRenderTarget);

    return S_OK;
}

HRESULT CRenderTargetManager::Add_MRT(const _wstring& _MRTTag, const _wstring& _TargetTag)
{
    CRenderTarget* pRenderTarget = Find_RenderTarget(_TargetTag);
    if (nullptr == pRenderTarget)
        return E_FAIL;

    list<CRenderTarget*>* pMRTList = Find_MRT(_MRTTag);
    if (nullptr == pMRTList)
    {
        list<CRenderTarget*> MRTList;
        MRTList.push_back(pRenderTarget);
        m_MRTs.emplace(_MRTTag, MRTList);
    }
    else
    {
        pMRTList->push_back(pRenderTarget);
    }

    Safe_AddRef(pRenderTarget);

    return S_OK;
}

HRESULT CRenderTargetManager::Begin_MRT(const _wstring& _MRTTag)
{
    list<CRenderTarget*>* pMRTList = Find_MRT(_MRTTag);
    if (nullptr == pMRTList || m_iMRTStackDepth >= MRT_STACK_CAPACITY)
        return E_FAIL;

    ID3D11ShaderResourceView* pNullSRVs[D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
    m_pDeviceContext->PSSetShaderResources(0, D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pNullSRVs);

    ID3D11RenderTargetView*& pPreviousRTV = m_pRenderTargetStack[m_iMRTStackDepth];
    ID3D11DepthStencilView*& pPreviousDSV = m_pDepthStencilStack[m_iMRTStackDepth];
    m_pDeviceContext->OMGetRenderTargets(1, &pPreviousRTV, &pPreviousDSV);
    ++m_iMRTStackDepth;

    ID3D11RenderTargetView* pRTVs[8] = { nullptr };
    _uint iNumViewports = {};
    for (auto& pRenderTarget : *pMRTList)
    {
        pRenderTarget->Clear();
        pRTVs[iNumViewports++] = pRenderTarget->Get_RTV();
    }

    m_pDeviceContext->OMSetRenderTargets(iNumViewports, pRTVs, pPreviousDSV);

    return S_OK;
}

HRESULT CRenderTargetManager::Begin_MRT(const _wstring& _MRTTag, ID3D11DepthStencilView* _pDSV)
{
    list<CRenderTarget*>* pMRTList = Find_MRT(_MRTTag);
    if (nullptr == pMRTList || m_iMRTStackDepth >= MRT_STACK_CAPACITY)
        return E_FAIL;

    ID3D11ShaderResourceView* pNullSRVs[D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
    m_pDeviceContext->PSSetShaderResources(0, D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pNullSRVs);

    ID3D11RenderTargetView*& pPreviousRTV = m_pRenderTargetStack[m_iMRTStackDepth];
    ID3D11DepthStencilView*& pPreviousDSV = m_pDepthStencilStack[m_iMRTStackDepth];
    m_pDeviceContext->OMGetRenderTargets(1, &pPreviousRTV, &pPreviousDSV);
    ++m_iMRTStackDepth;

    ID3D11RenderTargetView* pRTVs[8] = { nullptr };
    _uint iNumViewports = {};
    for (auto& pRenderTarget : *pMRTList)
    {
        pRenderTarget->Clear();
        pRTVs[iNumViewports++] = pRenderTarget->Get_RTV();
    }

    ID3D11DepthStencilView* pBindDSV = (nullptr != _pDSV) ? _pDSV : pPreviousDSV;
    m_pDeviceContext->OMSetRenderTargets(iNumViewports, pRTVs, pBindDSV);

    return S_OK;
}

HRESULT CRenderTargetManager::End_MRT()
{
    if (0 == m_iMRTStackDepth)
        return E_FAIL;

    --m_iMRTStackDepth;

    ID3D11RenderTargetView*& pPreviousRTV = m_pRenderTargetStack[m_iMRTStackDepth];
    ID3D11DepthStencilView*& pPreviousDSV = m_pDepthStencilStack[m_iMRTStackDepth];
    ID3D11ShaderResourceView* pNullSRVs[D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
    m_pDeviceContext->PSSetShaderResources(0, D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pNullSRVs);
    m_pDeviceContext->OMSetRenderTargets(1, &pPreviousRTV, pPreviousDSV);

    Safe_Release(pPreviousRTV);
    Safe_Release(pPreviousDSV);

    return S_OK;
}

HRESULT CRenderTargetManager::Bind_RT_SRV(const _wstring& _TargetTag, CShader* _pShader, const _char* _pConstantName)
{
    CRenderTarget* pRenderTarget = Find_RenderTarget(_TargetTag);
    if (nullptr == pRenderTarget)
        return E_FAIL;

    return pRenderTarget->Bind_SRV(_pShader, _pConstantName);
}

CRenderTarget* CRenderTargetManager::Find_RenderTarget(const _wstring& _TargetTag)
{
    auto iter = m_RenderTargets.find(_TargetTag);
    if (iter == m_RenderTargets.end())
        return nullptr;
    return iter->second;
}

list<class CRenderTarget*>* CRenderTargetManager::Find_MRT(const _wstring& _MRTTag)
{
    auto iter = m_MRTs.find(_MRTTag);
    if (iter == m_MRTs.end())
        return nullptr;
    return &iter->second;
}

CRenderTargetManager* CRenderTargetManager::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    return new CRenderTargetManager(_pDevice, _pDeviceContext);
}

void CRenderTargetManager::Free()
{
    __super::Free();

    while (0 < m_iMRTStackDepth)
    {
        --m_iMRTStackDepth;
        Safe_Release(m_pRenderTargetStack[m_iMRTStackDepth]);
        Safe_Release(m_pDepthStencilStack[m_iMRTStackDepth]);
    }

    for (auto& Pair : m_RenderTargets)
        Safe_Release(Pair.second);

    for (auto& Pair : m_MRTs)
    {
        for (auto& pRenderTarget : Pair.second)
            Safe_Release(pRenderTarget);
        Pair.second.clear();
    }
    m_MRTs.clear();

    Safe_Release(m_pDevice);
    Safe_Release(m_pDeviceContext);
}

#ifdef  _DEBUG
HRESULT CRenderTargetManager::Ready_RT_Debug(const _wstring& _TargetTag, _float _fX, _float _fY, _float _fSizeX, _float _fSizeY)
{
    CRenderTarget* pRenderTarget = Find_RenderTarget(_TargetTag);
    if (nullptr == pRenderTarget)
        return E_FAIL;

    return pRenderTarget->Ready_RT_Debug(_fX, _fY, _fSizeX, _fSizeY);
}

HRESULT CRenderTargetManager::Render_MRT(const _wstring& _MRTTag, CShader* _pShader, CVIBuffer_Rect* _pVIBuffer)
{
    list<class CRenderTarget*>* pMRTList = Find_MRT(_MRTTag);
    if (nullptr == pMRTList)
        return E_FAIL;

    if (FAILED(_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    for (auto& pRenderTarget : *pMRTList)
    {
        pRenderTarget->Render(_pShader, _pVIBuffer);
    }

    return S_OK;
}
#endif //  _DEBUG
