#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_DoorOpen : public IState
{
private:
	_bool m_bFinished = { false };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::DOOR_OPEN);
	}
	virtual _bool Is_StateFinished() const override { return m_bFinished; }
	virtual _bool Block_Locomotion() const override
	{
		return true;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_bFinished = false;
		_pActor->Set_Animation("key_open", false);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		if (_pActor->IsAnimFinished())
		{
			m_bFinished = true;
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END