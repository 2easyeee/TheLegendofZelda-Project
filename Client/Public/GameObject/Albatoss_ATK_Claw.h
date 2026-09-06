#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Albatoss.h"

NS_BEGIN(Client)
class CAlbatoss_ATK_Claw : public IPawnState
{
private:
	enum class PHASE { ST, LP, ED, NONE, END };
	PHASE m_ePhase;
	_float m_fDir = 1.f;

	/* Claw */
	STATE_TIME m_tClawTime = { false, 0.f, 3.f };

	/* SFX */
	STATE_TIME m_tClawSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::ATK_CLAW);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("attack_claw_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		m_ePhase = PHASE::ST;
		m_tClawTime.fAccTime = 0.f;
		
		/* Rotation */
		auto pTransform = _pActor->Get_TrasformForMove();
		_vector vPos = pTransform->Get_State(STATE::POSITION);
		_float fX = XMVectorGetX(vPos);

		m_fDir = (fX < 0.f) ? 1.f : -1.f;

		if (m_fDir > 0.f)
			pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), -90.f);
		else
			pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 90.f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
		auto pTransform = _pActor->Get_TrasformForMove();

		if (m_ePhase == PHASE::ST)
		{ 
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::LP;
				_pActor->Set_Animation("attack_claw", true);
				pMonster->Collider_ClawEnable(true);

				m_tClawSFX.Start(0.9f);
			}
		}
		else if (m_ePhase == PHASE::LP)
		{
			/* SFX */
			if (m_tClawSFX.Tick_Loop(_fTimeDelta))
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_Albatoss_Claw.wav", SOUND::EFFECT, 2.f);

			m_tClawTime.fAccTime += _fTimeDelta;
			if (m_tClawTime.fAccTime >= m_tClawTime.fAccDurationTime)
			{
				m_ePhase = PHASE::ED;
				_pActor->Set_Animation("attack_claw_ed", false);
			}
		}
		else if (m_ePhase == PHASE::ED)
		{
			pMonster->Collider_ClawEnable(false);
			pMonster->NextPattern();
			_pActor->RequestToChangeState(
				ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::HOVERING));
			return;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
	}
};
NS_END