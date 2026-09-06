#pragma once
#include "Client_Defines.h"
#include "ActorObject.h"
#include "IActorState.h"

class CMoriblinSpear_IDLE;
class CMoriblinSpear_Attack;
class CMoriblinSpear_WALK;
class CMoriblinSpear_Find;
class CMoriblinSpear_Look;
class CMoriblinSpear_Stance;
class CMoriblinSpear_StanceWalk;
class CMoriblinSpear_Damage;
class CMoriblinSpear_Dead;

NS_BEGIN(Engine)
class CActorObject;
class CBody;
NS_END

NS_BEGIN(Client)
class CMoriblinSpear final : public CActorObject
{
public:
	enum class STATE_ID { IDLE, WALK, FIND, LOOK, STANCE, STANCE_WALK, ATTACK, DAMAGE, DEAD, END };

private:
	CMoriblinSpear(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMoriblinSpear(const CMoriblinSpear& _Prototype);
	virtual ~CMoriblinSpear() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	/* State */
	void Register_State(class IActorState* _pState);
	virtual void RequestToChangeState(_uint _stateID) override;
	void Change_State();

	/* Transform */
	void Set_SpeedMulti(_float _fMulti);
	CTransform* Get_TrasformForMove() { if (m_pTransformCom) return m_pTransformCom; }

	/* Animation (Child) */
	virtual void Set_Animation(_string _AnimName, _bool _bLoop = true, _bool _isForce = false) override;
	virtual void Reset_Animation_TrackPosition() override;
	virtual void Set_AnimationSpeedMulti(_float _fAnimSpeedMulti) override;
	virtual _bool IsAnimFinished() const override;
	virtual _float Get_PlayRatio() const override;

	/* Moriblin */
	void Notify_WeaponThrow();
	void Notify_WeaponRespawn();
	void Set_SavedTargetPosition(_vector _vTargetPos);;
	_bool Is_Throwing() { return m_bIsThrowing; }
	
	/* Collider */
	void Collider_BodyEnable(_bool _bActive);
	void Collider_SensorEnable(_bool _bActive);

	/* Collision*/
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) override;

	/* Target */
	void Set_Target(class CGameObject* _pTarget) { m_pTarget = _pTarget; }
	class CGameObject* Get_Target() const { return m_pTarget; }
	_float Compute_Distance_To_Target();

	void Set_AttackTargetPos(_vector _vPosition) { m_vAttackTargetPos = _vPosition; }
	_vector Get_AttackTargetPos() const { return m_vAttackTargetPos; }

	/* Shader Effect */
	void Start_HitFlash();
	void Start_Dissolve();

private:
	_uint m_iState = {}; // TODO : deprectaed

	/* State */
	class CBody* m_pBody = { nullptr };

	/* State */
	CMoriblinSpear_IDLE* m_IDLE;
	CMoriblinSpear_Attack* m_Attack;
	CMoriblinSpear_WALK* m_Walk;
	CMoriblinSpear_Find* m_Find;
	CMoriblinSpear_Look* m_Look;
	CMoriblinSpear_Stance* m_Stance;
	CMoriblinSpear_StanceWalk* m_StanceWalk;
	CMoriblinSpear_Damage* m_Damage;
	CMoriblinSpear_Dead* m_Dead;

	/* State */
	class IActorState* m_pState = { nullptr };
	map<_uint, class IActorState*> m_StateMap;
	_uint m_NexState = { 0 };
	_bool m_bRequestStateChange = { false };

	/* Collider */
	class CBody_MoriblinSpear* m_pBodyMoriblin = { nullptr };

	/* Target */
	class CGameObject* m_pTarget = { nullptr };
	_vector m_vAttackTargetPos;
	_bool m_bIsThrowing = { false };

	/* HP */
	_float m_fHP = { 2.f };
	_bool m_bDelete = { false };
	_bool m_bDead = { false };
	STATE_TIME m_tDeleteTime = { false , 0.f, 2.5f };

	/* EFFECT */
	STATE_TIME m_tDeleteVFX = { false, 0.f, 0.6f };

private:
	HRESULT Create_Part(CPartObject::PART_CREATE_DESC _tCreateDesc);
	HRESULT Ready_Collider();
	HRESULT Ready_Navigation();

	void Take_Damage(_float _fDamage, _vector _vHitPos);
	void Start_DeleteVFX();

public:
	static CMoriblinSpear* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END