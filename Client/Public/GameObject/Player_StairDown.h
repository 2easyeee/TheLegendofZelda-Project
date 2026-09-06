#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_StairDown : public IState
{
private:
	enum class PHASE { LP, ED, END };
	PHASE m_ePhase = { PHASE::LP };
	STATE_TIME m_tFinishedTime = { false, 0.f, 5.f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::STAIR_DOWN);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }
	virtual _bool Block_Locomotion() const override
	{
		return true;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_ePhase = PHASE::LP;
		m_tFinishedTime.fAccTime = 0.f;
		_pActor->Set_Animation("ev_stairs_down", false);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		auto pPlayer = static_cast<CPlayer*>(_pActor);

		_vector vTarget = pPlayer->Get_StairTargetPos();

		/* 컷신 위치까지 MoveTo */
		auto pTransform = static_cast<CTransform*>(_pActor->Get_Component(TEXT("Com_Transform")));
		pTransform->MoveTo(vTarget, _fTimeDelta, 0.1f);

		// refactor
		m_tFinishedTime.fAccTime += _fTimeDelta;
		if (m_tFinishedTime.fAccTime >= m_tFinishedTime.fAccDurationTime)
		{
			m_ePhase = PHASE::END;
		}
		return;
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END