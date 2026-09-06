#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Togezo.h"

NS_BEGIN(Client)
class CTogezo_IDLE : public IPawnState
{
private:
	STATE_TIME m_IdleTime = { false, 0.f, 1.0f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Togezo::STATE_ID::IDLE);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("wait", false);
		_pActor->Set_AnimationSpeedMulti(1.f);

		m_IdleTime.fAccTime = 0.f;
		m_IdleTime.fAccDurationTime = 0.8f + (rand() % 70) / 100.f;
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_Togezo*>(_pActor);

		/* Follow */
		if (pMonster->Get_Target())
		{
			_float fDist = pMonster->Compute_Distance_To_Target();
			if (fDist < 5.f)
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Togezo::STATE_ID::DISCOVER));
			return;
		}

		m_IdleTime.fAccTime += _fTimeDelta;
		if (m_IdleTime.fAccTime >= m_IdleTime.fAccDurationTime)
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Togezo::STATE_ID::WALK));
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{

	}
};
NS_END