#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Albatoss.h"
#include "GameInstance.h"

NS_BEGIN(Client)
class CAlbatoss_Pop : public IPawnState
{
private:
	STATE_TIME m_tPopSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::POP);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("demo_pop", false);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(4.2f);

		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
		pMonster->Collider_BodyEnable(false);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		_float fRatio = _pActor->Get_PlayRatio();
		if (fRatio < 0.55f && fRatio >= 0.3f)
		{
			_pActor->Set_AnimationSpeedMulti(2.5f);
			CEventManager::GAME_EVENT tEvent = {};
			tEvent.eType = CEventManager::GAME_EVENT_TYPE::CUTSCENE_START;
			CGameInstance::GetInstance()->Push_GameEVENT(tEvent);
		}
		else if (fRatio < 0.9f && fRatio >= 0.55f)
		{
			_pActor->Set_AnimationSpeedMulti(2.5f);

			CEventManager::GAME_EVENT tEvent = {};
			tEvent.eType = CEventManager::GAME_EVENT_TYPE::SPAWN_BOSS;
			CGameInstance::GetInstance()->Push_GameEVENT(tEvent);
		}
		else if (fRatio >= 0.95f)
		{
			_pActor->Set_BoneSnap("PiccoloMaster_root", "waist");
		}

		if (fRatio >= 0.7f)
		{
			/* SFX */
			if (!m_tPopSFX.bActive)
			{
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_Albatoss_Pop.wav", SOUND::EFFECT);
				m_tPopSFX.bActive = true;
			}
		}

		if (_pActor->IsAnimFinished())
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::IDLE));
			CEventManager::GAME_EVENT tEvent = {};
			tEvent.eType = CEventManager::GAME_EVENT_TYPE::CUTSCENE_END;
			CGameInstance::GetInstance()->Push_GameEVENT(tEvent);
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
		pMonster->Collider_BodyEnable(true);

		/* SFX */
		CGameInstance::GetInstance()->Set_ChannelVolume(SOUND::BGM, 0.7f);
	}
};
NS_END