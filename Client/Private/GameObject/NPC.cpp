#include "GameObject/NPC.h"
#include "GameInstance.h"

CNPC::CNPC(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CPawnObject{ _pDevice, _pDeviceContext }
{
}

CNPC::CNPC(const CNPC& _Prototype)
    : CPawnObject(_Prototype)
{
}

HRESULT CNPC::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CNPC::Initialize(void* _pArg)
{
    /* 1. Pawn Object */
    if (FAILED(CPawnObject::Initialize(_pArg)))
        return E_FAIL;

    m_NPCName = Get_ObjectID(); // m_wstrObjectID �ᵵ �ɵ�.

    /* 6. Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

    return S_OK;
}

void CNPC::Priority_Update(_float _fTimeDelta)
{
    CPawnObject::Priority_Update(_fTimeDelta);
}

void CNPC::Update(_float _fTimeDelta)
{
    CPawnObject::Update(_fTimeDelta);

    /* Collider */
    if (m_pBodyCollider)
        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CNPC::Late_Update(_float _fTimeDelta)
{
    CPawnObject::Late_Update(_fTimeDelta);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::SHADOW, this)))
        return;
}

HRESULT CNPC::Render()
{
    if (FAILED(CPawnObject::Render()))
        return E_FAIL;

#ifdef _DEBUG
    //if (m_pBodyCollider) m_pBodyCollider->Render();
#endif

    return S_OK;
}

void CNPC::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
    if (_Dst->Get_Group() == GROUP::PLAYER)
        m_bPlayerInRange = true;
}

void CNPC::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CNPC::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
    if (_Dst->Get_Group() == GROUP::PLAYER)
        m_bPlayerInRange = false;
}

HRESULT CNPC::Ready_Collider()
{
    /* Collider (Body) */
    CBounding_AABB::AABB_DESC tAABBDesc = {};
    tAABBDesc.vExtents = _float3(0.4f, 0.7f, 0.4f);
    tAABBDesc.vCenter = _float3(0.f, tAABBDesc.vExtents.y, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_AABB"),
        TEXT("Com_BodyCollider"),
        reinterpret_cast<CComponent**>(&m_pBodyCollider),
        &tAABBDesc)))
        return E_FAIL;

    m_pBodyCollider->Set_Owner(this);
    m_pBodyCollider->Set_Group(GROUP::NPC);
    m_pGameInstance->Register_Collider(m_pBodyCollider);

    return S_OK;
}

CNPC* CNPC::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CNPC* pInstance = new CNPC(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CNPC");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CNPC::Clone(void* _pArg)
{
    CNPC* pInstance = new CNPC(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("Failed to Cloned : CNPC");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CNPC::Free()
{
    __super::Free();

    Safe_Release(m_pBodyCollider);
}
