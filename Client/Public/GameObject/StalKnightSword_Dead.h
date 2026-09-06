#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_StalKnightSword.h"

NS_BEGIN(Client)
class CStalKnightSword_Dead : public IPawnState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_StalKnightSword::STATE_ID::DEAD);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("dead_f", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		
		auto pMonster = static_cast<CMonster_StalKnightSword*>(_pActor);
		pMonster->Collider_BodyEnable(false);

		/* SFX */
		CGameInstance::GetInstance()->Play_Sound(L"ENEMY_StalKnightSword_Damage.wav", SOUND::EFFECT, 1.5f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
	}
};
NS_END