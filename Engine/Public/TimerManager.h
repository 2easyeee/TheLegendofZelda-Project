#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CTimerManager final : public CBase
{
private:
	CTimerManager();
	virtual ~CTimerManager() = default;

public:
	_float Get_TimeDelta(const _wstring& strTimerTag);
	_float Get_GlobalDelta(const _wstring& strTimerTag);

	HRESULT Add_Timer(const _wstring& strTimerTag);
	void Compute_TimeDelta(const _wstring& strTimerTag);

	/* EFFECT */
	void Start_HitStop(_float _fDuration, _float _fScale = 0.f);
	void Set_TimeScale(_float _fTimeScale);
	
private:
	map<const _wstring, class CTimer*> m_Timers;

	/* EFFECT */
	_float m_fTimeScale = { 1.f };
	_float m_fLocalTimeScale = { 1.f };

	_float m_fHitStopScale = { 1.f };
	_float m_fHitStopTime = { 0.f };
	
private:
	class CTimer* Find_Timer(const _wstring& strTimerTag);
	
public:
	static CTimerManager* Create();
	virtual void Free();
};
NS_END