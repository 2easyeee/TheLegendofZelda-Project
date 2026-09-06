#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Jump_SideView : public IState
{
private:
	enum class PHASE { AIR, LAND, END };
	PHASE m_ePhase;

	/* Jump */
	_float m_fJumpPower = { 8.f };
	_float m_fGravity = { -22.f };
	_float m_fVelocityY = { 0.f };

	_float m_fFixedZ = { 0.f };
	_float m_fGroundY = { 0.f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::JUMP_SIDEVIEW);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_ePhase = PHASE::AIR;
		_pActor->Set_Animation("jump", false);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(2.5f);
		_pActor->Set_SpeedMulti(1.f);

		/* Jump */
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Start_Jump();

		/* SFX */
		CGameInstance::GetInstance()->Play_RandomSound(L"Player_Jump_Talk", 3);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Apply_Gravity(_fTimeDelta);

		if (!pPlayer->Is_Air() && m_ePhase == PHASE::AIR)
		{
			_pActor->Set_Animation("land", false);
			m_ePhase = PHASE::LAND;
		}

		if (m_ePhase == PHASE::LAND)
		{
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::END;
			}
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		/* SFX */
		CGameInstance::GetInstance()->Play_RandomSound(L"Player_Land_Talk", 1, 0.5f);

		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END