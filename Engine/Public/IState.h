#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class IState
{
public:
	enum class STATE_ID { 
		IDLE, WALK, RUN, ATTACK, 
		JUMP, JUMP_SIDEVIEW, JUMP_DODGE,
		SHIELD,
		GET_ITEM, 
		BOOMERANG, DASH,
		LADDER_UP_ST, LADDER_UP_LP,
		STAIR_DOWN, 
		WARP_UP, WARP_DOWN,
		SLEEP, SLEEP_INTRO, TALK, DOOR_OPEN,
		SHIELD_HIT, DAMAGE, 
		DODGE, COUNTER, END };

	/* FSM 전이 입력 */
	enum class TRANSITION_INPUT {
		/* 이동 */
		MOVE_IDLE, MOVE_WALK, MOVE_RUN,
		
		/* 조작 */
		ATTACK_PRESSED,
		SHIELD_PRESSED,
		JUMP_SIDEVIEW_PRESSED,
		JUMP_DODGE_PRESSED,
		PERFECT_DODGE,
		BOOMERANG_PRESSED,

		/* 외부 반응 */
		DAMAGE_RECEVICED,
		SHIELD_HIT_RECEIVED,

		/* 필드 상호작용 */
		GET_ITEM_REQUESTED,
		DOOR_OPEN_REQUESTED,
		WARP_UP_REQUESTED,
		WARP_DOWN_REQESTED,
		WARP_DOWN_FINISHED,
		SLEEP_REQUESTED,
		LADDER_UP_REQUESTED,

		/* 내부 동작 완료 입력 */
		DODGE_FINISHED,
		LADDER_INTRO_FINISHED,
		LADDER_FINISHED,
		SHIELD_RESUME,
		SLEEP_FINISHED,
		DOOR_OPEN_FINISHED,
		GET_ITEM_FINISHED,

		END
	};
	 
public:
	virtual ~IState() = default;

public:
	virtual _uint Get_StateID() const = 0;
	virtual _bool Block_Locomotion() const { return true; }
	virtual _bool Is_StateFinished() const { return false; }
	virtual void OnStateEnter(class CActorObject* _pActor) = 0;
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) = 0;
	virtual void OnStateExit(class CActorObject* _pActor) = 0;
};
NS_END