#pragma once
#include "Client_Defines.h"
#include "PawnObject.h"

class CStalKnightSword_IDLE;
class CStalKnightSword_Walk;
class CStalKnightSword_Follow;
class CStalKnightSword_Piyo;
class CStalKnightSword_Attack;
class CStalKnightSword_Guard;
class CStalKnightSword_Damage;
class CStalKnightSword_Dead;

NS_BEGIN(Engine)
class CCollider;
class CNavigation;
NS_END

NS_BEGIN(Client)
class CMonster_StalKnightSword final : public CPawnObject
{
public:
	enum class STATE_ID { IDLE, WALK, FOLLOW, PIYO, ATTACK, GUARD, DAMAGE, DEAD, END };

private:
	CMonster_StalKnightSword(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMonster_StalKnightSword(const CMonster_StalKnightSword& _Prototype);
	virtual ~CMonster_StalKnightSword() = default;

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

	/* Collision*/
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) override;

	/* Target */
	void Set_Target(class CGameObject* _pTarget) { m_pTarget = _pTarget; }
	class CGameObject* Get_Target() const { return m_pTarget; }
	_float Compute_Distance_To_Target();

private:
	/* State */
	CStalKnightSword_IDLE* m_IDLE;
	CStalKnightSword_Walk* m_Walk;
	CStalKnightSword_Follow* m_Follow;
	CStalKnightSword_Piyo* m_Piyo;
	CStalKnightSword_Attack* m_Attack;
	CStalKnightSword_Guard* m_Guard;
	CStalKnightSword_Damage* m_Damage;
	CStalKnightSword_Dead* m_Dead;

	/* Collider */
	CCollider* m_pWeaponCollider = { nullptr };
	CCollider* m_pShieldCollider = { nullptr };
	CCollider* m_pSensorCollider = { nullptr };

	/* Target */
	class CGameObject* m_pTarget = { nullptr };

	/* HP */
	_float m_fHP = { 2.f };
	_bool m_bDelete = { false };
	_bool m_bDead = { false };
	STATE_TIME m_tDeleteTime = { false , 0.f, 2.5f };

private:
	HRESULT Ready_Collider();
	HRESULT Attach_Socket();
	HRESULT Ready_Navigation();

	void Take_Damage(_float _fDamage, _vector _vHitPos);

public:
	static CMonster_StalKnightSword* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END