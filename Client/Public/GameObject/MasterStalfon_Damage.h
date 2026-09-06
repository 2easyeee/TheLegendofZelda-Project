#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MasterStalfon.h"

NS_BEGIN(Client)
class CMasterStalfon_Damage : public IPawnState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::DAMAGE);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("damage", false);
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		/* Collider */
		pMonster->Collider_WeaponEnable(false);
		pMonster->Collider_ShieldEnable(false);
		pMonster->Collider_BodyEnable(true);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		if (_pActor->IsAnimFinished())
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::DOWN));
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		/* Collider */
		pMonster->Collider_BodyEnable(true);
	}
};
NS_END