#pragma once
#include "IActorState.h"
#include "ActorObject.h"

NS_BEGIN(Client)
class CMoriblinSpear;
NS_END

NS_BEGIN(Client)
class CMoriblinSpear_Attack : public IActorState
{
private:
	CMoriblinSpear* pM = { nullptr };
	_float m_fPrevRatio = 0.f; /// Attack Anim 

public: 
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMoriblinSpear::STATE_ID::ATTACK);
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		pM = static_cast<CMoriblinSpear*>(_pActor);
		_pActor->Set_Animation("spear_throw", false);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(1.f);

		m_fPrevRatio = 0.f;
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		/* Collider */
		auto pMonster = static_cast<CMoriblinSpear*>(_pActor);

		_float fRatio = _pActor->Get_PlayRatio();
		if (fRatio >= 0.3f && m_fPrevRatio < 0.3f)
		{
			pM->Notify_WeaponThrow();

			/* SFX */
			CGameInstance::GetInstance()->Play_RandomSound(L"ENEMY_MoriblinSpear_Throw", 4, 1.8f);
		}

		m_fPrevRatio = fRatio;

		if (pM->IsAnimFinished())
		{
			pM->Notify_WeaponRespawn();
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::IDLE));
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Reset_Animation_TrackPosition();
	}
};
NS_END