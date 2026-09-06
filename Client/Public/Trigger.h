#pragma once
#include "Client_Defines.h"
#include "WorldObject.h"

NS_BEGIN(Engine)
class CCollider;
class CTransform;
NS_END

NS_BEGIN(Client)
class CTrigger final : public CWorldObject
{
public:
	enum class TRIGGERTYPE 
	{ 
		LEVEL_CHANGE, DIALOGUE_START,
		CUTSCENE_START, CUTSCENE_END,
		DOOR_OPEN, 
		SPAWN_MONSTER, SPAWN_BOSS,
		ROOM_TRANSITION, SPAWN_TOWER,
		STAIR, LADDER_UP, LADDER_DOWN,
		BOX_OPEN, BOX_APPEAR,
		NAV_ATTACH,
		END 
	};

	typedef struct tagTriggerDesc
	{
		_float3 vPosition = { 0.f, 0.f, 0.f };
		_float3 vScale = { 0.5f, 0.5f, 0.5f};

		TRIGGERTYPE eType;
		_uint iValue;
		_float3 vTargetPosition = { 0.f, 0.f, 0.f };
	}TRIGGER_DESC;

private:
	CTrigger(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CTrigger(const CTrigger& _Prototype);
	virtual ~CTrigger() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual void OnTriggerEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnTriggerStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnTriggerExit(class CCollider* _pSrc, class CCollider* _Dst) override;

private:
	/* Com */
	CCollider* m_pColliderCom = {nullptr};

	_bool m_bTriggered = { false };

	/* Desc */
	TRIGGERTYPE m_eType; // For. 판단
	_uint m_iValue; // For. 정보전달
	_float3 m_vTargetPosition; // For. 정보전달

	/* Input */
	_bool m_bPlayerInRange = { false };

private:
	HRESULT Ready_Collider(_float3 _vExtents);

public:
	static CTrigger* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END