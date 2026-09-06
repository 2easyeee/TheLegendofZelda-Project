#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Togezo.h"

NS_BEGIN(Client)
class CTogezo_Dead : public IPawnState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Togezo::STATE_ID::DEAD);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("dead", false);
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pMonster = static_cast<CMonster_Togezo*>(_pActor);
		pMonster->Collider_BodyEnable(false);
		pMonster->Collider_SensorEnable(false);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
	}
};
NS_END