#pragma once
#include "IActorState.h"
#include "GameObject/MoriblinSpear.h"

NS_BEGIN(Client)
class CMoriblinSpear_StanceWalk : public IActorState
{
private:
    STATE_TIME tDecision = { false, 0.f, 0.3f };

public:
    virtual _uint Get_StateID() const override
    {
        return ENUM_TO_UINT(CMoriblinSpear::STATE_ID::STANCE_WALK);
    }
    virtual void OnStateEnter(class CActorObject* _pActor) override
    {
        _pActor->Set_Animation("stance_walk_f", true);
        _pActor->Set_AnimationSpeedMulti(1.f);
        _pActor->Set_SpeedMulti(1.f);
        tDecision.fAccTime = 0.f;
    }
    virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
    {
		auto pMonster = static_cast<CMoriblinSpear*>(_pActor);
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

		pMonster->MovewithCollision(vDir, _fTimeDelta);

		if (!pMonster->Get_Target())
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::IDLE));
			return;
		}

		tDecision.fAccTime += _fTimeDelta;
		if (tDecision.fAccTime >= tDecision.fAccDurationTime)
			return;
		tDecision.fAccTime = 0.f;

		_float fDist = pMonster->Compute_Distance_To_Target();
		if (fDist <= 5.f)
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::ATTACK)); // STANCE
		}
		else if (fDist > 8.f)
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::IDLE));
			return;
		}

        /* 중간부터 빨라지기 */
        //_float LerpTime = tMoveTime.fAccTime / tMoveTime.fAccDurationTime;
        //LerpTime = max(0.f, min(1.f, LerpTime));
        //_float speed = 1.f + (2.f - 1.f) * LerpTime;

        //_pActor->Set_AnimationSpeedMulti(speed);
        //_pActor->Set_SpeedMulti(speed);
    }
    virtual void OnStateExit(class CActorObject* _pActor) override
    {
        _pActor->Reset_Animation_TrackPosition();
        _pActor->Set_AnimationSpeedMulti(1.f);
        _pActor->Set_SpeedMulti(1.f);
    }
};
NS_END