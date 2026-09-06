#pragma once
#include "Client_Defines.h"
#include "PawnObject.h"

class CMoriblinSword_Walk;

NS_BEGIN(Engine)
class CCollider;
class CNavigation;
NS_END

NS_BEGIN(Client)
class CMonster_BuzzBlob final : public CPawnObject
{
public:
	enum class STATE_ID { IDLE, WALK, DAMAGE, END };

private:
	CMonster_BuzzBlob(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMonster_BuzzBlob(const CMonster_BuzzBlob& _Prototype);
	virtual ~CMonster_BuzzBlob() = default;

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
	CMoriblinSword_Walk* m_Walk;

	/* Target */
	class CGameObject* m_pTarget = { nullptr };

	/* HP */
	_float m_fHP = { 1.f };
	_bool m_bDelete = { false };

private:
	HRESULT Ready_Collider();
	HRESULT Ready_Navigation();

	void Take_Damage(_float _fDamage, _vector _vHitPos);

public:
	static CMonster_BuzzBlob* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END