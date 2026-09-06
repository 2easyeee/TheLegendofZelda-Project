#pragma once
#include "Client_Defines.h"
#include "PawnObject.h"

class CBomber_IDLE;
class CBomber_Walk;
class CBomber_Browoff;

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CMonster_Bomber final : public CPawnObject
{
public:
	enum class STATE_ID { IDLE, WALK, BROWOFF, END };

protected:
	CMonster_Bomber(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMonster_Bomber(const CMonster_Bomber& _Prototype);
	virtual ~CMonster_Bomber() = default;

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
	class CCollider* Get_BodyCollider() const { return m_pBodyCollider; } // For. Browoff Knockback

	/* Collision*/
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) override;

	/* Target */
	void Set_Target(class CGameObject* _pTarget) { m_pTarget = _pTarget; }
	class CGameObject* Get_Target() const { return m_pTarget; }

	/* Delete */
	void Notify_Delete();
	
private:
	/* State */
	CBomber_IDLE* m_IDLE;
	CBomber_Walk* m_Walk;
	CBomber_Browoff* m_Browoff;

	/* Target */
	class CGameObject* m_pTarget = { nullptr };

	/* HP */
	_float m_fHP = { 1.f };

	/* Pre Explode */
	STATE_TIME m_tPreExplodeFlash = { false, 0.f, 1.5f };
	STATE_TIME m_tExplodeTime = { false, 0.f, 5.f };

private:
	HRESULT Ready_Collider();
	HRESULT Ready_Navigation();

public:
	static CMonster_Bomber* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END