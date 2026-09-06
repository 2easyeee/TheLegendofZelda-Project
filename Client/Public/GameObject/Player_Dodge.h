#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Dodge : public IState
{
private:
	STATE_TIME m_tDodgeTime = { false, 0.f, 0.25f };
	_bool m_bFinished = { false };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::DODGE);
	}
	virtual _bool Is_StateFinished() const override { return m_bFinished; }
	virtual _bool Block_Locomotion() const override
	{
		return true;
	}
	virtual void OnStateEnter(class CActorObject* _pActor)
	{
		m_tDodgeTime.fAccTime = 0.f;
		m_bFinished = false;
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta)
	{
		m_tDodgeTime.fAccTime += _fTimeDelta;

		auto pPlayer = static_cast<CPlayer*>(_pActor);



		/* Counter Input */
		//if (pPlayer->Is_ActionPressed(IState::TRANSITION_INPUT::ATTACK_PRESSED))
		//{
		//	_pActor->RequestToChangeState(ENUM_TO_UINT(STATE_ID::COUNTER));
		//	return;
		//}


		/* Dodge End */
		if (m_tDodgeTime.fAccTime >= m_tDodgeTime.fAccDurationTime)
		{
			m_bFinished = true;
			//_pActor->RequestToChangeState(ENUM_TO_UINT(STATE_ID::IDLE));
		}
	}
	virtual void OnStateExit(CActorObject* _pActor)
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
		CGameInstance::GetInstance()->Set_TimeScale(1.f);
	}
};
NS_END