#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Warp_Down : public IState
{
private:
	enum class PHASE { ST, LP, ED, END };
	PHASE m_ePhase;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::WARP_DOWN);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_ePhase = PHASE::ST;
		_pActor->Set_Animation("warp_d_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		if (m_ePhase == PHASE::ST)
		{
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::LP;
				_pActor->Set_Animation("warp_d_lp", false);
			}
		}
		else if (m_ePhase == PHASE::LP)
		{
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::ED;
				_pActor->Set_Animation("warp_d_ed", false);
			}
		}
		else if (m_ePhase == PHASE::ED)
		{
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::END;
			}
		}
		return;
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END