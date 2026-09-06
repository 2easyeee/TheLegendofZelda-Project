#include "EventManager.h"

CEventManager::CEventManager()
{
}

void CEventManager::Update(_float _fTimeDelta)
{

}

void CEventManager::Push_UIEVENT(UI_EVENT& _tUIEvent)
{
    m_UIevents.push(_tUIEvent);
}

_bool CEventManager::Pop_UIEvent(UI_EVENT& _tUIEvent)
{
    if (m_UIevents.empty())
        return false;

    _tUIEvent = m_UIevents.front();
    m_UIevents.pop();

    return true;
}

void CEventManager::Push_CameraEVENT(CAMERA_EVENT& _tCameraEvent)
{
    m_CameraEvents.push(_tCameraEvent);
}

_bool CEventManager::Pop_CameraEVENT(CAMERA_EVENT& _tCameraEvent)
{
    if (m_CameraEvents.empty())
        return false;

    _tCameraEvent = m_CameraEvents.front();
    m_CameraEvents.pop();

    return true;
}

void CEventManager::Push_GameEVENT(GAME_EVENT& _tGameEvent)
{
    m_GameEvents.push(_tGameEvent);
}

_bool CEventManager::Pop_GameEVENT(GAME_EVENT& _tGameEvent)
{
    if (m_GameEvents.empty())
        return false;

    _tGameEvent = m_GameEvents.front();
    m_GameEvents.pop();

    return true;
}

CEventManager* CEventManager::Create()
{
    return new CEventManager();
}

void CEventManager::Free()
{
    __super::Free();
}
