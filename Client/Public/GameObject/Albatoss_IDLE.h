#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Albatoss.h"

NS_BEGIN(Client)
class CAlbatoss_IDLE : public IPawnState
{
private:
	STATE_TIME tIdleTime = { false, 0.f, 2.f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::IDLE);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("weak_hovering", true);
		tIdleTime.fAccTime = 0.f;

		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
		pMonster->Collider_BodyEnable(false);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		tIdleTime.fAccTime += _fTimeDelta;

		if (tIdleTime.fAccTime >= tIdleTime.fAccDurationTime)
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::HOVERING));
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{

	}
};
NS_END