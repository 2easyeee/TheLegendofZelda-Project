#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_StalKnightSword.h"

NS_BEGIN(Client)
class CStalKnightSword_Damage : public IPawnState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::DAMAGE);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("damage_f", false);
		_pActor->Set_AnimationSpeedMulti(1.f);

		/* SFX */
		CGameInstance::GetInstance()->Play_Sound(L"ENEMY_StalKnightSword_Damage.wav", SOUND::EFFECT, 1.5f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		if (_pActor->IsAnimFinished())
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::IDLE));
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
	}
};
NS_END