#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MasterStalfon.h"

NS_BEGIN(Client)
class CMasterStalfon_Pop : public IPawnState
{
private:
	enum class PHASE { WAIT, POP, END };
	CMasterStalfon_Pop::PHASE m_tPhase;
	STATE_TIME m_tPopTime = { false, 0.f, 3.f };

	/* SFX */
	STATE_TIME m_tPopSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::POP);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("pop_wait", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		m_tPhase = CMasterStalfon_Pop::PHASE::WAIT;
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		m_tPopTime.fAccTime += _fTimeDelta;
		switch (m_tPhase)
		{
		case CMasterStalfon_Pop::PHASE::WAIT:
		{
			if (m_tPopTime.fAccTime >= m_tPopTime.fAccDurationTime)
			{
				m_tPhase = CMasterStalfon_Pop::PHASE::POP;
				_pActor->Set_Animation("pop", false);
				_pActor->Reset_Animation_TrackPosition();

				/* SFX */
				m_tPopSFX.bActive = true;
			}
		}
			break;
		case CMasterStalfon_Pop::PHASE::POP:
		{
			/* 0.8f 이후에 Collider 생성 */
			auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
			_float fRatio = _pActor->Get_PlayRatio();

			if (m_tPopSFX.bActive && fRatio >= 0.6f)
			{
				m_tPopSFX.bActive = false;
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_Pop.wav", SOUND::EFFECT);
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_Walk_1.wav", SOUND::EFFECT);
			}

			if (fRatio > 0.8f)
			{
				pMonster->Collider_BodyEnable(true);
				pMonster->Collider_SensorEnable(true);
			}

			if (_pActor->IsAnimFinished())
			{
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::IDLE));
			}
		}
			break;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		CGameInstance::GetInstance()->Set_ChannelVolume(SOUND::BGM, 0.7f);

		{
			/* Light */
			CGameInstance::GetInstance()->Clear_Light();
			LIGHT_DESC m_tBossSceneLight = {};
			m_tBossSceneLight.eType = LIGHT::DIRECTIONAL;
			m_tBossSceneLight.vDiffuse = { 0.779f, 0.867f, 0.5f, 1.f };
			m_tBossSceneLight.vAmbient = { 0.6f, 0.6f, 0.6f, 1.f };
			m_tBossSceneLight.vSpecular = { 1.f, 1.f, 1.f, 1.f };
			m_tBossSceneLight.vDirection = { 0.f, -1.f, 0.f, 1.f};
			m_tBossSceneLight.eUsage = LIGHT_USAGE::STATIC;
			CGameInstance::GetInstance()->Add_Light(m_tBossSceneLight);
		}
	}
};
NS_END