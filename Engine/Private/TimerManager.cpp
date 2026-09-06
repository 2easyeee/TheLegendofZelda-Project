#include "TimerManager.h"
#include "Timer.h"

CTimerManager::CTimerManager()
    : m_fTimeScale(1.f)
    , m_fHitStopScale(1.f)
    , m_fHitStopTime(0.f)
{
}

_float CTimerManager::Get_TimeDelta(const _wstring& strTimerTag)
{
    /* 전체용 */
    CTimer* pInstance = Find_Timer(strTimerTag);

    if (nullptr == pInstance)
        return 0.f;

    _float fDelta = pInstance->Get_TimeDelta();

    return fDelta * m_fTimeScale * m_fHitStopScale;
}

_float CTimerManager::Get_GlobalDelta(const _wstring& strTimerTag)
{
    /* 영향 없는 글로벌 */
    CTimer* pInstance = Find_Timer(strTimerTag);

    if (nullptr == pInstance)
        return 0.f;

    return pInstance->Get_TimeDelta();
}

HRESULT CTimerManager::Add_Timer(const _wstring& strTimerTag)
{
    CTimer* pInstance = Find_Timer(strTimerTag);
    if (nullptr != pInstance)
        return E_FAIL;

    pInstance = CTimer::Create();
    if (nullptr == pInstance)
        return E_FAIL;

    m_Timers.emplace(strTimerTag, pInstance);

    return S_OK;
}

void CTimerManager::Compute_TimeDelta(const _wstring& strTimerTag)
{
    CTimer* pInstance = Find_Timer(strTimerTag);
    if (nullptr == pInstance)
        return;

    pInstance->Update_Timer();

    /* EFFECT : HitStop */
    _float fTimeDelta = pInstance->Get_TimeDelta();
    if (m_fHitStopTime > 0.f)
    {
        m_fHitStopTime -= fTimeDelta;
        if (m_fHitStopTime <= 0.f)
        {
            m_fHitStopTime = 0.f;
            m_fHitStopScale = 1.f;
        }
    }
}

void CTimerManager::Start_HitStop(_float _fDuration, _float _fScale)
{
    m_fHitStopTime = _fDuration;
    m_fHitStopScale = _fScale;
}

void CTimerManager::Set_TimeScale(_float _fTimeScale)
{
    m_fTimeScale = _fTimeScale;
}

CTimer* CTimerManager::Find_Timer(const _wstring& strTimerTag)
{
    auto iter = m_Timers.find(strTimerTag);

    if (iter == m_Timers.end())
        return nullptr;

    return iter->second;
}

CTimerManager* CTimerManager::Create()
{
    return new CTimerManager();
}

void CTimerManager::Free()
{
    __super::Free();

    /* map release */
    for (auto& Pair : m_Timers)
        Safe_Release(Pair.second);
    m_Timers.clear();
}
