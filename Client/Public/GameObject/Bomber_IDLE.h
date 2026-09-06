#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Bomber.h"

NS_BEGIN(Client)
class CBomber_IDLE : public IPawnState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Bomber::STATE_ID::IDLE);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("wait", true);
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
	}
};
NS_END