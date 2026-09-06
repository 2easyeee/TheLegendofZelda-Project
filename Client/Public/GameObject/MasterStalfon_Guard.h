#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MasterStalfon.h"

NS_BEGIN(Client)
class CMasterStalfon_Guard : public IPawnState
{
private:
	enum class PHASE { ST, LP, ED, END };
	CMasterStalfon_Guard::PHASE m_tPhase;
	STATE_TIME tGuardTime = { false, 0.f, 1.5f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::GUARD);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("guard_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		m_tPhase = CMasterStalfon_Guard::PHASE::ST;
		tGuardTime.fAccTime = 0.f;

		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		pMonster->Collider_BodyEnable(true);
		pMonster->Collider_ShieldEnable(true);

		/* SFX */
		//CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_Shield.wav", SOUND::EFFECT);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);

		tGuardTime.fAccTime += _fTimeDelta;
		switch (m_tPhase)
		{
		case Client::CMasterStalfon_Guard::PHASE::ST:
		{
			if (_pActor->IsAnimFinished())
			{
				m_tPhase = CMasterStalfon_Guard::PHASE::LP;
				_pActor->Set_Animation("guard_lp", true);
			}
		}
			break;
		case Client::CMasterStalfon_Guard::PHASE::LP:
		{
			if (tGuardTime.fAccTime >= tGuardTime.fAccDurationTime)
			{
				m_tPhase = CMasterStalfon_Guard::PHASE::ED;
				_pActor->Set_Animation("guard_ed", false);
			}
		}
		break;
		case Client::CMasterStalfon_Guard::PHASE::ED:
		{
			pMonster->NextPattern();
			pMonster->Run_CurrentPattern();
		}
			break;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
	}
};
NS_END