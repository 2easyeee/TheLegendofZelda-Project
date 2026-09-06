#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_IDLE : public IState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::IDLE);
	}
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{	
		_pActor->Set_Animation("idle_bare");
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Apply_Gravity(_fTimeDelta);
	}
	virtual void OnStateExit(class CActorObject* _pActor) override {}
};
NS_END