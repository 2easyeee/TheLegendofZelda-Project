#pragma once
#include "IState.h"

NS_BEGIN(Client)
class CPlayer_Attack : public IState
{
private:
	enum class PHASE { WAIT, TAP, CHARGE_ST, CHARGE_LP, CHARGE_ED, END };
	PHASE m_ePhase = { PHASE::TAP };
	STATE_TIME tChargeTime = { false, 0.f, 0.12f };

	/* Effect */
	_bool m_bTrailSpawned = { false };
	_bool m_bTrailSpawned_Charge = { false };

	/* SFX */
	STATE_TIME m_tTabSFX;
	STATE_TIME m_tChargeSTSFX;
	STATE_TIME m_tChargeEDSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::ATTACK);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor)
	{
		/* Àç»ç¿ë */
		tChargeTime.fAccTime = 0.f;
		m_ePhase = PHASE::WAIT;

		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Collider_SwordEnable(false);
	
		/* SFX */
		m_tTabSFX.Start(0.1f);
		m_tChargeSTSFX.Start(0.2f);
		m_tChargeEDSFX.Start(0.2f);
	}

	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta)
	{
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Apply_Gravity(_fTimeDelta);

		switch (m_ePhase)
		{
		case PHASE::WAIT:
		{
			if (_pActor->Is_ActionPressed(IState::TRANSITION_INPUT::ATTACK_PRESSED))
			{
				tChargeTime.fAccTime += _fTimeDelta;
				if (tChargeTime.fAccTime >= tChargeTime.fAccDurationTime)
				{
					m_ePhase = PHASE::CHARGE_ST;
					_pActor->Set_Animation("slash_hold_st", false);
					_pActor->Set_AnimationSpeedMulti(5.f);
				}
			}
			else
			{
				m_ePhase = PHASE::TAP;
				_pActor->Set_Animation("slash", false, true);
				_pActor->Set_AnimationSpeedMulti(2.f);
			}
		}
		break;
		case PHASE::TAP:
		{
			pPlayer->Collider_SwordEnable(true);

			/* Effect */
			if (!m_bTrailSpawned)
			{
				CEffectManager::EFFECT_EVENT tEvent = {};
				tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::TRAIL;
				tEvent.EffectName = TEXT("Trail");
				tEvent.pParentMatrix =
					static_cast<CTransform*>(pPlayer->Get_Component(TEXT("Com_Transform")))->Get_WorldMatrixPtr();
				tEvent.pTipSocketMatrix = pPlayer->Get_SocketMatrixPtr("itemA_L_top");
				tEvent.pBaseSocketMatrix = pPlayer->Get_SocketMatrixPtr("itemA_L");
				tEvent.fDuration = 0.3f;
				CGameInstance::GetInstance()->Push_EffectEVENT(tEvent);

				m_bTrailSpawned = true;
			}

			/* SFX */
			if (m_tTabSFX.Tick(_fTimeDelta))
			{
				/* TAB */
				CGameInstance::GetInstance()->Play_RandomSound(L"Player_Sword_Tab", 2);

				/* TALK */
				CGameInstance::GetInstance()->Play_RandomSound(L"Player_Sword_Tab_Talk", 3);
			}
			
			if (_pActor->IsAnimFinished())
				 m_ePhase = PHASE::END;
		}
		break;
		case PHASE::CHARGE_ST:
		{
			if (m_tChargeSTSFX.Tick(_fTimeDelta))
			{
				CGameInstance::GetInstance()->Play_Sound(L"Player_Sword_Chargeshot_ST.wav", SOUND::EFFECT, 1.5f);
			}

			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::CHARGE_LP;
				// slash_hold_shield_lp
				_pActor->Set_Animation("slash_hold_shield_lp", true);
				_pActor->Set_AnimationSpeedMulti(2.f);
			}
		}
		break;
		case PHASE::CHARGE_LP:
		{
			if (!_pActor->Is_ActionPressed(IState::TRANSITION_INPUT::ATTACK_PRESSED))
			{
				/* SFX */
				CGameInstance::GetInstance()->Play_Sound(L"Player_Sword_Chargeshot_LP.wav", SOUND::EFFECT, 1.5f);

				m_ePhase = PHASE::CHARGE_ED;
				// s_slash_hold_ed_u  / s_slash_hold_ed / ev_slash_hold_ed / slash_hold_ed_u
				_pActor->Set_Animation("ev_slash_hold_ed", false);
				_pActor->Set_AnimationSpeedMulti(1.5f);
			}
		}
		break;
		case PHASE::CHARGE_ED:
		{
			if (m_tChargeEDSFX.Tick(_fTimeDelta))
			{
				CGameInstance::GetInstance()->Play_Sound(L"Player_Sword_Chargeshot_Talk.wav", SOUND::EFFECT, 1.5f);
			}

			pPlayer->Collider_SwordEnable(true);

			/* Effect */
			if (!m_bTrailSpawned_Charge)
			{
				CEffectManager::EFFECT_EVENT tEvent = {};
				tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::TRAIL;
				tEvent.EffectName = TEXT("Trail");
				tEvent.pParentMatrix =
					static_cast<CTransform*>(pPlayer->Get_Component(TEXT("Com_Transform")))->Get_WorldMatrixPtr();
				tEvent.pTipSocketMatrix = pPlayer->Get_SocketMatrixPtr("itemA_L_top");
				tEvent.pBaseSocketMatrix = pPlayer->Get_SocketMatrixPtr("itemA_L");
				tEvent.fDuration = 0.8f;
				CGameInstance::GetInstance()->Push_EffectEVENT(tEvent);

				m_bTrailSpawned_Charge = true;
			}

			if (_pActor->IsAnimFinished())
				 m_ePhase = PHASE::END;
			break;
		}
		}
	}

	virtual void OnStateExit(CActorObject* _pActor)
	{
		m_bTrailSpawned = false;
		m_bTrailSpawned_Charge = false;

		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Collider_SwordEnable(false);
	}
};
NS_END