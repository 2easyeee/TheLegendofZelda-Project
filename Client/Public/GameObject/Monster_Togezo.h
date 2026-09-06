#pragma once
#include "Client_Defines.h"
#include "PawnObject.h"

class CTogezo_IDLE;
class CTogezo_Discover;
class CTogezo_Walk;
class CTogezo_Run;
class CTogezo_Rebound;
class CTogezo_Dead;

NS_BEGIN(Client)
class CMonster_Togezo final : public CPawnObject
{
public:
	enum class STATE_ID { IDLE, DISCOVER, WALK, RUN, REBOUND, STUN, DAMAGE, DEAD, END };

protected:
	CMonster_Togezo(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMonster_Togezo(const CMonster_Togezo& _Prototype);
	virtual ~CMonster_Togezo() = default;

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
	CTogezo_IDLE* m_IDLE;
	CTogezo_Discover* m_Discover;
	CTogezo_Walk* m_Walk;
	CTogezo_Run* m_Run;
	CTogezo_Rebound* m_Rebound;
	CTogezo_Dead* m_Dead;

	/* Collider */
	CCollider* m_pSensorCollider = { nullptr };

	/* Target */
	class CGameObject* m_pTarget = { nullptr };

	/* HP */
	_float m_fHP = { 2.f };
	STATE_TIME m_tDeleteTime = { false , 0.f, 1.5f };

private:
	HRESULT Ready_Collider();
	HRESULT Ready_Navigation();

	void Take_Damage(_float _fDamage, _vector _vHitPos);

public:
	static CMonster_Togezo* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END