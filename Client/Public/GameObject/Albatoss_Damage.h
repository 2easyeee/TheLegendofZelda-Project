#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Albatoss.h"

NS_BEGIN(Client)
class CAlbatoss_Damage : public IPawnState
{
private:
	STATE_TIME tInvincibleTime = { false, 0.f, 0.5f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::DAMAGE);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("damage_hovering", false, true);
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
		pMonster->Collider_BodyEnable(false);
		pMonster->Collider_ClawEnable(false);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		_float fRatio = _pActor->Get_PlayRatio();
		if (fRatio >= 0.5f)
		{
			auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
			pMonster->Collider_BodyEnable(true);
		}

		if (_pActor->IsAnimFinished())
		{
			auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
			_pActor->RequestToChangeState(pMonster->Get_PreState());
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
		pMonster->Collider_BodyEnable(true);
	}
};
NS_END