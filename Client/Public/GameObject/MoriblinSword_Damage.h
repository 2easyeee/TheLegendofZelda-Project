#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MoriblinSword.h"

NS_BEGIN(Client)
class CMoriblinSword_Damage : public IPawnState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MoriblinSword::STATE_ID::DAMAGE);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("damage_f", false);
		_pActor->Set_AnimationSpeedMulti(1.f);

		/* SFX */
		CGameInstance::GetInstance()->Play_RandomSound(L"ENEMY_Moriblin_Damage", 3, 2.f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		if (_pActor->IsAnimFinished())
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_MoriblinSword::STATE_ID::IDLE));
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{

	}
};
NS_END