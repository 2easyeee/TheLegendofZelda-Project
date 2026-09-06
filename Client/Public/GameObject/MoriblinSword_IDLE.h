#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MoriblinSword.h"

NS_BEGIN(Client)
class CMoriblinSword_IDLE : public IPawnState
{
private:
	STATE_TIME tIdleTime = { false, 0.f, 2.5f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MoriblinSword::STATE_ID::IDLE);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("wait", true);
		_pActor->Set_AnimationSpeedMulti(1.f);
		tIdleTime.fAccTime = 0.f;
		tIdleTime.fAccDurationTime = 1.5f + (rand() % 200) * 0.01f;
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_MoriblinSword*>(_pActor);

		/* Follow */
		_float fDist = pMonster->Compute_Distance_To_Target();
		if (fDist < 8.f)
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_MoriblinSword::STATE_ID::FOLLOW));
		else
		{
			tIdleTime.fAccTime += _fTimeDelta;
			if (tIdleTime.fAccTime < tIdleTime.fAccDurationTime)
				return;

			tIdleTime.fAccTime = 0.f;

			_float fRand = CGameInstance::GetInstance()->Random(0.f, 100.f);

			if (fRand < 60.f)
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_MoriblinSword::STATE_ID::WALK));
			else
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_MoriblinSword::STATE_ID::KYOROKYORO));

			return;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{

	}
};
NS_END