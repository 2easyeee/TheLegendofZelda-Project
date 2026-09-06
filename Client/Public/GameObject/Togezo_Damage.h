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
		_pActor->Set_Animation(0, false); // 0 : damage.smd
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		if (_pActor->IsAnimFinished())
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::IDLE));
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{

	}
};
NS_END