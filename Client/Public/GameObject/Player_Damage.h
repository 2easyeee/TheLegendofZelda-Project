#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Damage : public IState
{
private:
	_bool m_bFinished = { false };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::DAMAGE);
	}
	virtual _bool Is_StateFinished() const override { return m_bFinished; }
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_bFinished = false;
		_pActor->Set_Animation("dmg_wt_f", false, true);
		_pActor->Set_AnimationSpeedMulti(2.f);

		/* SFX */
		CGameInstance::GetInstance()->Play_RandomSound(L"Player_Damage_Talk", 5, 3.f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		if (_pActor->IsAnimFinished())
		{
			m_bFinished = true;
		}
		return;
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END