#pragma once
#include "IState.h"

NS_BEGIN(Client)
class CPlayer_Sleep : public IState
{
private:
	enum class PHASE { LP, ED, END };
	PHASE m_ePhase;
	STATE_TIME tStateTime = { false, 0.f, 2.f };

	/* SFX */
	STATE_TIME m_tOutOfBedSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::SLEEP);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }
	virtual _bool Block_Locomotion() const override
	{
		return true;
	}
	virtual void OnStateEnter(CActorObject* _pActor)
	{
		_pActor->Set_Animation("ev_sleep_lp", false);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(1.f);
		m_ePhase = PHASE::LP;
		tStateTime.fAccTime = 0.f;

		/* SFX */
		m_tOutOfBedSFX.Start(0.1f);
	}
	virtual void OnStateStay(CActorObject* _pActor, _float _fTimeDelta)
	{
		if (m_ePhase == PHASE::LP)
		{
			tStateTime.fAccTime += _fTimeDelta;
			if (tStateTime.fAccTime >= tStateTime.fAccDurationTime)
			{
				m_ePhase = PHASE::ED;
				_pActor->Set_Animation("ev_outbed", false);

				auto pPlayer = static_cast<CPlayer*>(_pActor);
				pPlayer->Set_WakeUpEvent();
			}
		}
		else if (m_ePhase == PHASE::ED)
		{
			_float fRatio = _pActor->Get_PlayRatio();
			if (fRatio >= 0.7f)
			{
				if (m_tOutOfBedSFX.Tick(_fTimeDelta))
				{
					/* SFX */
					CGameInstance::GetInstance()->Play_Sound(L"Player_OutOfBed_Talk.wav", SOUND::EFFECT);
				}
			}

			if (_pActor->IsAnimFinished())
			{
				auto pPlayer = static_cast<CPlayer*>(_pActor);
				pPlayer->Set_SleepEndEvent(); // Excep. Sleep 은 일부러 넘겨줘야함.

				m_ePhase = PHASE::END;
			}
		}
		return;
	}
	virtual void OnStateExit(CActorObject* _pActor)
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END