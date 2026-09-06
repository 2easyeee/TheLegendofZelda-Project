#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_LadderUp_ST : public IState
{
private:
	_bool m_bFinished = { false };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::LADDER_UP_ST);
	}
	virtual _bool Is_StateFinished() const override { return m_bFinished; }
	virtual _bool Block_Locomotion() const override
	{
		return true;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_bFinished = false;

		_pActor->Set_Animation("ladder_up_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);

		auto pPlayer = static_cast<CPlayer*>(_pActor);
		auto pTransform = static_cast<CTransform*>(pPlayer->Get_Component(TEXT("Com_Transform")));
		if (!pTransform)
			return;
		pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), -180.f);
		
		/* Snap */
		_vector vPos = pTransform->Get_State(STATE::POSITION);
		vPos = XMVectorSetX(vPos, 0.9f);
		vPos = XMVectorSetZ(vPos, -2.65f);
		pTransform->Set_State(STATE::POSITION, vPos);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		if (!m_bFinished && _pActor->IsAnimFinished())
		{
			m_bFinished = true;
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_SpeedMulti(1.f);
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END