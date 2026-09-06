#pragma once
#include "Client_Defines.h"
#include "PawnObject.h"

class CMoriblinSword_Attack;
class CMoriblinSword_Damage;
class CMoriblinSword_Dead;
class CMoriblinSword_Find;
class CMoriblinSword_Follow;
class CMoriblinSword_Guard;
class CMoriblinSword_IDLE;
class CMoriblinSword_KyoroKyoro;
class CMoriblinSword_Walk;
class CMoriblinSword_Piyo;

NS_BEGIN(Engine)
class CCollider;
class CNavigation;
NS_END

NS_BEGIN(Client)
class CMonster_MoriblinSword final : public CPawnObject
{
public:
	enum class STATE_ID { IDLE, WALK, FIND, FOLLOW, KYOROKYORO, PIYO, ATTACK, GUARD, DAMAGE, DEAD, END };

private:
	CMonster_MoriblinSword(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMonster_MoriblinSword(const CMonster_MoriblinSword& _Prototype);
	virtual ~CMonster_MoriblinSword() = default;

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
	CMoriblinSword_Attack* m_Attack;
	CMoriblinSword_Damage* m_Damage;
	CMoriblinSword_Dead* m_Dead;
	CMoriblinSword_Find* m_Find;
	CMoriblinSword_Follow* m_Follow;
	CMoriblinSword_Guard* m_Guard;
	CMoriblinSword_IDLE* m_IDLE;
	CMoriblinSword_KyoroKyoro* m_KyoroKyoro;
	CMoriblinSword_Walk* m_Walk;
	CMoriblinSword_Piyo* m_Piyo;

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
	static CMonster_MoriblinSword* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END