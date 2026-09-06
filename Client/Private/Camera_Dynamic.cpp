#include "Camera_Dynamic.h"
#include "GameInstance.h"

CCamera_Dynamic::CCamera_Dynamic(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CCamera { _pDevice, _pDeviceContext }
{
}

CCamera_Dynamic::CCamera_Dynamic(const CCamera_Dynamic& _Prototype)
    : CCamera (_Prototype)
{
}

HRESULT CCamera_Dynamic::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCamera_Dynamic::Initialize(void* _pArg)
{
    if (FAILED(CCamera::Initialize(_pArg)))
        return E_FAIL;

    return S_OK;
}

void CCamera_Dynamic::Priority_Update(_float _fTimeDelta)
{
    /* KeyInput */
    if (m_pGameInstance->Get_DIKeyPressing(DIK_UP))
    {
        m_pTransformCom->Go_Straight(_fTimeDelta);
    }
    if (m_pGameInstance->Get_DIKeyPressing(DIK_DOWN))
    {
        m_pTransformCom->Go_Backward(_fTimeDelta);
    }
    if (m_pGameInstance->Get_DIKeyPressing(DIK_LEFT))
    {
        m_pTransformCom->Go_Left(_fTimeDelta);
    }
    if (m_pGameInstance->Get_DIKeyPressing(DIK_RIGHT))
    {
        m_pTransformCom->Go_Right(_fTimeDelta);
    }

    /* MouseInput */
    if (m_bMouseEnable)
    {
        _long MouseMove = {};

        if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMM::X))
        {
            m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f),
                (_fTimeDelta * MouseMove * m_fMouseSensor));
        }
        if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMM::Y))
        {
            m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::RIGHT),
                (_fTimeDelta * MouseMove * m_fMouseSensor));
        }
    }

    /* PipeLine */
    __super::Update_PipeLines();
}

void CCamera_Dynamic::Update(_float _fTimeDelta)
{
    /* Mouse */
    Mouse_Enable();
}

void CCamera_Dynamic::Late_Update(_float _fTimeDelta)
{
}

HRESULT CCamera_Dynamic::Render()
{
    return S_OK;
}

HRESULT CCamera_Dynamic::Ready_Components()
{
    return S_OK;
}

void CCamera_Dynamic::Mouse_Enable()
{
    if (m_pGameInstance->Get_DIKeyDown(DIK_RSHIFT))
    {
        m_bMouseEnable = !m_bMouseEnable;
    }
}

CCamera_Dynamic* CCamera_Dynamic::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CCamera_Dynamic* pInstance = new CCamera_Dynamic(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CCamera_Dynamic(Free)");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CCamera_Dynamic::Clone(void* _pArg)
{
    CCamera_Dynamic* pInstance = new CCamera_Dynamic(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CCamera_Dynamic(Free)");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCamera_Dynamic::Free()
{
    __super::Free();
}
