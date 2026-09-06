#pragma once
#include "Base.h"
#include "GameObject.h"
#include "Level.h"

NS_BEGIN(Engine)
class CEventManager final : public CBase
{
public:
	enum class UIEVENTTYPE {
		ALL_OFF, INIT_ON,
		RUPEE_ADD, RUPEE_SPEND, HP_ADD, HP_DAMAGE, HP_ADD_MAXHP,
		INVENTORY_ON, INVENTORY_OFF,
		INVENTORY_ADD_ITEM, INVENTORY_REMOVE_ITEM, INVENTORY_USE_ITEM,
		INVENTORY_MOVE_CURSOR, INVENTORY_SELECTED_SLOT,
		DIALOGUE_ALL_OFF, DIALOGUE_MESSAGEBOX_ON, DIALOGUE_GETMESSAGEBOX_ON,
		DIALOGUE_CHOICE_ON, DIALOGUE_CHOICE_OFF, DIALOGUE_NEXT, END
	};
	typedef struct tagUIEvent
	{
		UIEVENTTYPE eUIEvent;
		DIR eUIDir;
		EQUIPBTN eEquipBtn;
		_int iValue = 0;
		_float fValue = 0.f;
		_wstring wstrText;
	}UI_EVENT;

public:
	enum class CAMERA_EVENT_TYPE {
		FOLLOW_TARGET, RELEASE_TARGET,
		SHAKE, HIT_ZOOM, // 순간 줌
		MOVE_TO_POS, LOOK_AT_POS, // 특정 위치 이동, 특정 위치 바라보기
		LOCK_AXIS, UNLOCK_AXIS, // 축 고정
		START_CUTSCENE_CAM, END_CUTSCENE_CAM,
		DIALOGUE, DIALOGUE_END,
		END
	};
	typedef struct tagCameraEvent
	{
		CAMERA_EVENT_TYPE eType;
		CGameObject* pTarget = { nullptr };
		_vector vPos = XMVectorZero();
		_float fValue1 = { 0.f };
		_float fValue2 = { 0.f };
		_wstring wstrValue = { TEXT("") };
	}CAMERA_EVENT;

public:
	enum class GAME_EVENT_TYPE {
		LEVEL_CHANGE, DIALOGUE_START,
		CUTSCENE_START, CUTSCENE_END,
		DOOR_OPEN,
		SPAWN_MONSTER, SPAWN_BOSS,
		ROOM_TRANSITION, SPAWN_TOWER,
		STAIR, LADDER_UP, LADDER_DOWN,
		BOX_OPEN, BOX_APPEAR,
		PLAY_SOUND,
		PLAYER_DAMAGE,
		INPUT_LOCK,
		NAV_ATTACH,
		GRASS_CUT,
		END
	};
	typedef struct tagGameEvent
	{
		GAME_EVENT_TYPE eType;
		CGameObject* pSender = { nullptr };
		_vector vPos = XMVectorZero();
		_float fValue1 = { 0.f };
		_float fValue2 = { 0.f };
		_int iValue = { 0 };
	}GAME_EVENT;

private:
	CEventManager();
	virtual ~CEventManager() = default;

public:
	void Update(_float _fTimeDelta);

	void Push_UIEVENT(UI_EVENT& _tUIEvent);
	_bool Pop_UIEvent(UI_EVENT& _tUIEvent);

	void Push_CameraEVENT(CAMERA_EVENT& _tCameraEvent);
	_bool Pop_CameraEVENT(CAMERA_EVENT& _tCameraEvent);

	void Push_GameEVENT(GAME_EVENT& _tGameEvent);
	_bool Pop_GameEVENT(GAME_EVENT& _tGameEvent);

private:
	queue<UI_EVENT>			m_UIevents;
	queue<CAMERA_EVENT>		m_CameraEvents;
	queue<GAME_EVENT>		m_GameEvents;

public:
	static CEventManager* Create();
	virtual void Free() override;
};
NS_END