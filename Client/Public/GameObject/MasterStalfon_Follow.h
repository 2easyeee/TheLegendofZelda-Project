#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MasterStalfon.h"

NS_BEGIN(Client)
class CMasterStalfon_Follow : public IPawnState
{
private:
	STATE_TIME tDecision = { false, 0.f, 0.3f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::FOLLOW);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("walk", true);
		_pActor->Set_AnimationSpeedMulti(1.f);

		tDecision.fAccTime = 0.f;

		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		pMonster->Collider_BodyEnable(true);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		auto pTarget = pMonster->Get_Target();
		if (!pTarget)
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::IDLE));
			return;
		}

		CTransform* pTransform_My = static_cast<CTransform*>(_pActor->Get_Component(TEXT("Com_Transform")));
		CTransform* pTransform_Target = static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));

		_vector vMy = pTransform_My->Get_State(STATE::POSITION);
		_vector vTarget = pTransform_Target->Get_State(STATE::POSITION);

		_vector vDir = XMVectorSetY(vTarget - vMy, 0.f);
		vDir = XMVector3Normalize(vDir);

		pMonster->MoveWithNav(vDir, _fTimeDelta);

		tDecision.fAccTime += _fTimeDelta;
		if (tDecision.fAccTime < tDecision.fAccDurationTime)
			return;
		tDecision.fAccTime = 0.f;

		_float fDist = pMonster->Compute_Distance_To_Target();
		if (fDist <= 2.5f)
		{
			_pActor->RequestToChangeState(
				ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::ATTACK));
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		pMonster->Collider_BodyEnable(false);
	}
};
NS_END