#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Run : public IState
{
public:
	virtual STATE_ID Get_StateID() const override
	{
		return STATE_ID::RUN;
	}

	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		_pActor->Set_Animation("run_bare");
		_pActor->Set_AnimationSpeedMulti(3.f);
		_pActor->Set_SpeedMulti(5.f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_SpeedMulti(1.f);
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END