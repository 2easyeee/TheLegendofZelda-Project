#include "GameObject/Weapon_MoriblinSpear.h"
#include "GameInstance.h"
#include "GameObject/Spear.h"

CWeapon_MoriblinSpear::CWeapon_MoriblinSpear(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CPartObject{ _pDevice, _pDeviceContext }
{
}


CWeapon_MoriblinSpear::CWeapon_MoriblinSpear(const CWeapon_MoriblinSpear& _Prototype)
    : CPartObject(_Prototype)
{
}

HRESULT CWeapon_MoriblinSpear::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CWeapon_MoriblinSpear::Initialize(void* _pArg)
{
    /* 1. PartObject */
    PART_CREATE_DESC* tInitDesc = static_cast<PART_CREATE_DESC*>(_pArg);
    if (FAILED(CPartObject::Initialize(tInitDesc)))
        return E_FAIL;

    if (&tInitDesc->tPartDesc)
    {
        m_pParentState = tInitDesc->tPartDesc.pPartentSTATE;
        m_pSocketMatrix = tInitDesc->tPartDesc.pSocketMatrix;
    }

    /* Add Prototype (Spear) */
    /* Client (Need), MapTool (Need X) */
    CModel* pModel = CModel::Create(m_pDevice, m_pDeviceContext,
        "../../Resources/Models/NonAnim/Weapon_MoriblinSpear/MoriblinSpearWeapon.mesh");
    if (!pModel)
        return E_FAIL;

    if (FAILED(m_pGameInstance->Add_Prototype(
        m_ObjectDesc.iLevel,
        TEXT("Prototype_Component_Model_Map_MoriblinSpearWeapon"),
        pModel)))
    {
        Safe_Release(pModel);
        //MSG_BOX("[WARNING] FILED TO REGISTER MODEL : CWeapon_MoriblinSpear(Spear Object)");
    }

    Respawn();

    return S_OK;
}

void CWeapon_MoriblinSpear::Priority_Update(_float _fTimeDelta)
{
}

void CWeapon_MoriblinSpear::Update(_float _fTimeDelta)
{
    if (m_eWeaponState != WEAPON_STATE::ATTACHED)
        return;

    /* 1. SocketMatrix */
    _matrix WorldMatrix = XMMatrixIdentity();
    _matrix SocketMatrix = XMLoadFloat4x4(m_pSocketMatrix);
    for (size_t i = 0; i < 3; i++)
    {
        SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);
    }
    WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * SocketMatrix;

    /* 2. Update WorldMatrix */
    XMStoreFloat4x4(&m_CombinedWorldMatrix, WorldMatrix * XMLoadFloat4x4(m_pParentMatrix));
    /* РќДо */
    if (m_pSpear)
        m_pSpear->Update_WorldMatrix(m_CombinedWorldMatrix);
}

void CWeapon_MoriblinSpear::Late_Update(_float _fTimeDelta)
{
    CPartObject::Late_Update(_fTimeDelta);
}

HRESULT CWeapon_MoriblinSpear::Render()
{
    return S_OK;
}

void CWeapon_MoriblinSpear::Throw()
{
    if (m_eWeaponState != WEAPON_STATE::ATTACHED)
        return;

    if (!m_pSpear)
        return;

    /* Fly */
    m_pSpear->Fly();

    m_pSpear = nullptr;
}

void CWeapon_MoriblinSpear::Throw_Target(_vector _vTargetPos)
{
    if (m_eWeaponState != WEAPON_STATE::ATTACHED)
        return;

    if (!m_pSpear)
        return;

    /* Fly */
    m_pSpear->Fly_Target(_vTargetPos);

    m_pSpear = nullptr;
}

void CWeapon_MoriblinSpear::Respawn()
{
    /* 1. Spear */
    CSpear::SPEAR_INIT_DESC tDesc = {};

    CGameObject::OBJECT_DESC tObjectDesc = {};
    SET_DESC(tObjectDesc.ObjectID, TEXT("Spear"));
    tObjectDesc.iLevel = m_ObjectDesc.iLevel;
    SET_DESC(tObjectDesc.LayerTag, m_ObjectDesc.LayerTag);
    SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Actor_Weapon_Spear"));
    SET_DESC(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxMesh"));
    SET_DESC(tObjectDesc.ModelTag, TEXT("Prototype_Component_Model_Map_MoriblinSpearWeapon"));
    tDesc.tObjectDesc = tObjectDesc;

    _matrix WorldMatrix = XMLoadFloat4x4(&m_CombinedWorldMatrix);
    for (_int i = 0; i < 3; ++i)
    {
        WorldMatrix.r[i] = XMVector3Normalize(WorldMatrix.r[i]);
    }
    XMStoreFloat4x4(&tDesc.tSpearDesc.WorldMatrix, WorldMatrix);

    CGameObject* pGameObject = { nullptr };
    m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        tDesc.tObjectDesc.ObjectTag,
        tDesc.tObjectDesc.iLevel,
        tDesc.tObjectDesc.LayerTag,
        &tDesc, &pGameObject);

    m_pSpear = static_cast<CSpear*>(pGameObject);

    m_eWeaponState = WEAPON_STATE::ATTACHED;
}

CWeapon_MoriblinSpear* CWeapon_MoriblinSpear::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CWeapon_MoriblinSpear* pInstance = new CWeapon_MoriblinSpear(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CWeapon_MoriblinSpear");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CWeapon_MoriblinSpear::Clone(void* _pArg)
{
    CWeapon_MoriblinSpear* pInstance = new CWeapon_MoriblinSpear(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CWeapon_MoriblinSpear");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CWeapon_MoriblinSpear::Free()
{
    __super::Free();
}
