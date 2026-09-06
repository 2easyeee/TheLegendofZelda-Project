#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Jump : public IState
{
private:
	enum class PHASE { AIR, LAND, END };
	PHASE m_ePhase;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::JUMP);
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
		_pActor->Set_AnimationSpeedMulti(2.5f);
		_pActor->Set_SpeedMulti(1.f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		if (m_ePhase == PHASE::AIR)
		{
			if (_pActor->Is_OnGround())
			{
				m_ePhase = PHASE::LAND;
				_pActor->Set_Animation("land", false);
			}
		}
		else if (m_ePhase == PHASE::LAND)
		{
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::END;
			}
			return;
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END