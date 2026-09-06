#include "GameObject/Player.h"
#include "GameInstance.h"
#include "GameObject/Body_Player.h"
#include "GameObject/Weapon_Player.h"
#include "GameObject/Player_IDLE.h"
#include "GameObject/Player_Walk.h"
#include "GameObject/Player_Run.h"
#include "GameObject/Player_Jump.h"
#include "GameObject/Player_Attack.h"
#include "GameObject/Player_Shield.h"
#include "GameObject/Player_GetItem.h"
#include "GameObject/Player_Boomerang.h"
#include "GameObject/Player_Damage.h"
#include "GameObject/Player_Warp_Up.h"
#include "GameObject/Player_Warp_Down.h"
#include "GameObject/Player_Sleep.h"
#include "GameObject/Player_Sleep_Intro.h"
#include "GameObject/Player_ShieldHit.h"
#include "GameObject/Player_StairDown.h"
#include "GameObject/Player_LadderUp_ST.h"
#include "GameObject/Player_LadderUp_LP.h"
#include "GameObject/Player_Jump_SideView.h"
#include "GameObject/Player_Dodge.h"
#include "GameObject/Player_Counter.h"
#include "GameObject/Player_Jump_Dodge.h"
#include "GameObject/Player_DoorOpen.h"
#include "GameObject/Boomerang.h"

CPlayer::CPlayer(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CActorObject{ _pDevice, _pDeviceContext }
{
}

CPlayer::CPlayer(const CPlayer& _Prototype)
    : CActorObject(_Prototype)
{
}

HRESULT CPlayer::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CPlayer::Initialize(void* _pArg)
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
    Register_State(new CPlayer_IDLE());
    Register_State(new CPlayer_Walk());
    Register_State(new CPlayer_Run());
    Register_State(new CPlayer_Jump());
    Register_State(new CPlayer_Attack());
    Register_State(new CPlayer_Shield());
    Register_State(new CPlayer_GetItem());
    Register_State(new CPlayer_Boomerang());
    Register_State(new CPlayer_Damage());
    Register_State(new CPlayer_Warp_Up());
    Register_State(new CPlayer_Warp_Down());
    Register_State(new CPlayer_Sleep());
    Register_State(new CPlayer_Sleep_Intro());
    Register_State(new CPlayer_ShieldHit());
    Register_State(new CPlayer_StairDown());
    Register_State(new CPlayer_LadderUp_ST());
    Register_State(new CPlayer_LadderUp_LP());
    Register_State(new CPlayer_Jump_SideView());
    Register_State(new CPlayer_Dodge());
    Register_State(new CPlayer_Counter());
    Register_State(new CPlayer_Jump_Dodge());
    Register_State(new CPlayer_DoorOpen());

    /* 6. Connet (ActionController) */
    if (m_pActionController)
        m_pPlayerInputController = CPlayerInputController::Create(m_pActionController);
    
    /* 7. Casting */
    m_pBody = static_cast<CBody*>(m_vecPartObjects[ENUM_TO_UINT(PART::BODY)]);
    m_pBodyPlayer = static_cast<CBody_Player*>(m_vecPartObjects[ENUM_TO_UINT(PART::BODY)]);
    if (m_pBodyPlayer)
        m_pBodyPlayer->Set_Owner_Parent(this);
    m_pWeaponPlayer = static_cast<CWeapon_Player*>(m_vecPartObjects[ENUM_TO_UINT(PART::WEAPON)]);

    /* State */
    m_NextState = ENUM_TO_UINT(IState::STATE_ID::SLEEP_INTRO);
    m_iNextStatePriority = HIGHEST;
    m_bRequestStateChange = true;
    Change_State();

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

    return S_OK;
}

void CPlayer::Priority_Update(_float _fTimeDelta)
{
    _float fGlobalTime = m_pGameInstance->Get_GlobalDelta(TEXT("Timer_60"));
   
    CActorObject::Priority_Update(fGlobalTime * m_fLocalTimeScale);
}

void CPlayer::Update(_float _fTimeDelta)
{
    _float fGlobalTime = m_pGameInstance->Get_GlobalDelta(TEXT("Timer_60"));

    /* 1. KeyInput */
    if (m_pPlayerInputController)
        m_pPlayerInputController->Update();

    CActorObject::Update(fGlobalTime * m_fLocalTimeScale);

    /* Collider (Parent) */
    if (m_pBodyPlayer)
        m_pBodyPlayer->Set_ParentWorld(m_pTransformCom->Get_WorldMatrixPtr());
    
    /* Collider */
    if (m_pBodyCollider)
        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* Navigation */
    if (m_pNavCom && !m_bAir)
    {
        _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
        _vector vNavPos = m_pNavCom->SetUp_OnNavigation(vPos);

        if (m_bAir)
        {
            vNavPos = XMVectorSetY(vNavPos, XMVectorGetY(vPos));
        }

        m_pTransformCom->Set_State(STATE::POSITION, vNavPos);
    }

    /* Wind */
    Update_Wind(fGlobalTime * m_fLocalTimeScale);

    /* Dodge */
    Update_Dodge();

    /* Mesh UI */
    Update_EquipMesh(_fTimeDelta);

    if (m_pGameInstance->Get_DIKeyDown(DIK_O))
    {
        _bool bActive = m_pBodyCollider->IsActive();
        Collider_BodyEnable(!bActive);

        wchar_t buffer[128];
        swprintf_s(buffer, L"[Collider] Body Active (After): %s\n", !bActive ? L"TRUE" : L"FALSE");
        OutputDebugStringW(buffer);
    }
}

void CPlayer::Late_Update(_float _fTimeDelta)
{
    _float fGlobalTime = m_pGameInstance->Get_GlobalDelta(TEXT("Timer_60"));

    CActorObject::Late_Update(fGlobalTime * m_fLocalTimeScale);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CPlayer::Render()
{
#ifdef _DEBUG
    //if (m_pBodyCollider) m_pBodyCollider->Render();
    //if (m_pNavCom) m_pNavCom->Render();
#endif // _DEBUG

    return S_OK;
}

void CPlayer::Set_TransitionInput(IState::TRANSITION_INPUT _eInput)
{
    if (_eInput == IState::TRANSITION_INPUT::END)
        return;

    /* Excep. �ǰ� */
    if (_eInput == IState::TRANSITION_INPUT::DAMAGE_RECEVICED)
    {
        for (_bool& bInput : m_TransitionInputs)
            bInput = false;
    }

    /* Excep. �̵� */
    if (_eInput == IState::TRANSITION_INPUT::MOVE_IDLE
        || _eInput == IState::TRANSITION_INPUT::MOVE_WALK
        || _eInput == IState::TRANSITION_INPUT::MOVE_RUN)
    {
        m_TransitionInputs[ENUM_TO_UINT(IState::TRANSITION_INPUT::MOVE_IDLE)] = false;
        m_TransitionInputs[ENUM_TO_UINT(IState::TRANSITION_INPUT::MOVE_WALK)] = false;
        m_TransitionInputs[ENUM_TO_UINT(IState::TRANSITION_INPUT::MOVE_RUN)] = false;
    }

    /* input Ȱ��ȭ */
    m_TransitionInputs[ENUM_TO_UINT(_eInput)] = true;
}

_bool CPlayer::Is_CurrentStateFinished() const
{
    /* ���� ���� �Ϸ� */
    return m_pState && m_pState->Is_StateFinished();
}

_bool CPlayer::Is_ActionPressed(IState::TRANSITION_INPUT _eInput) const
{
    return CActorObject::Is_ActionPressed(_eInput);
}

void CPlayer::Evaluate_Transitions()
{
    if (!m_pState)
        return;

    using STATE_ID = IState::STATE_ID;
    using INPUT = IState::TRANSITION_INPUT;

    /* Player FSM ���� ���� ���̺�
     * ���� ��Ģ�� �Ѱ����� �����Ѵ�. 
     * { ���� ����, �Է�, ���� ����, �켱����, �߰� ���� }
     */
    static constexpr STATE_TRANSITION StateTransitionTable[] =
    {
        { STATE_ID::IDLE, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION },
        { STATE_ID::IDLE, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION },
        { STATE_ID::WALK, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION },
        { STATE_ID::WALK, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION },
        { STATE_ID::RUN, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION },
        { STATE_ID::RUN, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION },

        { STATE_ID::IDLE, INPUT::ATTACK_PRESSED, STATE_ID::ATTACK, ACTION },
        { STATE_ID::WALK, INPUT::ATTACK_PRESSED, STATE_ID::ATTACK, ACTION },
        { STATE_ID::RUN, INPUT::ATTACK_PRESSED, STATE_ID::ATTACK, ACTION },
        { STATE_ID::IDLE, INPUT::SHIELD_PRESSED, STATE_ID::SHIELD, ACTION },
        { STATE_ID::WALK, INPUT::SHIELD_PRESSED, STATE_ID::SHIELD, ACTION },
        { STATE_ID::RUN, INPUT::SHIELD_PRESSED, STATE_ID::SHIELD, ACTION },
        { STATE_ID::IDLE, INPUT::JUMP_SIDEVIEW_PRESSED, STATE_ID::JUMP_SIDEVIEW, ACTION },
        { STATE_ID::WALK, INPUT::JUMP_SIDEVIEW_PRESSED, STATE_ID::JUMP_SIDEVIEW, ACTION },
        { STATE_ID::RUN, INPUT::JUMP_SIDEVIEW_PRESSED, STATE_ID::JUMP_SIDEVIEW, ACTION },
        { STATE_ID::IDLE, INPUT::JUMP_DODGE_PRESSED, STATE_ID::JUMP_DODGE, ACTION },
        { STATE_ID::WALK, INPUT::JUMP_DODGE_PRESSED, STATE_ID::JUMP_DODGE, ACTION },
        { STATE_ID::RUN, INPUT::JUMP_DODGE_PRESSED, STATE_ID::JUMP_DODGE, ACTION },
        { STATE_ID::IDLE, INPUT::BOOMERANG_PRESSED, STATE_ID::BOOMERANG, ACTION },
        { STATE_ID::WALK, INPUT::BOOMERANG_PRESSED, STATE_ID::BOOMERANG, ACTION },
        { STATE_ID::RUN, INPUT::BOOMERANG_PRESSED, STATE_ID::BOOMERANG, ACTION },

        { STATE_ID::SHIELD, INPUT::SHIELD_HIT_RECEIVED, STATE_ID::SHIELD_HIT, REACTION },
        { STATE_ID::JUMP_DODGE, INPUT::PERFECT_DODGE, STATE_ID::DODGE, REACTION },

        { STATE_ID::IDLE, INPUT::LADDER_UP_REQUESTED, STATE_ID::LADDER_UP_ST, HIGHEST },
        { STATE_ID::WALK, INPUT::LADDER_UP_REQUESTED, STATE_ID::LADDER_UP_ST, HIGHEST },
        { STATE_ID::RUN, INPUT::LADDER_UP_REQUESTED, STATE_ID::LADDER_UP_ST, HIGHEST },
        { STATE_ID::JUMP_SIDEVIEW, INPUT::LADDER_UP_REQUESTED, STATE_ID::LADDER_UP_ST, HIGHEST },
        { STATE_ID::SLEEP_INTRO, INPUT::SLEEP_REQUESTED, STATE_ID::SLEEP, HIGHEST },
        { STATE_ID::IDLE, INPUT::GET_ITEM_REQUESTED, STATE_ID::GET_ITEM, HIGHEST },
        { STATE_ID::WALK, INPUT::GET_ITEM_REQUESTED, STATE_ID::GET_ITEM, HIGHEST },
        { STATE_ID::RUN, INPUT::GET_ITEM_REQUESTED, STATE_ID::GET_ITEM, HIGHEST },
        { STATE_ID::IDLE, INPUT::DOOR_OPEN_REQUESTED, STATE_ID::DOOR_OPEN, HIGHEST },
        { STATE_ID::WALK, INPUT::DOOR_OPEN_REQUESTED, STATE_ID::DOOR_OPEN, HIGHEST },
        { STATE_ID::RUN, INPUT::DOOR_OPEN_REQUESTED, STATE_ID::DOOR_OPEN, HIGHEST },
        { STATE_ID::IDLE, INPUT::WARP_UP_REQUESTED, STATE_ID::WARP_UP, HIGHEST },
        { STATE_ID::WALK, INPUT::WARP_UP_REQUESTED, STATE_ID::WARP_UP, HIGHEST },
        { STATE_ID::RUN, INPUT::WARP_UP_REQUESTED, STATE_ID::WARP_UP, HIGHEST },
        { STATE_ID::WARP_UP, INPUT::WARP_DOWN_REQESTED, STATE_ID::WARP_DOWN, HIGHEST },

        { STATE_ID::LADDER_UP_ST, INPUT::END, STATE_ID::LADDER_UP_LP, HIGHEST,
            [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::LADDER_UP_LP, INPUT::END, STATE_ID::IDLE, HIGHEST,
            [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::DODGE, INPUT::END, STATE_ID::COUNTER, REACTION,
            [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::SLEEP, INPUT::END, STATE_ID::IDLE, HIGHEST,
            [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::DOOR_OPEN, INPUT::END, STATE_ID::IDLE, HIGHEST,
            [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::GET_ITEM, INPUT::END, STATE_ID::IDLE, HIGHEST,
            [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::WARP_DOWN, INPUT::END, STATE_ID::IDLE, HIGHEST,
            [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::SHIELD_HIT, INPUT::END, STATE_ID::SHIELD, ACTION,
            [](const CPlayer& Player) { return Player.Is_CurrentStateFinished() && Player.Is_ActionPressed(INPUT::SHIELD_PRESSED); } },

        { STATE_ID::ATTACK, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::ATTACK, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::ATTACK, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::SHIELD, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::SHIELD, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::SHIELD, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::DAMAGE, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::DAMAGE, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::DAMAGE, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::COUNTER, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::COUNTER, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::COUNTER, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::JUMP, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::JUMP, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::JUMP, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::JUMP_SIDEVIEW, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::JUMP_SIDEVIEW, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::JUMP_SIDEVIEW, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::JUMP_DODGE, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::JUMP_DODGE, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::JUMP_DODGE, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::BOOMERANG, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::BOOMERANG, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::BOOMERANG, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::SHIELD_HIT, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished() && !Player.Is_ActionPressed(INPUT::SHIELD_PRESSED); } },
        { STATE_ID::SHIELD_HIT, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished() && !Player.Is_ActionPressed(INPUT::SHIELD_PRESSED); } },
        { STATE_ID::SHIELD_HIT, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished() && !Player.Is_ActionPressed(INPUT::SHIELD_PRESSED); } },
        { STATE_ID::STAIR_DOWN, INPUT::MOVE_IDLE, STATE_ID::IDLE, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::STAIR_DOWN, INPUT::MOVE_WALK, STATE_ID::WALK, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },
        { STATE_ID::STAIR_DOWN, INPUT::MOVE_RUN, STATE_ID::RUN, LOCOMOTION, [](const CPlayer& Player) { return Player.Is_CurrentStateFinished(); } },

        { STATE_ID::IDLE, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION },
        { STATE_ID::WALK, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION },
        { STATE_ID::RUN, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION },
        { STATE_ID::ATTACK, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION },
        { STATE_ID::SHIELD, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION },
        { STATE_ID::JUMP_SIDEVIEW, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION },
        { STATE_ID::JUMP_DODGE, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION },
        { STATE_ID::BOOMERANG, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION },
        { STATE_ID::SHIELD_HIT, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION },
        { STATE_ID::DODGE, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION },
        { STATE_ID::COUNTER, INPUT::DAMAGE_RECEVICED, STATE_ID::DAMAGE, REACTION }
    };

    const STATE_ID eCurrent = static_cast<STATE_ID>(m_pState->Get_StateID());
    const STATE_TRANSITION* pSelected = nullptr;

    /* ���� �˻� */
    for (const STATE_TRANSITION& Transition : StateTransitionTable)
    {
        if (Transition.eFrom != eCurrent)
            continue;
        if (Transition.eInput != INPUT::END && !m_TransitionInputs[ENUM_TO_UINT(Transition.eInput)])
            continue;
        if (Transition.pCondition && !Transition.pCondition(*this))
            continue;
        if (!pSelected || Transition.iPriority > pSelected->iPriority)
            pSelected = &Transition;
    }

    if (!pSelected)
        return;

    /* ���� ���� ���� �� ����� �Է� �Һ� */
    m_NextState = ENUM_TO_UINT(pSelected->eTo);
    m_iNextStatePriority = pSelected->iPriority;
    m_bRequestStateChange = true;

    if (pSelected->eInput != INPUT::END)
        m_TransitionInputs[ENUM_TO_UINT(pSelected->eInput)] = false;
}
void CPlayer::Set_Animation(_string _AnimName, _bool _bLoop, _bool _isForce)
{
    if (m_pBody)
        m_pBody->Set_Animation(_AnimName, _bLoop, _isForce);
}

void CPlayer::Reset_Animation_TrackPosition()
{
    if (m_pBody)
        m_pBody->Reset_Animation_TrackPosition();
}

void CPlayer::Set_AnimationSpeedMulti(_float _fAnimSpeedMulti)
{
    if (m_pBody)
        m_pBody->Set_AnimationSpeedMulti(_fAnimSpeedMulti);
}

_bool CPlayer::IsAnimFinished() const
{
    if (m_pBody)
        return m_pBody->IsAnimFinished();
    return false;
}

_float CPlayer::Get_PlayRatio() const
{
    if (m_pBody)
        return m_pBody->Get_PlayRatio();
    return 0.f;
}

void CPlayer::Notify_WeaponThrow(_float3 _vDir)
{
    auto* pWeapon = static_cast<CWeapon_Player*>(
        m_vecPartObjects[ENUM_TO_UINT(PART::WEAPON)]);

    if (pWeapon)
        pWeapon->Throw(_vDir);
}

_vector CPlayer::Get_AimDirection() const
{
    if (m_pActionController)
        return m_pActionController->Get_AimDirection();
    
    return XMVectorZero();
}

_vector CPlayer::Filter_KnockBackAxis(_vector _vDir)
{
    if (m_eViewMode == VIEW_MODE::TOP)
        _vDir = XMVectorSetY(_vDir, 0.f);
    else if (m_eViewMode == VIEW_MODE::SIDE)
    {
        _vDir = XMVectorSetY(_vDir, 0.f);
        _vDir = XMVectorSetZ(_vDir, 0.f);
    }

    return _vDir;
}

void CPlayer::Set_DodgePhase(DODGE _eDodgePhase)
{
    m_eDogePhase = _eDodgePhase;
}

_uint CPlayer::Get_DodgePhase() const
{
    return ENUM_TO_UINT(m_eDogePhase);
}

void CPlayer::Update_Dodge()
{
    switch (m_eDogePhase)
    {
    case Client::CPlayer::DODGE::NONE:
    {
    }
        break;
    case Client::CPlayer::DODGE::SLOW:
    {
        /* Pefect Dodge */
        m_pGameInstance->Set_TimeScale(0.05f);
        Set_LocalTimeScale(0.2f);
        //m_pGameInstance->Start_HitStop(0.03f, 0.f);
    }
        break;
    case Client::CPlayer::DODGE::ATTACK:
    {
        /* Camera EFFECT */
        m_pGameInstance->Set_TimeScale(1.f);
        Set_LocalTimeScale(2.f);
        //m_bCameraShake = true; // ���ϰ� 1��

        // TODO : Here !
        // attack++ if (7) SET_dodgePahse(ED) -> return dungeonModeCamera
    }
        break;
    case Client::CPlayer::DODGE::ED:
    {
        // ����ȭ
        m_pGameInstance->Set_TimeScale(1.f);
        Set_LocalTimeScale(1.f);
    }
        break;
    }
}

void CPlayer::Set_InputMode(CPlayerInputController::INPUT_MODE _eMode)
{
    if (m_pPlayerInputController)
        m_pPlayerInputController->Set_InputMode(_eMode);
}

void CPlayer::Set_InputLock(_bool _bActive)
{
    if (m_pPlayerInputController)
        m_pPlayerInputController->Set_InputLock(_bActive);

    if (_bActive)
    {
        for (_bool& bInput : m_TransitionInputs)
            bInput = false;
        Set_TransitionInput(IState::TRANSITION_INPUT::MOVE_IDLE);
    }
}

void CPlayer::Apply_Gravity(_float _fTimeDelta)
{
    if (!m_bAir)
        return;

    auto pTransform = static_cast<CTransform*>(Get_Component(TEXT("Com_Transform")));
    _vector vPrevPos = pTransform->Get_State(STATE::POSITION);

    if (m_fVelocityY > 0)
        m_fVelocityY += m_fGravity * 0.7f * _fTimeDelta;
    else
        m_fVelocityY += m_fGravity * 1.4f * _fTimeDelta;

    _float fNewY = XMVectorGetY(vPrevPos) + m_fVelocityY * _fTimeDelta;
    _vector vNewPos = XMVectorSetY(vPrevPos, fNewY);

    /* Falling */
    _vector rayStart = vPrevPos + XMVectorSet(0.f, 0.1f, 0.f, 0.f);
    if (m_fVelocityY < 0.f)
    {
        //if (m_pGameInstance->Raycast_GroundPlane(rayStart, vNewPos, m_fGroundY))
        //{
        //    //vNewPos = XMVectorSetY(vPrevPos, m_fGroundY);
        //    vNewPos = XMVectorSetY(vNewPos, m_fGroundY);
        //    m_fVelocityY = 0.f;
        //    m_bAir = false;
        //}

        bool bGround = m_pGameInstance->Raycast_GroundPlane(rayStart, vNewPos, m_fGroundY);

        if (!bGround && m_pNavCom)
        {
            int iCell = m_pNavCom->Find_CurrentCell(vNewPos);
            if (iCell != -1)
            {
                bGround = true;
                m_fGroundY = XMVectorGetY(vPrevPos);
            }
        }

        if (bGround)
        {
            vNewPos = XMVectorSetY(vNewPos, m_fGroundY);
            m_fVelocityY = 0.f;
            m_bAir = false;
        }
    }

    pTransform->Set_State(STATE::POSITION, vNewPos);
}

void CPlayer::Start_Jump()
{
    m_bAir = true;
    m_fVelocityY = m_fJumpPower;

    auto pTransform = static_cast<CTransform*>(Get_Component(TEXT("Com_Transform")));
    _vector vPos = pTransform->Get_State(STATE::POSITION);

    m_fGroundY = XMVectorGetY(vPos);
}

_bool CPlayer::Is_Air()
{
    return m_bAir;
}

void CPlayer::Collider_BodyEnable(_bool _bActive)
{
    if (m_pBodyCollider)
        m_pBodyCollider->Set_Active(_bActive);
}

void CPlayer::Collider_SwordEnable(_bool _bActive)
{
    if (m_pBodyPlayer)
        m_pBodyPlayer->Collider_SwordEnable(_bActive);
}

void CPlayer::Collider_ShiledEnable(_bool _bActive)
{
    if (m_pBodyPlayer)
        m_pBodyPlayer->Collider_ShiledEnable(_bActive);
}

void CPlayer::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
    if (_pSrc->Get_Group() == GROUP::PLAYER)
    {
        if (_Dst->Get_Group() == GROUP::WEAPON_MONSTER)
        {
            /* Camera Shaking */
            m_bCameraShake = true;

            /* KnockBack */
            CTransform* pTargetTransform = static_cast<CTransform*>(_Dst->Get_Owner()->Get_Component(TEXT("Com_Transform")));
            _vector vWeaponPos = pTargetTransform->Get_State(STATE::POSITION);
            Take_Damage(1.f, vWeaponPos);
        }

        if (_Dst->Get_Group() == GROUP::MONSTER)
        {
            /* Camera Shaking */
            m_bCameraShake = true;

            /* KnockBack */
            CTransform* pTargetTransform = static_cast<CTransform*>(_Dst->Get_Owner()->Get_Component(TEXT("Com_Transform")));
            _vector vWeaponPos = pTargetTransform->Get_State(STATE::POSITION);
            Take_Damage(1.f, vWeaponPos);
        }

        if (_Dst->Get_Group() == GROUP::WIND)
        {
            m_bInWind = true;

            auto pWindTransform = static_cast<CTransform*>(_Dst->Get_Owner()->Get_Component(TEXT("Com_Transform")));
            
            _vector vWindPos = pWindTransform->Get_State(STATE::POSITION);
            _vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);

            m_vWindDir = XMVector3Normalize(vPlayerPos - vWindPos);
        }

        if (_Dst->Get_Group() == GROUP::ATTACK_WINDOW_MONSTER)
        {
            if (m_pState->Get_StateID() == ENUM_TO_UINT(IState::STATE_ID::JUMP_DODGE))
            {
                Set_TransitionInput(IState::TRANSITION_INPUT::PERFECT_DODGE);
                return;
            }
        }
    }

    if (_pSrc->Get_Group() == GROUP::SHIELD_PLAYER)
    {
        if (_Dst->Get_Group() == GROUP::WEAPON_MONSTER)
        {
            /* Camera Shaking */
            m_bCameraShake = true;

            /* KnockBack */
            CTransform* pTargetTransform = static_cast<CTransform*>(_Dst->Get_Owner()->Get_Component(TEXT("Com_Transform")));
            _vector vWeaponPos = pTargetTransform->Get_State(STATE::POSITION);
            Start_KnockBack(vWeaponPos, 1.0f, 0.3f);

            /* Animation */
            m_pActionController->Push_Action(IState::TRANSITION_INPUT::SHIELD_HIT_RECEIVED);
        }

        if (_Dst->Get_Group() == GROUP::MONSTER)
        {
            /* Camera Shaking */
            m_bCameraShake = true;

            /* KnockBack */
            CTransform* pTargetTransform = static_cast<CTransform*>(_Dst->Get_Owner()->Get_Component(TEXT("Com_Transform")));
            _vector vWeaponPos = pTargetTransform->Get_State(STATE::POSITION);
            Start_KnockBack(vWeaponPos, 1.0f, 0.3f);

            /* Animation */
            m_pActionController->Push_Action(IState::TRANSITION_INPUT::SHIELD_HIT_RECEIVED);
        }
    }
}

void CPlayer::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CPlayer::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
    if (_Dst->Get_Group() == GROUP::WIND)
    {
        m_bInWind = false;
    }
}

_bool CPlayer::Notify_CameraShake()
{
    if (!m_bCameraShake)
        return false;

    m_bCameraShake = false;
    return true;
}

_bool CPlayer::Notify_SleepEndEvent()
{
    if (!m_bSleepEndEvent)
        return false;

    m_bSleepEndEvent = false;
    return true;
}

void CPlayer::Set_SleepEndEvent()
{
    m_bSleepEndEvent = true;
}

_bool CPlayer::Notify_WakeUpEvent()
{
    if (!m_bWakeUpEvent)
        return false;

    m_bWakeUpEvent = false;
    return true;
}

void CPlayer::Set_WakeUpEvent()
{
    m_bWakeUpEvent = true;
}

void CPlayer::Own_Ladder_Value(_float _fTopValue, _float _fBottomValue)
{
    m_fLadderTopPosY = _fTopValue;
    m_fLadderBottomPosY = _fBottomValue;
}

void CPlayer::Return_Ladder_Y_Value(_float& _fTopValue, _float& _fBottomValue)
{
    _fTopValue = m_fLadderTopPosY;
    _fBottomValue = m_fLadderBottomPosY;
}

void CPlayer::Set_StairTargetPos(_vector _TargetPos)
{
    m_vStairTargetPos = _TargetPos;
}

_vector CPlayer::Get_StairTargetPos()
{
    return m_vStairTargetPos;
}

HRESULT CPlayer::Create_Part(CPartObject::PART_CREATE_DESC _tCreateDesc)
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
            = dynamic_cast<CBody_Player*>(m_vecPartObjects[ENUM_TO_UINT(PART::BODY)])
            ->Get_SocketMatrixPtr(WSTRTOCHAR(tPartCreateDesc.SocketName).c_str());
    }

    return CContainerObject::Add_PartObject(
        ENUM_TO_UINT(tPartCreateDesc.ePartType), RESOURCE_LEVEL_STATIC,
        tPartCreateDesc.tObjectDesc.ObjectTag, &tPartCreateDesc);
}

HRESULT CPlayer::Ready_Collider()
{
    /* Body */
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

    m_pBodyCollider->Set_Group(GROUP::PLAYER);
    m_pBodyCollider->Set_Owner(this);
    m_pGameInstance->Register_Collider(m_pBodyCollider);

    return S_OK;
}

void CPlayer::Set_LocalTimeScale(_float _fTimeScale)
{
    m_fLocalTimeScale = _fTimeScale;
}

void CPlayer::Take_Damage(_float _fDamage, _vector _vHitPos)
{
    if (m_fHP < 0.f)
        return;

    m_fHP -= _fDamage;

    if (m_fHP <= 0.f)
    {
        Start_Dissolve();
        Start_KnockBack(_vHitPos, 3.8f, 0.8f);
        m_pActionController->Push_Action(IState::TRANSITION_INPUT::DAMAGE_RECEVICED);

        /* UI Event */
        //CEventManager::UI_EVENT tUIEvent = {};
        //tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::HP_ADD_MAXHP;
        //tUIEvent.fValue = 1.f;
        //m_pGameInstance->Push_UIEVENT(tUIEvent);
    }
    else
    {
        Start_HitFlash();
        Start_KnockBack(_vHitPos, 3.8f, 0.8f);
        m_pActionController->Push_Action(IState::TRANSITION_INPUT::DAMAGE_RECEVICED);

        /* UI Event */
        CEventManager::UI_EVENT tUIEvent = {};
        tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::HP_DAMAGE;
        tUIEvent.fValue = 0.5f;
        m_pGameInstance->Push_UIEVENT(tUIEvent);
    }
}

void CPlayer::Start_HitFlash()
{
    if (m_pBody)
        m_pBody->Start_HitFlash();
}

void CPlayer::Start_Dissolve()
{
    if (m_pBody)
        m_pBody->Start_Dissolve();
}

const _float4x4* CPlayer::Get_SocketMatrixPtr(const _char* _pBoneName)
{
    if (m_pBodyPlayer)
        return m_pBodyPlayer->Get_SocketMatrixPtr(_pBoneName);
}

void CPlayer::Update_Wind(_float _fTimeDelta)
{
    if (m_bInWind)
    {
        _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
        vPos += m_vWindDir * m_fWindPower * _fTimeDelta;
        m_pTransformCom->Set_State(STATE::POSITION, vPos);
    }
}

void CPlayer::Update_EquipMesh(_float _fTimeDelta)
{
    /* Debug Equip */
    if (m_pGameInstance->Get_DIKeyDown(DIK_B))
    {
        m_pBodyPlayer->Get_BodyModel()->Set_MeshActive("linkSwordA_MI_sowrdA", true);
        m_pBodyPlayer->Get_BodyModel()->Set_MeshActive("linkSwordA_MI_sowrdAball", true);

        m_pWeaponPlayer->m_pBoomerang->Set_Active(false);

        /* UI Event */
        CEventManager::UI_EVENT tUIEvent = {};
        tUIEvent.eEquipBtn = EQUIPBTN::X;
        tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_SELECTED_SLOT;
        m_pGameInstance->Push_UIEVENT(tUIEvent);

        /* SFX */
        m_pGameInstance->Play_Sound(L"UI_Inventory_XY.wav", SOUND::EFFECT);
    }

    if (m_pGameInstance->Get_DIKeyDown(DIK_N))
    {
        m_pBodyPlayer->Get_BodyModel()->Set_MeshActive("linkShieldA_MI_shieldA", true);

        /* UI Event */
        CEventManager::UI_EVENT tUIEvent = {};
        tUIEvent.eEquipBtn = EQUIPBTN::Y;
        tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_SELECTED_SLOT;
        m_pGameInstance->Push_UIEVENT(tUIEvent);

        /* SFX */
        m_pGameInstance->Play_Sound(L"UI_Inventory_XY.wav", SOUND::EFFECT);
    }

    /* Debug Equip */
    if (m_pGameInstance->Get_DIKeyDown(DIK_M))
    {
        m_pWeaponPlayer->m_pBoomerang->Set_Active(true);

        m_pBodyPlayer->Get_BodyModel()->Set_MeshActive("linkSwordA_MI_sowrdA", false);
        m_pBodyPlayer->Get_BodyModel()->Set_MeshActive("linkSwordA_MI_sowrdAball", false);

        /* UI Event */
        CEventManager::UI_EVENT tUIEvent = {};
        tUIEvent.eEquipBtn = EQUIPBTN::X;
        tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_SELECTED_SLOT;
        m_pGameInstance->Push_UIEVENT(tUIEvent);

        /* SFX */
        m_pGameInstance->Play_Sound(L"UI_Inventory_XY.wav", SOUND::EFFECT);
    }
}

HRESULT CPlayer::Ready_Navigation(_uint _iNavLevel, _wstring _NavTag)
{
    if (m_pNavCom)
    {
        Remove_Component(TEXT("Com_Navigation"));
        Safe_Release(m_pNavCom);
        m_pNavCom = nullptr;
    }

    CNavigation::NAVIGATION_DESC tNavDesc = {};
    tNavDesc.iCurrentCellIndex = -1;

    if (FAILED(Add_Component(
        _iNavLevel,
        _NavTag,
        TEXT("Com_Navigation"), 
        reinterpret_cast<CComponent**>(&m_pNavCom), 
        &tNavDesc)))
        return E_FAIL;

    return S_OK;
}

void CPlayer::Set_ViewMode(VIEW_MODE _eMode)
{
    m_eViewMode = _eMode;
}

CPlayer* CPlayer::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CPlayer* pInstance = new CPlayer(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CPlayer");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CPlayer::Clone(void* _pArg)
{
    CPlayer* pInstance = new CPlayer(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CPlayer");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CPlayer::Free()
{
    __super::Free();

    Safe_Release(m_pPlayerInputController);
}
