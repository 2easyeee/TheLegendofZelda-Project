#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Shield : public IState
{
private:
	enum class PHASE { LP, ED, END };
	PHASE m_ePhase = { PHASE::LP };
	_string m_strCurrentAnim;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::SHIELD);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_ePhase = PHASE::LP;
		m_strCurrentAnim = "shield_lp";
		_pActor->Set_Animation(m_strCurrentAnim, true);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(1.f);
		
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Collider_ShiledEnable(true);
		pPlayer->Collider_BodyEnable(false);

		/* SFX */
		CGameInstance::GetInstance()->Play_Sound(L"Player_Shield_Up_ST.wav", SOUND::EFFECT);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Apply_Gravity(_fTimeDelta);

		if (m_ePhase == PHASE::ED)
		{
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::END;
			}
			return;
		}

		if (!_pActor->Is_ActionPressed(IState::TRANSITION_INPUT::SHIELD_PRESSED))
		{
			m_ePhase = PHASE::ED;
			m_strCurrentAnim = "shield_ed";
			_pActor->Set_Animation(m_strCurrentAnim, false);
			_pActor->Reset_Animation_TrackPosition();
			_pActor->Set_AnimationSpeedMulti(3.5f);
			return;
		}

		_string strTargetAnim 
				= _pActor->Is_MoveInputEnable() ? "shield_hold_f" : "shield_lp";
		
		if (m_strCurrentAnim != strTargetAnim)
		{
			m_strCurrentAnim = strTargetAnim;
			_pActor->Set_Animation(m_strCurrentAnim, true, true);
			_pActor->Reset_Animation_TrackPosition();
			_pActor->Set_AnimationSpeedMulti(1.f);
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Collider_ShiledEnable(false);
		pPlayer->Collider_BodyEnable(true);
	}
};
NS_END