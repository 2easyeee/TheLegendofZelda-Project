#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Albatoss.h"

NS_BEGIN(Client)
class CAlbatoss_Dead : public IPawnState
{
public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::DEAD);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("dead", false);
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
		pMonster->Collider_BodyEnable(false);
		pMonster->Collider_ClawEnable(false);

		/* Notify */
		pMonster->Notify_Delete();
	
		/* SFX */
		CGameInstance::GetInstance()->Stop_Sound(SOUND::BGM);
		CGameInstance::GetInstance()->Play_Sound(L"Ambience_Tower_Wind_Depre.wav", SOUND::EFFECT, 0.8f);
		CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_Albatoss_Pop.wav", SOUND::EFFECT);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{

	}
};
NS_END