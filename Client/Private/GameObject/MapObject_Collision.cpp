#include "GameObject/MapObject_Collision.h"
#include "GameInstance.h"
#include "WorldObject.h"

CMapObject_Collision::CMapObject_Collision(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CMapObject{ _pDevice, _pDeviceContext }
{
}

CMapObject_Collision::CMapObject_Collision(const CMapObject_Collision& _Prototype)
    : CMapObject(_Prototype)
{
}

HRESULT CMapObject_Collision::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Collision::Initialize(void* _pArg)
{
    if (FAILED(CMapObject::Initialize(_pArg)))
        return E_FAIL;

    MAP_INIT_DESC* pInitDesc = static_cast<MAP_INIT_DESC*>(_pArg);

    /* Collider */
    if (FAILED(Ready_Collider(&pInitDesc->tWorldDesc)))
        return E_FAIL;

    return S_OK;
}

void CMapObject_Collision::Priority_Update(_float _fTimeDelta)
{
}

void CMapObject_Collision::Update(_float _fTimeDelta)
{
    if (m_pColliderCom)
    {
        _float3 vScale = m_pTransformCom->Get_Scaled();

        m_pColliderCom->Set_Extents(_float3(
            m_vBaseExtents.x * vScale.x,
            m_vBaseExtents.y * vScale.y,
            m_vBaseExtents.z * vScale.z));

        _matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
        WorldMatrix.r[0] = XMVector3Normalize(WorldMatrix.r[0]);
        WorldMatrix.r[1] = XMVector3Normalize(WorldMatrix.r[1]);
        WorldMatrix.r[2] = XMVector3Normalize(WorldMatrix.r[2]);

        m_pColliderCom->Update(WorldMatrix);
    }
}

void CMapObject_Collision::Late_Update(_float _fTimeDelta)
{
    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CMapObject_Collision::Render()
{
    if (!m_bActive)
        return S_OK;

#ifdef _DEBUG
    /* Collider */
    //if (m_pColliderCom) m_pColliderCom->Render();
#endif // _DEBUG

    return S_OK;
}

HRESULT CMapObject_Collision::Ready_Collider(WORLD_DESC* _Desc)
{
    _float3 vScale = m_pTransformCom->Get_Scaled();
    m_vBaseExtents = _float3(
        (_Desc->vColliderExtents.x * 0.5f) / vScale.x,
        (_Desc->vColliderExtents.y * 0.5f) / vScale.y,
        (_Desc->vColliderExtents.z * 0.5f) / vScale.z);

    /* Collider */
    CBounding_AABB::AABB_DESC tAABBDesc = {};
    tAABBDesc.vExtents = m_vBaseExtents;
    tAABBDesc.vCenter = _Desc->vColliderCenter;

    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_AABB"),
        TEXT("Com_Collider"),
        reinterpret_cast<CComponent**>(&m_pColliderCom),
        &tAABBDesc)))
        return E_FAIL;

    m_pColliderCom->Set_Owner(this);
    m_pColliderCom->Set_Group(GROUP::MAP);
    m_pColliderCom->Set_Active(true);
    m_pGameInstance->Register_Collider(m_pColliderCom);

    return S_OK;
}

CMapObject_Collision* CMapObject_Collision::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CMapObject_Collision* pInstance = new CMapObject_Collision(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CMapObject_Collision");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CMapObject_Collision::Clone(void* _pArg)
{
    CMapObject_Collision* pInstance = new CMapObject_Collision(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CMapObject_Collision");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMapObject_Collision::Free()
{
    __super::Free();

    Safe_Release(m_pColliderCom);
}
