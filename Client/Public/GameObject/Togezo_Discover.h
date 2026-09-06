#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Togezo.h"

NS_BEGIN(Client)
class CTogezo_Discover : public IPawnState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Togezo::STATE_ID::DISCOVER);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("discover", false);
		_pActor->Set_AnimationSpeedMulti(2.f);
		_pActor->Set_SpeedMulti(1.5f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		if (_pActor->IsAnimFinished())
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Togezo::STATE_ID::RUN));
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);
	}
};
NS_END