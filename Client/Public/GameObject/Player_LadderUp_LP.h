#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_LadderUp_LP : public IState
{
private:
	enum class PHASE { WAIT, LP, ED, END };
	PHASE m_ePhase;
	PHASE m_ePrevPhase;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::LADDER_UP_LP);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_ePhase = PHASE::WAIT;
		m_ePrevPhase = PHASE::END;
		_pActor->Set_Animation("ladder_wait", true);
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		/* PlayerY > LadderY */
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		CTransform* pTransform = static_cast<CTransform*>(pPlayer->Get_Component(TEXT("Com_Transform")));
		_float fY = XMVectorGetY(pTransform->Get_State(STATE::POSITION));

		_float fLadderYTop, fLadderYBottom;
		pPlayer->Return_Ladder_Y_Value(fLadderYTop, fLadderYBottom);

		_vector vMove = pPlayer->Get_AimDirection();
		_float fInputY = XMVectorGetY(vMove);

		/* ÆÇ´Ü */
		if (m_ePhase != PHASE::ED && fY > fLadderYTop)
		{
			m_ePrevPhase = m_ePhase;
			m_ePhase = PHASE::ED;
			_pActor->Set_Animation("ladder_up_ed", false);
		}

		if (m_ePhase != PHASE::ED && fY < fLadderYBottom)
		{
			m_ePhase = PHASE::END;
			return;
		}

		if (m_ePhase != PHASE::ED)
		{
			PHASE eNewPhase = fabs(fInputY) > 0.1f ? PHASE::LP : PHASE::WAIT;

			if (eNewPhase != m_ePhase)
			{
				m_ePrevPhase = m_ePhase;
				m_ePhase = eNewPhase;

				switch (m_ePhase)
				{
				case PHASE::WAIT:
					_pActor->Set_Animation("ladder_wait", true);
					break;
				case PHASE::LP:
				{
					if (fInputY > 0.f)
					{
						_pActor->Set_Animation("ladder_up", true);
					}
					else if (fInputY < 0.f)
					{
						_pActor->Set_Animation("ladder_down", true);
					}
					_pActor->Set_SpeedMulti(1.5f);
				}
					break;
				}
			}
		}

		if (m_ePhase == PHASE::ED)
		{
			_pActor->Set_AnimationSpeedMulti(0.5f);
			_pActor->Set_SpeedMulti(1.f);
			pTransform->MoveTo(XMVectorSet(0.9f, 0.f, -0.333f, 1.f), _fTimeDelta, 0.1f);

			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::END;
			}
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_SpeedMulti(1.f);
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Set_InputMode(CPlayerInputController::INPUT_MODE::SIDE);
	}
};
NS_END