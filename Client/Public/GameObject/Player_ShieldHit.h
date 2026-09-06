#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_ShieldHit : public IState
{
private:
	_bool m_bFinished = { false };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::SHIELD_HIT);
	}
	virtual _bool Is_StateFinished() const override { return m_bFinished; }
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_bFinished = false;
		_pActor->Set_Animation("shieldhit", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Collider_BodyEnable(false);
		pPlayer->Collider_ShiledEnable(true);

		/* SFX */
		CGameInstance::GetInstance()->Play_RandomSound(L"Player_Shield_Guard", 2);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		if (_pActor->IsAnimFinished())
			m_bFinished = true;
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Collider_BodyEnable(true);
		pPlayer->Collider_ShiledEnable(false);
	}
};
NS_END