#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Togezo.h"

NS_BEGIN(Client)
class CTogezo_Rebound : public IPawnState
{
private:
	enum class PHASE { ST, LP, ED, END };
	CTogezo_Rebound::PHASE m_ePhase;
	STATE_TIME m_tStunTime = { false, 0.f, 3.f };

	/* SFX */
	STATE_TIME m_tReboundSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Togezo::STATE_ID::REBOUND);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("rebound_st", false);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);
		m_ePhase = CTogezo_Rebound::PHASE::ST;
		m_tStunTime.fAccTime = 0.f;

		/* SFX */
		m_tReboundSFX.Start(0.2f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		switch (m_ePhase)
		{
		case CTogezo_Rebound::PHASE::ST:
		{
			if (_pActor->IsAnimFinished())
			{
				_pActor->Set_Animation("stun", true);
				_pActor->Reset_Animation_TrackPosition();
				m_ePhase = CTogezo_Rebound::PHASE::LP;
			}
		}
			break;
		case CTogezo_Rebound::PHASE::LP:
		{
			m_tStunTime.fAccTime += _fTimeDelta;
			if (m_tStunTime.fAccTime >= m_tStunTime.fAccDurationTime)
			{
				_pActor->Set_Animation("stun_ed", false);
				_pActor->Reset_Animation_TrackPosition();
				m_ePhase = CTogezo_Rebound::PHASE::ED;
			}
		}
			break;
		case CTogezo_Rebound::PHASE::ED:
		{
			if (_pActor->IsAnimFinished())
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Togezo::STATE_ID::IDLE));
		}
			break;
		}

		/* SFX */
		if (m_tReboundSFX.Tick_Loop(_fTimeDelta))
		{
			CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Togezo_Rebound.wav", SOUND::EFFECT);
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);

		auto pMonster = static_cast<CMonster_Togezo*>(_pActor);
		pMonster->Collider_BodyEnable(true);
		pMonster->Collider_SensorEnable(true);
	}
};
NS_END