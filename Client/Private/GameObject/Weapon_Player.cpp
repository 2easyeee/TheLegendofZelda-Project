#include "GameObject/Weapon_Player.h"
#include "GameInstance.h"
#include "GameObject/Boomerang.h"

CWeapon_Player::CWeapon_Player(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CPartObject { _pDevice, _pDeviceContext }
{
}

CWeapon_Player::CWeapon_Player(const CWeapon_Player& _Prototype)
    : CPartObject (_Prototype)
{
}

HRESULT CWeapon_Player::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CWeapon_Player::Initialize(void* _pArg)
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

    /* Add Prototype (Boomerang) */
    /* Client (Need), MapTool (Need X) */
    //CModel* pModel = CModel::Create(m_pDevice, m_pDeviceContext,
    //    "../../Resources/Models/NonAnim/Weapon_Boomerang/Boomerang.mesh");
    //if (!pModel)
    //    return E_FAIL;

    //if (FAILED(m_pGameInstance->Add_Prototype(
    //    m_ObjectDesc.iLevel,
    //    TEXT("Prototype_Component_Model_Map_Boomerang"),
    //    pModel)))
    //{
    //    Safe_Release(pModel);
    //    MSG_BOX("[WARNING] FILED TO REGISTER MODEL : CWeapon_Boomerang(Boomerang Object)");
    //}

    /* Create */
    Create_Boomerang();
    m_pBoomerang->Set_Active(false);

    return S_OK;
}

void CWeapon_Player::Priority_Update(_float _fTimeDelta)
{
}

void CWeapon_Player::Update(_float _fTimeDelta)
{
    if (!m_pBoomerang)
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

    m_pBoomerang->Set_ReturnTargetMatrix(m_CombinedWorldMatrix);

    if (m_pBoomerang->IsAttached())
        m_eWeaponState = WEAPON_STATE::ATTACHED;

    /* РќДо */
    if (m_eWeaponState == WEAPON_STATE::ATTACHED)
        m_pBoomerang->Update_WorldMatrix(m_CombinedWorldMatrix);
}

void CWeapon_Player::Late_Update(_float _fTimeDelta)
{
    CPartObject::Late_Update(_fTimeDelta);
}

HRESULT CWeapon_Player::Render()
{
    return S_OK;
}

void CWeapon_Player::Throw(_float3 _vDir)
{
    if (!m_pBoomerang)
        return;

    /* Throw */
    m_eWeaponState = WEAPON_STATE::DETACHED;
    m_pBoomerang->Throw(_vDir);
}

void CWeapon_Player::Create_Boomerang()
{
    /* 1. Boomerang */
    CBoomerang::BOOMERANG_INIT_DESC tDesc = {};

    CGameObject::OBJECT_DESC tObjectDesc = {};
    SET_DESC(tObjectDesc.ObjectID, TEXT("Boomerang"));
    tObjectDesc.iLevel = m_ObjectDesc.iLevel;
    SET_DESC(tObjectDesc.LayerTag, m_ObjectDesc.LayerTag);
    SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Actor_Weapon_Boomerang"));
    SET_DESC(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
    SET_DESC(tObjectDesc.ModelTag, TEXT("Prototype_Component_Model_Map_Boomerang"));
    tDesc.tObjectDesc = tObjectDesc;

    _matrix WorldMatrix = XMLoadFloat4x4(&m_CombinedWorldMatrix);
    for (_int i = 0; i < 3; ++i)
    {
        WorldMatrix.r[i] = XMVector3Normalize(WorldMatrix.r[i]);
    }
    XMStoreFloat4x4(&tDesc.tBoomerangDesc.WorldMatrix, WorldMatrix);

    CGameObject* pGameObject = { nullptr };
    m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        tDesc.tObjectDesc.ObjectTag,
        tDesc.tObjectDesc.iLevel,
        tDesc.tObjectDesc.LayerTag,
        &tDesc, &pGameObject);

    m_pBoomerang = static_cast<CBoomerang*>(pGameObject);
}

CWeapon_Player* CWeapon_Player::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CWeapon_Player* pInstance = new CWeapon_Player(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CWeapon_Player");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CWeapon_Player::Clone(void* _pArg)
{
    CWeapon_Player* pInstance = new CWeapon_Player(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CWeapon_Player");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CWeapon_Player::Free()
{
    __super::Free();
}
