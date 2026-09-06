#include "GameObject/Body_Player.h"
#include "GameInstance.h"
#include "GameObject/Player.h"

CBody_Player::CBody_Player(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CBody{ _pDevice, _pDeviceContext }
{
}

CBody_Player::CBody_Player(const CBody_Player& _Prototype)
    : CBody(_Prototype)
{
}

HRESULT CBody_Player::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBody_Player::Initialize(void* _pArg)
{
    /* 1. Body */
    if (FAILED(CBody::Initialize(_pArg)))
        return E_FAIL;

    /* 6. Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

    /* Init Equip */
    m_pModelCom->Set_MeshActive("linkSwordA_MI_sowrdA", false);
    m_pModelCom->Set_MeshActive("linkSwordA_MI_sowrdAball", false);
    m_pModelCom->Set_MeshActive("linkShieldA_MI_shieldA", false);

    return S_OK;
}

void CBody_Player::Priority_Update(_float _fTimeDelta)
{
    CBody::Priority_Update(_fTimeDelta);
}

void CBody_Player::Update(_float _fTimeDelta)
{
    CBody::Update(_fTimeDelta);

    /* 6. Collider (Socket) */
    if (FAILED(Attach_Socket()))
        return;
}

void CBody_Player::Late_Update(_float _fTimeDelta)
{
    CBody::Late_Update(_fTimeDelta);
}

HRESULT CBody_Player::Render()
{
    if (FAILED(CBody::Render()))
        return E_FAIL;

#ifdef _DEBUG
    //if (m_pSwordCollider) m_pSwordCollider->Render();
    //if (m_pShieldCollider) m_pShieldCollider->Render();
#endif // _DEBUG

    return S_OK;
}

void CBody_Player::Collider_SwordEnable(_bool _bActive)
{
    if (m_pSwordCollider)
        m_pSwordCollider->Set_Active(_bActive);
}

void CBody_Player::Collider_ShiledEnable(_bool _bActive)
{
    if (m_pShieldCollider)
        m_pShieldCollider->Set_Active(_bActive);
}

void CBody_Player::Set_Owner_Parent(CGameObject* _pOwner)
{
    if (m_pSwordCollider)
    {
        m_pSwordCollider->Set_Owner(_pOwner);
        m_pGameInstance->Register_Collider(m_pSwordCollider);
    }

    if (m_pShieldCollider)
    {
        m_pShieldCollider->Set_Owner(_pOwner);
        m_pGameInstance->Register_Collider(m_pShieldCollider);
    }
}

HRESULT CBody_Player::Ready_Collider()
{
    /* Sword */
    CBounding_OBB::OBB_DESC tOBBDesc = {};
    tOBBDesc.vAngles = _float3(0.f, 0.f, 0.f);

    tOBBDesc.vExtents = _float3(0.08f, 0.08f, 0.55f);
    tOBBDesc.vCenter = _float3(0.f, 0.f, -0.3f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_OBB"),
        TEXT("Com_SwordCollider"),
        reinterpret_cast<CComponent**>(&m_pSwordCollider),
        &tOBBDesc)))
        return E_FAIL;

    m_pSwordCollider->Set_Group(GROUP::WEAPON_PLAYER);
    m_pSwordCollider->Set_Active(false);

    /* Shield */
    tOBBDesc.vExtents = _float3(0.3f + 0.5f, 0.07f, 0.45f + 0.5f);
    tOBBDesc.vCenter = _float3(0.1f, -0.1f, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_OBB"),
        TEXT("Com_ShieldCollider"),
        reinterpret_cast<CComponent**>(&m_pShieldCollider),
        &tOBBDesc)))
        return E_FAIL;

    m_pShieldCollider->Set_Group(GROUP::SHIELD_PLAYER);
    m_pShieldCollider->Set_Active(false);

    return S_OK;
}

HRESULT CBody_Player::Attach_Socket()
{
    if (!m_pModelCom)
        return S_FALSE;

    if (!m_pParentMatrix)
        return S_FALSE;

    _matrix WorldMatrix = XMLoadFloat4x4(m_pParentMatrix);

    /* Sword : itemA_R */
    if (m_pSwordCollider)
    {
        const _float4x4* pBone = m_pModelCom->Get_BoneMatrixPtr("itemA_L");
        
        if (pBone)
        {
            _matrix Bone = XMLoadFloat4x4(pBone);
            _matrix Final = Bone * WorldMatrix;
            m_pSwordCollider->Update(Final);
        }
    }

    /* Shield : itemA_L */
    if (m_pShieldCollider)
    {
        const _float4x4* pBone = m_pModelCom->Get_BoneMatrixPtr("itemA_R");

        if (pBone)
        {
            _matrix Bone = XMLoadFloat4x4(pBone);
            _matrix Final = Bone * WorldMatrix;
            m_pShieldCollider->Update(Final);
        }
    }

    return S_OK;
}

CBody_Player* CBody_Player::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CBody_Player* pInstance = new CBody_Player(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CBody_Player");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CBody_Player::Clone(void* _pArg)
{
    CBody_Player* pInstance = new CBody_Player(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CBody_Player");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBody_Player::Free()
{
    __super::Free();

    Safe_Release(m_pSwordCollider);
    Safe_Release(m_pShieldCollider);
}
