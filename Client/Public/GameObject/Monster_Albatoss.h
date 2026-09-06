#pragma once
#include "Client_Defines.h"
#include "PawnObject.h"

class CAlbatoss_Pop;
class CAlbatoss_ATK_Claw;
class CAlbatoss_ATK_Flapping;
class CAlbatoss_Damage;
class CAlbatoss_Dead;
class CAlbatoss_Hovering;
class CAlbatoss_IDLE;
class CAlbatoss_Rush;
class CAlbatoss_PiccoloWait;

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CMonster_Albatoss final : public CPawnObject
{
public:
	enum class STATE_ID { PICCOLO_WAIT, POP, IDLE, HOVERING, ATK_CLAW, ATK_FLAPPING, RUSH, DAMAGE, DEAD, END };
	enum class PATTERN { RUSH, FLAPPING, CLAW, END };

private:
	CMonster_Albatoss(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMonster_Albatoss(const CMonster_Albatoss& _Prototype);
	virtual ~CMonster_Albatoss() = default;

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
	void Collider_WindEnable(_bool _bActive);
	void Collider_ClawEnable(_bool _bActive);

	/* Collision*/
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) override;

	/* Target */
	void Set_Target(class CGameObject* _pTarget) { m_pTarget = _pTarget; }
	class CGameObject* Get_Target() const { return m_pTarget; }

	/* Feather */
	void Shoot_Feather(_vector _vTargetPos);
	void Respawn();
	
	/* Pattern */
	CMonster_Albatoss::PATTERN Get_CurrentPattern() const;
	void NextPattern();
	_uint Get_PreState() const;

	/* Check State */
	_bool IsDead() const;
	void Notify_Delete();

	/* Rush */
	_float Get_LastRushDir() const;
	void Set_LastRushDir(_float _fDir);

private:
	/* State */
	CAlbatoss_Pop* m_Pop;
	CAlbatoss_ATK_Claw* m_ATK_Claw;
	CAlbatoss_ATK_Flapping* m_ATK_Flapping;
	CAlbatoss_Damage* m_Damage;
	CAlbatoss_Dead* m_Dead;
	CAlbatoss_Hovering* m_Hovering;
	CAlbatoss_IDLE* m_IDLE;
	CAlbatoss_Rush* m_Rush;
	CAlbatoss_PiccoloWait* m_PiccoloWait;

	/* Collider */
	CCollider* m_pWindCollider = { nullptr };
	CCollider* m_pClawCollider_L = { nullptr };
	CCollider* m_pClawCollider_R = { nullptr };

	/* Target */
	class CGameObject* m_pTarget = { nullptr };

	/* HP */
	_float m_fHP = { 10.f };
	STATE_TIME m_tDeleteTime = { false, 0.f, 5.f };
	STATE_TIME m_tDissovleTime;
	STATE_TIME m_tRupee = { false, 0.f, 0.5f };

	/* Feather */
	class CAlbatoss_Feather* m_pFeather = { nullptr };
	class CSpear* m_pSpear = { nullptr };

	/* Pattern */
	vector<PATTERN> m_PatternQueue = {
		PATTERN::RUSH,
		PATTERN::FLAPPING,
		PATTERN::CLAW
	};
	_int m_iPatternIndex = { 0 };
	_uint m_iPreState = { 0 };

	/* Rush */
	_float m_fLastRushDir = { 1.f };

private:
	HRESULT Ready_Collider();
	HRESULT Attach_Socket();

	void Take_Damage(_float _fDamage, _vector _vHitPos);
	void Drop_Rupee();
	void Spawn_Rupee(_vector vPos, _vector vDir);
	virtual void Start_DeleteVFX() override;

public:
	static CMonster_Albatoss* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END