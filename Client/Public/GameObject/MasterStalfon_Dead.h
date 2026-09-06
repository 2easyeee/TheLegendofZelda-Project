#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MasterStalfon.h"

NS_BEGIN(Client)
class CMasterStalfon_Dead : public IPawnState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::DEAD);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("dead", true);
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		/* Collider */
		pMonster->Collider_WeaponEnable(false);
		pMonster->Collider_ShieldEnable(false);
		pMonster->Collider_BodyEnable(false);

		/* SFX */
		CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_Pop.wav", SOUND::EFFECT, 2.f);
		CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_Delete.wav", SOUND::EFFECT, 2.f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{

	}
};
NS_END