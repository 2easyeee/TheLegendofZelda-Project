#include "UIObject.h"
#include "GameInstance.h"

CUIObject::CUIObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CGameObject { _pDevice, _pDeviceContext }
{
}

CUIObject::CUIObject(const CUIObject& _Prototype)
    : CGameObject (_Prototype)
{
}

HRESULT CUIObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUIObject::Initialize(void* _pArg)
{ 
    UI_INIT_DESC* pInitDesc = static_cast<UI_INIT_DESC*>(_pArg);

    /* 0. CGameObject 의 Init */
    if (FAILED(CGameObject::Initialize(&pInitDesc->tObjectDesc)))
        return E_FAIL;

   
    /* 2. UI_DESC 전달 */
    if (&pInitDesc->tuiDesc)
    {
        if (FAILED(m_pTransformCom->Initialize(&pInitDesc->tuiDesc)))
            return E_FAIL;

        UI_DESC* pDesc = static_cast<UI_DESC*>(&pInitDesc->tuiDesc);
        m_fX = pDesc->fX;
        m_fY = pDesc->fY;
        m_fSizeX = pDesc->fSizeX;
        m_fSizeY = pDesc->fSizeY;
        m_eUILayer = pDesc->eUILayer;
        m_eUIInteraction = pDesc->eUIInteraction;
        m_eUIBlend = pDesc->eUIBlend;
    }

    Apply_UITransform();
    
    return S_OK;
}

void CUIObject::Priority_Update(_float _fTimeDelta)
{
}

void CUIObject::Update(_float _fTimeDelta)
{
}

void CUIObject::Late_Update(_float _fTimeDelta)
{
}

HRESULT CUIObject::Render()
{
    return S_OK;
}

HRESULT CUIObject::Bind_ViewMatrix(CShader* _pShader, const _char* _pConstantName)
{
    return _pShader->Bind_Matrix(_pConstantName, &m_ViewMatrix);
}

HRESULT CUIObject::Bind_ProjMatrix(CShader* _pShader, const _char* _pConstantName)
{
    return _pShader->Bind_Matrix(_pConstantName, &m_ProjMatrix);
}

void CUIObject::Apply_UITransform()
{
    /* 1. Viewport 설정 */
    D3D11_VIEWPORT ViewportDesc = {};
    _uint iNumViewports = { 1 };
    m_pDeviceContext->RSGetViewports(&iNumViewports, &ViewportDesc);
    XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.f, 1.f));

    /* 3. Transform 에 동기화 */
    m_pTransformCom->Set_Scale(m_fSizeX, m_fSizeY);
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_fX - ViewportDesc.Width * 0.5f, -m_fY + ViewportDesc.Height * 0.5f, 0.f, 1.f));
}

void CUIObject::Set_UIScale(_float _fSizeX, _float _fSizeY)
{
    m_fSizeX = _fSizeX;
    m_fSizeY = _fSizeY;
}

_bool CUIObject::IsPicked(HWND hWnd)
{
    RECT rcUI = {
        m_fX - (m_fSizeX * 0.5f),
        m_fY - (m_fSizeY * 0.5f),
        m_fX + (m_fSizeX * 0.5f),
        m_fY + (m_fSizeY * 0.5f)};
    POINT ptMouse;

    GetCursorPos(&ptMouse);
    ScreenToClient(hWnd, &ptMouse);

    return PtInRect(&rcUI, ptMouse);
}

void CUIObject::Free()
{
    __super::Free();
}
