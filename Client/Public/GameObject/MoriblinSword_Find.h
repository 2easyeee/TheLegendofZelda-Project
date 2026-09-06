#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MoriblinSword.h"

NS_BEGIN(Client)
class CMoriblinSword_Find : public IPawnState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MoriblinSword::STATE_ID::FIND);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("find", false);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(2.5f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		if (_pActor->IsAnimFinished())
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_MoriblinSword::STATE_ID::FOLLOW));
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{

	}
};
NS_END