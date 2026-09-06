#include "GameObject/MoriblinSpear.h"
#include "GameInstance.h"
#include "GameObject/Body_MoriblinSpear.h"
#include "GameObject/Weapon_MoriblinSpear.h"
#include "GameObject/MoriblinSpear_IDLE.h"
#include "GameObject/MoriblinSpear_Attack.h"
#include "GameObject/MoriblinSpear_Walk.h"
#include "GameObject/MoriblinSpear_Find.h"
#include "GameObject/MoriblinSpear_Look.h"
#include "GameObject/MoriblinSpear_Stance.h"
#include "GameObject/MoriblinSpear_StanceWalk.h"
#include "GameObject/MoriblinSpear_Damage.h"
#include "GameObject/MoriblinSpear_Dead.h"

CMoriblinSpear::CMoriblinSpear(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CActorObject{ _pDevice, _pDeviceContext }
{
}

CMoriblinSpear::CMoriblinSpear(const CMoriblinSpear& _Prototype)
    : CActorObject(_Prototype)
{
}

HRESULT CMoriblinSpear::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMoriblinSpear::Initialize(void* _pArg)
{
    if (FAILED(CActorObject::Initialize(_pArg)))
        return E_FAIL;

    CONTAINERR_INIT_DESC* pInitDesc = static_cast<CONTAINERR_INIT_DESC*>(_pArg);

    /* 2. Create Parts */
    for (const auto& Part : pInitDesc->vecPartsDescs)
    {
        if (FAILED(Create_Part(Part)))
            return E_FAIL;
    }

    /* 5. Init State */
    Register_State(new CMoriblinSpear_IDLE());
    Register_State(new CMoriblinSpear_Attack());
    Register_State(new CMoriblinSpear_WALK());
    Register_State(new CMoriblinSpear_Find());
    Register_State(new CMoriblinSpear_Look());
    Register_State(new CMoriblinSpear_Stance());
    Register_State(new CMoriblinSpear_StanceWalk());
    Register_State(new CMoriblinSpear_Damage());
    Register_State(new CMoriblinSpear_Dead());

    RequestToChangeState(ENUM_TO_UINT(STATE_ID::IDLE));

    /* 7. Casting */
    m_pBody = static_cast<CBody*>(m_vecPartObjects[ENUM_TO_UINT(PART::BODY)]);
    m_pBodyMoriblin = static_cast<CBody_MoriblinSpear*>(m_vecPartObjects[ENUM_TO_UINT(PART::BODY)]);
    if (m_pBodyMoriblin)
        m_pBodyMoriblin->Set_Owner_Parent(this);
    
    /* Here Transform */
    if (&pInitDesc->tContainerDesc)
    {
        CContainerObject::CONTAINER_DESC tDesc = pInitDesc->tContainerDesc;
        m_pTransformCom->Set_Scale(tDesc.vScale.x, tDesc.vScale.y, tDesc.vScale.z);
        //m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 90.f);
        m_pTransformCom->Set_State(Engine::STATE::POSITION, XMVectorSet(tDesc.vPosition.x, tDesc.vPosition.y, tDesc.vPosition.z, 1.f));
    }

    /* Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

    /* Navigation */
    if (FAILED(Ready_Navigation()))
        return E_FAIL;

    return S_OK;
}

void CMoriblinSpear::Priority_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    CContainerObject::Priority_Update(_fTimeDelta);
}

void CMoriblinSpear::Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    // refactor
    if (m_tDeleteTime.bActive)
    {
        m_tDeleteTime.fAccTime += _fTimeDelta;
        m_tDeleteVFX.fAccTime += _fTimeDelta;

        if (m_tDeleteVFX.bActive && m_tDeleteVFX.fAccTime >= m_tDeleteVFX.fAccDurationTime)
        {
            Start_DeleteVFX();
            m_tDeleteVFX.bActive = false;
        }

        if (m_tDeleteTime.fAccTime >= m_tDeleteTime.fAccDurationTime)
        {
            this->Set_Active(false);
            Collider_BodyEnable(false);
            Collider_SensorEnable(false);
        }
    }

    /* EFFECT */
    if (m_tKnockBack.bActive)
    {
        MoveWithKnockBack(_fTimeDelta);

        /* 2. State */
        Change_State();
    }
    else
    {
        /* 1. FSM */
        if (m_pState)
            m_pState->OnStateStay(this, _fTimeDelta);

        /* 2. State */
        Change_State();
    }

    /* 0. Container */
    CContainerObject::Update(_fTimeDelta);

    /* Collider (Parent) */
    if (m_pBodyMoriblin)
        m_pBodyMoriblin->Set_ParentWorld(m_pTransformCom->Get_WorldMatrixPtr());
    
    /* Collider */
    if (m_pBodyCollider)
        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    
    /* Navigation */
    if (m_pNavCom)
        m_pTransformCom->Set_State(STATE::POSITION, m_pNavCom->SetUp_OnNavigation(m_pTransformCom->Get_State(STATE::POSITION)));
}

void CMoriblinSpear::Late_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    CContainerObject::Late_Update(_fTimeDelta);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CMoriblinSpear::Render()
{	
    if (!m_bActive)
		return S_OK;

#ifdef _DEBUG
    //if (m_pBodyCollider) m_pBodyCollider->Render();
    //if (m_pNavCom) m_pNavCom->Render();
#endif // _DEBUG

    return S_OK;
}

void CMoriblinSpear::Register_State(IActorState* _pState)
{
    if (!_pState)
        return;
    _uint ID = _pState->Get_StateID();
    m_StateMap.emplace(ID, _pState);

    if (!m_pState)
        m_pState = _pState;
}

void CMoriblinSpear::RequestToChangeState(_uint _StateID)
{
    m_NexState = _StateID;
    m_bRequestStateChange = true;
}

void CMoriblinSpear::Change_State()
{
    if (!m_bRequestStateChange)
        return;

    auto iter = m_StateMap.find(m_NexState);
    if (iter == m_StateMap.end())
    {
        m_bRequestStateChange = false;
        return;
    }

    if (m_pState)
         m_pState->OnStateExit(this);

    m_pState = iter->second;
    m_pState->OnStateEnter(this);

    m_bRequestStateChange = false;
}

void CMoriblinSpear::Set_SpeedMulti(_float _fMulti)
{
    if (m_pTransformCom)
        m_pTransformCom->Set_SpeedMulti(_fMulti);
}

void CMoriblinSpear::Set_Animation(_string _AnimName, _bool _bLoop, _bool _isForce)
{
    if (m_pBody)
        m_pBody->Set_Animation(_AnimName, _bLoop);
}

void CMoriblinSpear::Reset_Animation_TrackPosition()
{
    if (m_pBody)
        m_pBody->Reset_Animation_TrackPosition();
}

void CMoriblinSpear::Set_AnimationSpeedMulti(_float _fAnimSpeedMulti)
{
    if (m_pBody)
        m_pBody->Set_AnimationSpeedMulti(_fAnimSpeedMulti);
}

_bool CMoriblinSpear::IsAnimFinished() const
{
    if (m_pBody)
        return m_pBody->IsAnimFinished();

    return false;
}

_float CMoriblinSpear::Get_PlayRatio() const
{   
    if (m_pBody)
        return m_pBody->Get_PlayRatio();

    return 0.f;
}

void CMoriblinSpear::Notify_WeaponThrow()
{
    auto* pWeapon = static_cast<CWeapon_MoriblinSpear*>(
        m_vecPartObjects[ENUM_TO_UINT(PART::WEAPON)]);

    Set_AttackTargetPos(static_cast<CTransform*>(
        m_pTarget->Get_Component(TEXT("Com_Transform")))->Get_State(STATE::POSITION));

    if (pWeapon)
        pWeapon->Throw_Target(m_vAttackTargetPos);

    m_bIsThrowing = true;
}

void CMoriblinSpear::Notify_WeaponRespawn()
{
    auto* pWeapon = static_cast<CWeapon_MoriblinSpear*>(
        m_vecPartObjects[ENUM_TO_UINT(PART::WEAPON)]);

    if (pWeapon)
        pWeapon->Respawn();

    m_bIsThrowing = false;
}

void CMoriblinSpear::Set_SavedTargetPosition(_vector _vTargetPos)
{
    m_vAttackTargetPos = _vTargetPos;
}

void CMoriblinSpear::Collider_BodyEnable(_bool _bActive)
{
    if (m_pBodyCollider)
        m_pBodyCollider->Set_Active(_bActive);
}

void CMoriblinSpear::Collider_SensorEnable(_bool _bActive)
{
    if (m_pBodyMoriblin)
        m_pBodyMoriblin->Collider_SensorEnable(_bActive);
}

void CMoriblinSpear::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
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

void CMoriblinSpear::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CMoriblinSpear::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
}

_float CMoriblinSpear::Compute_Distance_To_Target()
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

void CMoriblinSpear::Start_HitFlash()
{
    if (m_pBody)
        m_pBody->Start_HitFlash();
}

void CMoriblinSpear::Start_Dissolve()
{
    if (m_pBody)
        m_pBody->Start_Dissolve();
}

HRESULT CMoriblinSpear::Create_Part(CPartObject::PART_CREATE_DESC _tCreateDesc)
{
    CPartObject::PART_CREATE_DESC tPartCreateDesc = {};

    /* OBJECT DESC*/
    tPartCreateDesc = _tCreateDesc;

    /* PART DESC */
    tPartCreateDesc.tPartDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    tPartCreateDesc.tPartDesc.pPartentSTATE = &m_iState;
    if (!tPartCreateDesc.SocketName.empty())
    {
        tPartCreateDesc.tPartDesc.pSocketMatrix
            = dynamic_cast<CBody_MoriblinSpear*>(m_vecPartObjects[ENUM_TO_UINT(PART::BODY)])
            ->Get_SocketMatrixPtr(WSTRTOCHAR(tPartCreateDesc.SocketName).c_str());
    }

    return CContainerObject::Add_PartObject(
        ENUM_TO_UINT(tPartCreateDesc.ePartType), RESOURCE_LEVEL_STATIC,
        tPartCreateDesc.tObjectDesc.ObjectTag, &tPartCreateDesc);
}

HRESULT CMoriblinSpear::Ready_Collider()
{
    /* Body */
    CBounding_AABB::AABB_DESC tAABBDesc = {};
    tAABBDesc.vExtents = _float3(0.6f, 0.8f, 0.6f);
    tAABBDesc.vCenter = _float3(0.f, tAABBDesc.vExtents.y, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_AABB"),
        TEXT("Com_BodyCollider"),
        reinterpret_cast<CComponent**>(&m_pBodyCollider),
        &tAABBDesc)))
        return E_FAIL;

    m_pBodyCollider->Set_Group(GROUP::MONSTER);
    m_pGameInstance->Register_Collider(m_pBodyCollider);
    m_pBodyCollider->Set_Owner(this);

    return S_OK;
}

HRESULT CMoriblinSpear::Ready_Navigation()
{
    CNavigation::NAVIGATION_DESC tNavDesc = {};
    tNavDesc.iCurrentCellIndex = -1;

    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Navigation_MARTHAsBAY_MONSTER"),
        TEXT("Com_Navigation"),
        reinterpret_cast<CComponent**>(&m_pNavCom),
        &tNavDesc)))
        return E_FAIL;

    _int iCell = m_pNavCom->Find_CurrentCell(m_pTransformCom->Get_State(STATE::POSITION));
    m_pNavCom->Set_CurrentCell(iCell);

    return S_OK;
}

void CMoriblinSpear::Take_Damage(_float _fDamage, _vector _vHitPos)
{
    if (m_fHP < 0.f)
        return;

    m_fHP -= _fDamage;

    if (m_fHP <= 0.f)
    {
        //Start_Dissolve();
        Start_KnockBack(_vHitPos, 10.f, 0.5f);
        m_tDeleteTime.bActive = true;
        m_tDeleteVFX.bActive = true;
        RequestToChangeState(ENUM_TO_UINT(STATE_ID::DEAD));
    }
    else
    {
        Start_HitFlash();
        Start_KnockBack(_vHitPos, 8.f, 0.8f);
        RequestToChangeState(ENUM_TO_UINT(STATE_ID::DAMAGE));
    }
}

void CMoriblinSpear::Start_DeleteVFX()
{
    /* Effect Pos */
    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    vPos += XMVectorSet(0.f, 1.f, 0.f, 0.f);

    /* Star */
    _vector basePos = vPos;
    _int iCount = 8;

    for (_int i = 0; i < iCount; ++i)
    {
        /* Effect */
        CEffectManager::EFFECT_EVENT tEvent{};
        tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::SPAWN_EFFECT;
        tEvent.EffectName = TEXT("Star");

        _float fAngle = XM_2PI * ((rand() % 1000) / 1000.f);
        _float fX = cosf(fAngle);
        _float fZ = sinf(fAngle);
        _float fY = ((rand() % 100) / 100.f) * 0.5f;

        _vector vDir = XMVector3Normalize(XMVectorSet(fX, fY, fZ, 0.f));

        _float foffsetScale = 0.1f + ((rand() % 100) / 100.f) * 0.3f;
        _vector vOffset = vDir * foffsetScale;

        tEvent.vPosition = basePos + vOffset;
        tEvent.vDirection = vDir;

        m_pGameInstance->Push_EffectEVENT(tEvent);
    }

    /* Purple_Smoke */
    CEffectManager::EFFECT_EVENT tEvent{};
    tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::SPAWN_EFFECT;
    tEvent.EffectName = TEXT("Plane");
    tEvent.vPosition = vPos;
    m_pGameInstance->Push_EffectEVENT(tEvent);

    /* SFX */
    m_pGameInstance->Play_Sound(L"ENEMY_Common_Delete_0.wav", SOUND::EFFECT, 3.5f);
}

CMoriblinSpear* CMoriblinSpear::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CMoriblinSpear* pInstance = new CMoriblinSpear(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CMoriblinSpear");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CMoriblinSpear::Clone(void* _pArg)
{
    CMoriblinSpear* pInstance = new CMoriblinSpear(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CMoriblinSpear");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMoriblinSpear::Free()
{
    __super::Free();

    for (auto& Pair : m_StateMap)
        Safe_Delete(Pair.second);
    m_StateMap.clear();

    m_pState = nullptr;
}
