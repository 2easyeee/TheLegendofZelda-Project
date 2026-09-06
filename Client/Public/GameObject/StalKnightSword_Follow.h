#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_StalKnightSword.h"

NS_BEGIN(Client)
class CStalKnightSword_Follow : public IPawnState
{
private:
	STATE_TIME tDecision = { false, 0.f, 0.3f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::FOLLOW);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("walk", true);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(1.5f);
		_pActor->Set_SpeedMulti(1.5f);

		tDecision.fAccTime = 0.f;
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_StalKnightSword*>(_pActor);
		auto pTarget = pMonster->Get_Target();

		if (!pTarget)
			return;

		CTransform* pMy = pMonster->Get_TrasformForMove();
		CTransform* pTargetTransform =
			static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));

		_vector vMy = pMy->Get_State(STATE::POSITION);
		_vector vTarget = pTargetTransform->Get_State(STATE::POSITION);

		_vector vDir = XMVectorSetY(vTarget - vMy, 0.f);
		vDir = XMVector3Normalize(vDir);

		pMonster->MoveWithNav(vDir, _fTimeDelta);

		if (!pMonster->Get_Target())
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::IDLE));
			return;
		}

		tDecision.fAccTime += _fTimeDelta;
		if (tDecision.fAccTime >= tDecision.fAccDurationTime)
			return;
		tDecision.fAccTime = 0.f;

		_float fDist = pMonster->Compute_Distance_To_Target();
		if (fDist <= 2.f)
		{
			_float fRand = CGameInstance::GetInstance()->Random(0.f, 100.f);

			if (fRand < 60.f)
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::ATTACK));
			else
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::GUARD));
		}
		else if (fDist > 8.f)
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::IDLE));
			return;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);
	}
};
NS_END