#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_StalKnightSword.h"

NS_BEGIN(Client)
class CStalKnightSword_Guard : public IPawnState
{
private:
	STATE_TIME tGuardTime = { false, 0.f, 2.5f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::GUARD);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("guard", false);
		_pActor->Set_AnimationSpeedMulti(0.2f);

		tGuardTime.fAccTime = 0.f;

		// 방패 활성화
		auto pMonster = static_cast<CMonster_StalKnightSword*>(_pActor);
		pMonster->Collider_ShieldEnable(true);
		pMonster->Collider_BodyEnable(false);

		/* SFX */
		CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Common_Sword_Hit_ToShield.wav", SOUND::EFFECT);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		tGuardTime.fAccTime += _fTimeDelta;
		if (tGuardTime.fAccTime >= tGuardTime.fAccDurationTime)
		{
			_pActor->RequestToChangeState(
				ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::IDLE));
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		auto pMonster = static_cast<CMonster_StalKnightSword*>(_pActor);
		pMonster->Collider_ShieldEnable(false);
		pMonster->Collider_BodyEnable(true);
	}
};
NS_END