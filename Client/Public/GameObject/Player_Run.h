#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Run : public IState
{
private:
	STATE_TIME m_tFootStepSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::RUN);
	}
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		_pActor->Set_Animation("run_bare");
		_pActor->Set_AnimationSpeedMulti(2.f);
		_pActor->Set_SpeedMulti(5.f);

		m_tFootStepSFX.Start(0.2f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Apply_Gravity(_fTimeDelta);

		/* SFX */
		if (m_tFootStepSFX.Tick_Loop(_fTimeDelta))
		{
			CGameInstance::GetInstance()->Play_RandomSound(L"Player_Footstep_Dungeon", 3);
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_SpeedMulti(1.f);
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END