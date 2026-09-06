#pragma once
#include "Client_Defines.h"
#include "ActorObject.h"
#include "GameObject/PlayerInputController.h"
#include "EventManager.h"

class CPlayer_IDLE;
class CPlayer_Walk;
class CPlayer_Run;
class CPlayer_Jump;
class CPlayer_Jump_SideView;
class CPlayer_Attack;
class CPlayer_Shield;
class CPlayer_GetITem;
class CPlayer_Boomerang;
class CPlayer_Damage;
class CPlayer_Warp_Up;
class CPlayer_Warp_Down;
class CPlayer_Sleep;
class CPlayer_Sleep_Intro;
class CPlayer_ShieldHit;
class CPlayer_StairDown;
class CPlayer_LadderUp_ST;
class CPlayer_LadderUp_LP;
class CPlayer_LadderUp_ED;
class CPlayer_Dodge;
class CPlayer_Counter;
class CPlayer_Jump_Dodge;
class CPlayer_DoorOpen;

NS_BEGIN(Engine)
class CActorObject;
class CBody;
class CNavigation;
NS_END

NS_BEGIN(Client)
class CPlayer final : public CActorObject
{
public:
	enum class VIEW_MODE { TOP, SIDE, END };
	enum class DODGE { NONE, SLOW, ATTACK, ED, END };

private:
	CPlayer(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CPlayer(const CPlayer& _Prototype);
	virtual ~CPlayer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	/* State*/
	virtual void Set_TransitionInput(IState::TRANSITION_INPUT _eInput) override;
	virtual void Evaluate_Transitions() override;
	_bool Is_CurrentStateFinished() const;
	_bool Is_ActionPressed(IState::TRANSITION_INPUT _eInput) const;

	/* Animation (Child) */
	virtual void Set_Animation(_string _AnimName, _bool _bLoop = true, _bool _isForce = false) override;
	virtual void Reset_Animation_TrackPosition() override;
	virtual void Set_AnimationSpeedMulti(_float _fAnimSpeedMulti) override;
	virtual _bool IsAnimFinished() const override;
	virtual _float Get_PlayRatio() const override;

	/* Player */
	virtual void Notify_WeaponThrow(_float3 _vDir) override;
	_vector Get_AimDirection() const;
	virtual _vector Filter_KnockBackAxis(_vector _vDir) override;

	/* Player (Dodge) */
	void Set_DodgePhase(DODGE _eDodgePhase);
	_uint Get_DodgePhase() const;
	void Update_Dodge();

	/* Player(Input) */
	void Set_InputMode(CPlayerInputController::INPUT_MODE _eMode);
	void Set_InputLock(_bool _bActive);
	void Apply_Gravity(_float _fTimeDelta);
	void Start_Jump();
	_bool Is_Air();

	/* Collider */
	void Collider_BodyEnable(_bool _bActive);
	void Collider_SwordEnable(_bool _bActive);
	void Collider_ShiledEnable(_bool _bActive);

	/* Collision*/
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) override;

	/* Camera */
	_bool Notify_CameraShake();

	/* Event */
	_bool Notify_SleepEndEvent();
	void Set_SleepEndEvent();
	_bool Notify_WakeUpEvent();
	void Set_WakeUpEvent();
	void Own_Ladder_Value(_float _fTopValue, _float _fBottomValue);
	void Return_Ladder_Y_Value(_float& _fTopValue, _float& _fBottomValue);
	void Set_StairTargetPos(_vector _TargetPos);
	_vector Get_StairTargetPos();

	/* Reset Navigation */
	HRESULT Ready_Navigation(_uint _iNavLevel, _wstring _NavTag);

	/* View Mode */
	void Set_ViewMode(VIEW_MODE _eMode);

	/* DAMAGE */
	void Take_Damage(_float _fDamage, _vector _vHitPos);

	/* Shader Effect */
	void Start_HitFlash();
	void Start_Dissolve();

	/* Socket (Mapping) */
	const _float4x4* Get_SocketMatrixPtr(const _char* _pBoneName);

private:
	/* 전이 규칙
	 * 현재 상태 (eFrom) + 입력 (eInput) + 추가 조건 (pCondition) => 다음 상태 (eTo)
	 * iPriority : 여러 규칙이 동시에 성립할 경우는 우선순위가 가장 높은 전이를 택한다.
	 */
	struct STATE_TRANSITION
	{
		IState::STATE_ID eFrom = { IState::STATE_ID::END };
		IState::TRANSITION_INPUT eInput = { IState::TRANSITION_INPUT::END };
		IState::STATE_ID eTo = { IState::STATE_ID::END };
		_uint iPriority = {};
		_bool (*pCondition)(const CPlayer&) = nullptr;
	};

	/* 이동 < 액션 < 피격 반응 < 강제 전이 */
	enum { LOCOMOTION, ACTION, REACTION, HIGHEST, END };

	_uint m_iState = {}; // TODO : deprectaed

	/* Timer */
	_float m_fLocalTimeScale = { 1.f };

	/* FSM Context */
	_bool m_TransitionInputs[ENUM_TO_UINT(IState::TRANSITION_INPUT::END)] = {};

	/* State */
	class CBody* m_pBody = { nullptr };

	/* State */
	CPlayer_IDLE*			m_IDLE;
	CPlayer_Walk*			m_WALK;
	CPlayer_Run*			m_RUN;
	CPlayer_Jump*			m_Jump;
	CPlayer_Jump_SideView*	m_Jump_SideView;
	CPlayer_Attack*			m_Attack;
	CPlayer_Shield*			m_Shield;
	CPlayer_GetITem*		m_GetItem;
	CPlayer_Boomerang*		m_Boomerang;
	CPlayer_Damage*			m_Damage;
	CPlayer_Warp_Up*		m_Warp_Up;
	CPlayer_Warp_Down*		m_Warp_Down;
	CPlayer_Sleep*			m_Sleep;
	CPlayer_Sleep_Intro*	m_Sleep_Intro;
	CPlayer_ShieldHit*		m_ShieldHit;
	CPlayer_StairDown*		m_StairDown;
	CPlayer_LadderUp_ST*	m_LadderUp_ST;
	CPlayer_LadderUp_LP*	m_LadderUp_LP;
	CPlayer_Dodge*			m_Dodge;
	CPlayer_Counter*		m_Counter;
	CPlayer_Jump_Dodge*		m_Jump_Dodge;
	CPlayer_DoorOpen*		m_pDoorOpen;

	/* InputController */
	class CPlayerInputController* m_pPlayerInputController = { nullptr };

	/* Collider */
	class CBody_Player* m_pBodyPlayer = { nullptr };
	class CWeapon_Player* m_pWeaponPlayer = { nullptr };

	/* Camera */
	_bool m_bCameraShake = { false };

	/* Event */
	_bool m_bSleepEndEvent = { false };
	_bool m_bWakeUpEvent = { false };
	_float m_fLadderTopPosY = { 0.f };
	_float m_fLadderBottomPosY = { 0.f };
	_vector m_vStairTargetPos = XMVectorZero();

	/* View Mode */
	VIEW_MODE m_eViewMode = { VIEW_MODE::TOP };

	/* Wind */
	_bool m_bInWind = { false };
	_vector m_vWindDir = XMVectorZero();
	_float m_fWindPower = { 2.f };

	/* Dodge & Counter */
	_bool m_bPerfectDodge = { false };
	DODGE m_eDogePhase = DODGE::NONE;

	/* Jump */
	_float m_fJumpPower = { 8.f };
	_float m_fGravity = { -22.f };
	_float m_fVelocityY = { 0.f };
	_float m_fGroundY = { 0.f };
	_bool m_bAir = { false };

	/* HP */
	_float m_fHP = { 100.f };

private:
	HRESULT Create_Part(CPartObject::PART_CREATE_DESC _tCreateDesc);
	HRESULT Ready_Collider();
	void Set_LocalTimeScale(_float _fTimeScale);

	void Update_Wind(_float _fTimeDelta);

	void Update_EquipMesh(_float _fTimeDelta);

public:
	static CPlayer* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END