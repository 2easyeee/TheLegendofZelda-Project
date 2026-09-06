#pragma once
#include "Client_Defines.h"
#include "PawnObject.h"

class CMasterStalfon_IDLE;
class CMasterStalfon_Walk;
class CMasterStalfon_Look;
class CMasterStalfon_Attack;
class CMasterStalfon_Damage;
class CMasterStalfon_Down;
class CMasterStalfon_Dead;
class CMasterStalfon_Pop;
class CMasterStalfon_Escape;
class CMasterStalfon_Guard;
class CMasterStalfon_Jump;
class CMasterStalfon_KyoroKyro;
class CMasterStalfon_Follow;
class CMasterStalfon_Teleport;
class CMasterStalfon_Pop_Wait;

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CMonster_MasterStalfon final : public CPawnObject
{
public:
	enum class STATE_ID { POP_WAIT, POP, IDLE, WALK, FOLLOW, LOOK, KYOROKYORO, ATTACK, GUARD, JUMP, TELEPORT, DAMAGE, DOWN, ESCAPE, DEAD, END };
	enum class PATTERN { GUARD, ATTACK, JUMP, ESCAPE, TELEPORT };

protected:
	CMonster_MasterStalfon(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMonster_MasterStalfon(const CMonster_MasterStalfon& _Prototype);
	virtual ~CMonster_MasterStalfon() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	/* Collider */
	void Collider_BodyEnable(_bool _bActive);
	void Collider_WeaponEnable(_bool _bActive);
	void Collider_ShieldEnable(_bool _bActive);
	void Collider_SensorEnable(_bool _bActive);
	void Collider_AttackWindowEnable(_bool _bActive);

	/* Render */
	void Render_Enable(_bool _bActive);

	/* Collision*/
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) override;

	/* Target */
	void Set_Target(class CGameObject* _pTarget) { m_pTarget = _pTarget; }
	class CGameObject* Get_Target() const { return m_pTarget; }
	_float Compute_Distance_To_Target();

	/* Pattern */
	CMonster_MasterStalfon::PATTERN Get_CurrentPattern() const;
	void NextPattern();
	void Run_CurrentPattern();
	_uint Get_PreState() const;

	/* Camera */
	void Notify_CameraShake();
	_bool Check_CameraShake();

private:
	/* State */
	CMasterStalfon_IDLE* m_IDLE;
	CMasterStalfon_Walk* m_Walk;
	CMasterStalfon_Look* m_Look;
	CMasterStalfon_Attack* m_Attack;
	CMasterStalfon_Damage* m_Damage;
	CMasterStalfon_Down* m_Down;
	CMasterStalfon_Dead* m_Dead;
	CMasterStalfon_Pop* m_Pop;
	CMasterStalfon_Escape* m_Escape;
	CMasterStalfon_Guard* m_Guard;
	CMasterStalfon_Jump* m_Jump;
	CMasterStalfon_KyoroKyro* m_KyoroKyro;
	CMasterStalfon_Follow* m_Follow;
	CMasterStalfon_Teleport* m_Teleport;
	CMasterStalfon_Pop_Wait* m_Pop_Wait;

	/* Collider */
	CCollider* m_pWeaponCollider = { nullptr };
	CCollider* m_pShieldCollider = { nullptr };
	CCollider* m_pSensorCollider = { nullptr };
	CCollider* m_pAttackWindowCollider = { nullptr };

	/* Target */
	class CGameObject* m_pTarget = { nullptr };

	/* HP */
	_float m_fHP = { 20.f };
	STATE_TIME m_tDeleteTime = { false, 0.f, 3.5f };

	/* Pattern */
	vector<PATTERN> m_PatternQueue = {
		PATTERN::GUARD,
		PATTERN::ATTACK,
		PATTERN::ATTACK,
		PATTERN::JUMP,
		PATTERN::JUMP,
		PATTERN::ESCAPE,
		PATTERN::ESCAPE,
		PATTERN::ESCAPE,
	};
	_int m_iPatternIndex = { 0 };
	_uint m_iPreState = { 0 };

	/* Render */
	_bool m_bRenderActive = { true };

	/* Effect */
	_bool m_bDissolveStarted = { false };
	_bool m_bCameraShake = { false };
	STATE_TIME m_tDeleteSFX;

private:
	HRESULT Ready_Collider();
	HRESULT Attach_Socket();
	HRESULT Ready_Navigation();

	void Take_Damage(_float _fDamage, _vector _vHitPos);
	void Drop_Rupee();
	void Spawn_Rupee(_vector vPos, _vector vDir);
	virtual void Start_DeleteVFX() override;

public:
	static CMonster_MasterStalfon* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END