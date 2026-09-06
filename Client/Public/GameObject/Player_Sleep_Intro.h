#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Sleep_Intro : public IState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::SLEEP_INTRO);
	}
	virtual _bool Block_Locomotion() const override
	{
		return true;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		_pActor->Set_Animation("ev_sleep_lp", true);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{

	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		/* SFX */
		CGameInstance::GetInstance()->Play_Sound(L"Player_SleepEvent_Talk.wav", SOUND::EFFECT);
	}
};
NS_END