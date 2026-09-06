#pragma once
#include "Base.h"
#include "Effect.h"
#include "TrailEffect.h"

NS_BEGIN(Engine)
class CEffectManager final : public CBase
{
public:
	enum class EFFECT_EVENT_TYPE {
		SPAWN_EFFECT, TRAIL,
	};
	typedef struct tagEffectEvent
	{
		EFFECT_EVENT_TYPE eType;
		_wstring EffectName;
		_vector vPosition;
		_vector vDirection;
		_float fDuration;
		_float fScale;

		/* Trail */
		const _float4x4* pParentMatrix = { nullptr };
		const _float4x4* pBaseSocketMatrix = { nullptr };
		const _float4x4* pTipSocketMatrix = { nullptr };
;	}EFFECT_EVENT;

private:
	CEffectManager();
	virtual ~CEffectManager() = default;

public:
	HRESULT Initialize();
	void Update(_float _fTimeDelta);
	
public:
	void Push_EffectEVENT(EFFECT_EVENT& _tEffectEvent);
	_bool Pop_EffectEVENT(EFFECT_EVENT& _tEffectEvent);

	HRESULT Register_EffectEvent(_wstring _EventName, CEffect::EFFECT_INIT_DESC _tDesc);
	HRESULT Register_EffectEvent(_wstring _EventName, CTrailEffect::TRAIL_INIT_DESC _tDesc);

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	/* Event */
	queue<EFFECT_EVENT> m_EffectEvents;

	/* Register */
	map<_wstring, CEffect::EFFECT_INIT_DESC> m_EffectRegistry;
	map<_wstring, CTrailEffect::TRAIL_INIT_DESC> m_TrailEffectRegistry;

public:
	static CEffectManager* Create();
	virtual void Free() override;
};
NS_END