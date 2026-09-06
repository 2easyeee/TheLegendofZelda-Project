#include "GameObject/Monster_Togezo.h"
#include "GameInstance.h"
#include "GameObject/Togezo_IDLE.h"
#include "GameObject/Togezo_Discover.h"
#include "GameObject/Togezo_Walk.h"
#include "GameObject/Togezo_Run.h"
#include "GameObject/Togezo_Rebound.h"
#include "GameObject/Togezo_Dead.h"

CMonster_Togezo::CMonster_Togezo(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CPawnObject { _pDevice, _pDeviceContext }
{
}

CMonster_Togezo::CMonster_Togezo(const CMonster_Togezo& _Prototype)
    : CPawnObject (_Prototype)
{
}

HRESULT CMonster_Togezo::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMonster_Togezo::Initialize(void* _pArg)
{
    /* 1. Pawn Object */
    if (FAILED(CPawnObject::Initialize(_pArg)))
        return E_FAIL;

    /* 5. Init State */
    Register_State(new CTogezo_IDLE());
    Register_State(new CTogezo_Discover());
    Register_State(new CTogezo_Walk());
    Register_State(new CTogezo_Run());
    Register_State(new CTogezo_Rebound());
    Register_State(new CTogezo_Dead());

    RequestToChangeState(ENUM_TO_UINT(STATE_ID::IDLE));
    Change_State();

    /* 6. Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

    /* Navigation */
    if (FAILED(Ready_Navigation()))
        return E_FAIL;

    return S_OK;
}

void CMonster_Togezo::Priority_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    CPawnObject::Priority_Update(_fTimeDelta);
}

void CMonster_Togezo::Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    // refactor 
    if (m_tDeleteTime.bActive)
    {
        m_tDeleteTime.fAccTime += _fTimeDelta;
        if (m_tDeleteTime.fAccTime >= m_tDeleteTime.fAccDurationTime)
        {
            this->Set_Active(false);
            Collider_BodyEnable(false);
            Collider_SensorEnable(false);
        }
    }

    /* PawnObject */
    CPawnObject::Update(_fTimeDelta);

    /* Collider */
    if (m_pBodyCollider)
        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    if (m_pSensorCollider)
        m_pSensorCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    
    /* Navigation */
    if (m_pNavCom)
        m_pTransformCom->Set_State(STATE::POSITION, m_pNavCom->SetUp_OnNavigation(m_pTransformCom->Get_State(STATE::POSITION)));
}

void CMonster_Togezo::Late_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    CPawnObject::Late_Update(_fTimeDelta);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::SHADOW, this)))
        return;
}

HRESULT CMonster_Togezo::Render()
{
    if (!m_bActive)
        return S_OK;

    if (FAILED(CPawnObject::Render()))
        return E_FAIL;

#ifdef _DEBUG
    //if (m_pBodyCollider) m_pBodyCollider->Render();
    //if (m_pSensorCollider) m_pSensorCollider->Render();
#endif

    return S_OK;
}

void CMonster_Togezo::Collider_BodyEnable(_bool _bActive)
{
    if (m_pBodyCollider)
        m_pBodyCollider->Set_Active(_bActive);
}

void CMonster_Togezo::Collider_SensorEnable(_bool _bActive)
{
    if (m_pSensorCollider)
        m_pSensorCollider->Set_Active(_bActive);
}

void CMonster_Togezo::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
    if (_pSrc->Get_Group() == GROUP::MONSTER)
    {
        if (_Dst->Get_Group() == GROUP::WEAPON_PLAYER)
        {
            // KnockBack
            CTransform* pTargetTransform = static_cast<CTransform*>(_Dst->Get_Owner()->Get_Component(TEXT("Com_Transform")));
            _vector vWeaponPos = pTargetTransform->Get_State(STATE::POSITION);
            Take_Damage(1.f, vWeaponPos);
        }
    }

    if (_pSrc->Get_Group() == GROUP::SENSOR_MONSTER)
    {
        if (_Dst->Get_Group() == GROUP::PLAYER)
        {
            Set_Target(_Dst->Get_Owner());
        }
    }
}

void CMonster_Togezo::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
    if (_pSrc->Get_Group() == GROUP::SENSOR_MONSTER)
    {
        if (_Dst->Get_Group() == GROUP::PLAYER)
        {
            Set_Target(_Dst->Get_Owner());
        }
    }
}

void CMonster_Togezo::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
    if (_pSrc->Get_Group() == GROUP::SENSOR_MONSTER)
    {
        if (_Dst->Get_Group() == GROUP::PLAYER)
        {
            Set_Target(nullptr);
        }
    }
}

_float CMonster_Togezo::Compute_Distance_To_Target()
{
    if (!m_pTarget)
        return FLT_MAX;

    CTransform* pTransform_Target = static_cast<CTransform*>(m_pTarget->Get_Component(TEXT("Com_Transform")));

    _vector vMy = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vTarget = pTransform_Target->Get_State(STATE::POSITION);

    _vector DeleteY = XMVectorSetY(vTarget - vMy, 0.f);
    _vector vDir = XMVector3Normalize(DeleteY);

    /* Distance */
    _float fDist = XMVectorGetX(XMVector3Length(DeleteY));

    return fDist;
}

HRESULT CMonster_Togezo::Ready_Collider()
{
    /* Collider (Body) */
    CBounding_AABB::AABB_DESC tAABBDesc = {};
    tAABBDesc.vExtents = _float3(0.6f, 0.6f, 0.6f);
    tAABBDesc.vCenter = _float3(0.f, tAABBDesc.vExtents.y, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_AABB"),
        TEXT("Com_BodyCollider"),
        reinterpret_cast<CComponent**>(&m_pBodyCollider),
        &tAABBDesc)))
        return E_FAIL;

    m_pBodyCollider->Set_Owner(this);
    m_pBodyCollider->Set_Group(GROUP::MONSTER);
    m_pGameInstance->Register_Collider(m_pBodyCollider);

    /* Collider (Sensor) */
    CBounding_Sphere::SPHERE_DESC tSphereDesc = {};
    tSphereDesc.fRadius = 5.f;
    tSphereDesc.vCenter = _float3(0.f, tSphereDesc.fRadius, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_SPHERE"),
        TEXT("Com_SensorCollider"),
        reinterpret_cast<CComponent**>(&m_pSensorCollider),
        &tSphereDesc)))
        return E_FAIL;
    m_pSensorCollider->Set_Owner(this);
    m_pSensorCollider->Set_Group(GROUP::SENSOR_MONSTER);
    m_pGameInstance->Register_Collider(m_pSensorCollider);

    return S_OK;
}

HRESULT CMonster_Togezo::Ready_Navigation()
{
    CNavigation::NAVIGATION_DESC tNavDesc = {};
    tNavDesc.iCurrentCellIndex = 35;

    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Navigation_Dungeon_MONSTER"),
        TEXT("Com_Navigation"),
        reinterpret_cast<CComponent**>(&m_pNavCom),
        &tNavDesc)))
        return E_FAIL;

    _int iCell = m_pNavCom->Find_CurrentCell(m_pTransformCom->Get_State(STATE::POSITION));
    m_pNavCom->Set_CurrentCell(iCell);

    return S_OK;
}

void CMonster_Togezo::Take_Damage(_float _fDamage, _vector _vHitPos)
{
    if (m_fHP < 0.f)
        return;

    m_fHP -= _fDamage;

    if (m_fHP <= 0.f)
    {
        Start_HitFlash();
        Start_Dissolve();
        Start_KnockBack(_vHitPos, 10.f, 0.5f);
        RequestToChangeState(ENUM_TO_UINT(STATE_ID::DEAD));
        m_tDeleteTime.bActive = true;
        m_tDeleteTime.fAccTime = 0.f;
    }
    else
    {
        Start_HitFlash();
        Start_KnockBack(_vHitPos, 8.f, 0.8f);
        RequestToChangeState(ENUM_TO_UINT(STATE_ID::REBOUND));
    }
}

CMonster_Togezo* CMonster_Togezo::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CMonster_Togezo* pInstance = new CMonster_Togezo(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CMonster_Togezo");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CMonster_Togezo::Clone(void* _pArg)
{
    CMonster_Togezo* pInstance = new CMonster_Togezo(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("Failed to Cloned : CMonster_Togezo");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMonster_Togezo::Free()
{
    __super::Free();

    Safe_Release(m_pSensorCollider);
}
