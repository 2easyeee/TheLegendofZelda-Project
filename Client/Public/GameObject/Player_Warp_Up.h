#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Warp_Up : public IState
{
private:
	enum class PHASE { ST, LP, ED };
	PHASE m_ePhase;

	/* Rot */
	_float m_fRotateSpeed = { 720.f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::WARP_UP);
	}
	virtual _bool Block_Locomotion() const override
	{
		return true;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		_pActor->Set_Animation("warp_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		m_ePhase = PHASE::ST;

		/* Rot */
		CTransform* pTransform = static_cast<CTransform*>(_pActor->Get_Component(TEXT("Com_Transform")));
		pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 0.f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		CTransform* pTransform = static_cast<CTransform*>(_pActor->Get_Component(TEXT("Com_Transform")));

		_float fRatio = _pActor->Get_PlayRatio();
		if (fRatio >= 0.9f)
		{
			// nothing
			_pActor->Set_SpeedMulti(1.f);
		}
		else if (fRatio >= 0.3f)
		{
			_float t = (fRatio - 0.3f) / (0.9f - 0.3f);
			t = max(0.f, min(t, 1.f));
			_float eased = EaseInCubic(t);

			_float fBase = 1000.f;
			_float fMax = 2000.f;
			_float fSpeed = fBase + (fMax - fBase) * eased;

			pTransform->Set_SpeedMulti(fSpeed);
			pTransform->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), _fTimeDelta);
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);
	}
};
NS_END