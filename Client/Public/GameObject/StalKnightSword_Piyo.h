#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_StalKnightSword.h"

NS_BEGIN(Client)
class CStalKnightSword_Piyo : public IPawnState
{
private:
	STATE_TIME tStunTime = { false, 0.f, 1.5f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::PIYO);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("stagger", false);
		_pActor->Set_AnimationSpeedMulti(1.f);

		tStunTime.fAccTime = 0.f;

		auto pMonster = static_cast<CMonster_StalKnightSword*>(_pActor);
		pMonster->Collider_ShieldEnable(false);
		pMonster->Collider_BodyEnable(true);

		/* SFX */
		CGameInstance::GetInstance()->Play_Sound(L"ENEMY_StalKnightSword_Search.wav", SOUND::EFFECT, 2.f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		tStunTime.fAccTime += _fTimeDelta;
		if (tStunTime.fAccTime >= tStunTime.fAccDurationTime)
		{
			_pActor->RequestToChangeState(
				ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::GUARD));
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
	}
};
NS_END