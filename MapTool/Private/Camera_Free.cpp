#include "Camera_Free.h"
#include "GameInstance.h"

CCamera_Free::CCamera_Free(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CCamera { _pDevice, _pDeviceContext }
{
}

CCamera_Free::CCamera_Free(const CCamera_Free& _Prototype)
    : CCamera (_Prototype)
{
}

HRESULT CCamera_Free::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCamera_Free::Initialize(void* _pArg)
{
    if (FAILED(CCamera::Initialize(_pArg)))
        return E_FAIL;

    return S_OK;
}

void CCamera_Free::Priority_Update(_float _fTimeDelta)
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
    _long MouseMove = {};

    if (m_bMouseEnable)
    {
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

void CCamera_Free::Update(_float _fTimeDelta)
{
    Mouse_Enable();
}

void CCamera_Free::Late_Update(_float _fTimeDelta)
{
}

HRESULT CCamera_Free::Render()
{
    return S_OK;
}

HRESULT CCamera_Free::Ready_Components()
{
    return S_OK;
}

void CCamera_Free::Mouse_Enable()
{
    if (m_pGameInstance->Get_DIKeyDown(DIK_RSHIFT))
    {
        m_bMouseEnable = !m_bMouseEnable;
    }
}

CCamera_Free* CCamera_Free::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CCamera_Free* pInstance = new CCamera_Free(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CCamera_Free");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CCamera_Free::Clone(void* _pArg)
{
    CCamera_Free* pInstance = new CCamera_Free(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CCamera_Free");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCamera_Free::Free()
{
    __super::Free();
}
