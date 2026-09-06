#pragma once
#include "IActorState.h"
#include "GameObject/MoriblinSpear.h"

NS_BEGIN(Client)
class CMoriblinSpear_IDLE : public IActorState
{
private:
	STATE_TIME tIdleTime = { false, 0.f, 2.5f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMoriblinSpear::STATE_ID::IDLE);
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		_pActor->Set_Animation("wait", true);
		_pActor->Set_AnimationSpeedMulti(1.f);
		tIdleTime.fAccTime = 0.f;
		tIdleTime.fAccDurationTime = 1.5f + (rand() % 200) * 0.01f;
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMoriblinSpear*>(_pActor);

		/* Follow */
		_float fDist = pMonster->Compute_Distance_To_Target();
		if (fDist < 8.f && !pMonster->Is_Throwing())
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::FIND));
		else
		{
			tIdleTime.fAccTime += _fTimeDelta;
			if (tIdleTime.fAccTime < tIdleTime.fAccDurationTime)
				return;

			tIdleTime.fAccTime = 0.f;

			_float fRand = CGameInstance::GetInstance()->Random(0.f, 100.f);

			if (fRand < 60.f)
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::WALK));
			else
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::LOOK));

			return;
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override {}
};
NS_END