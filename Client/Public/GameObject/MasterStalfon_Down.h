#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MasterStalfon.h"

NS_BEGIN(Client)
class CMasterStalfon_Down : public IPawnState
{
private:
	enum class PHASE { ST, LP, ED, END };
	CMasterStalfon_Down::PHASE m_Phase;
	STATE_TIME m_tLoopTime = { false, 0.f, 3.f };

	/* SFX */
	STATE_TIME m_tBoneOffSFX;
	STATE_TIME m_tSwrodOffSFX;
	STATE_TIME m_tBoneOnSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::DOWN);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		if (m_Phase == PHASE::LP)
		{
			m_Phase = CMasterStalfon_Down::PHASE::ED;
			_pActor->Set_Animation("down_ed", false);			
			m_tLoopTime.fAccTime = 0.f;
			return;
		}

		m_Phase = CMasterStalfon_Down::PHASE::ST;
		_pActor->Set_Animation("down_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		m_tLoopTime.fAccTime = 0.f;
		m_tLoopTime.fAccDurationTime = 3.f;

		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		pMonster->Collider_SensorEnable(false);
		pMonster->Collider_WeaponEnable(false);
		pMonster->Collider_ShieldEnable(false);

		/* SFX */
		m_tBoneOffSFX.bActive = false;
		m_tBoneOnSFX.bActive = false;
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		switch (m_Phase)
		{
		case CMasterStalfon_Down::PHASE::ST:
		{
			_float fRaito = _pActor->Get_PlayRatio();
			/* SFX */
			if (fRaito < 0.1f)
			{
				if (!m_tBoneOffSFX.bActive)
				{
					CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_BoneOff.wav", SOUND::EFFECT, 2.f);
					m_tBoneOffSFX.bActive = true;
				}
			}
			else if (fRaito >= 0.3f)
			{
				if (!m_tSwrodOffSFX.bActive)
				{
					CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_SwordOff.wav", SOUND::EFFECT, 2.f);
					m_tSwrodOffSFX.bActive = true;
				}
			}

			if (_pActor->IsAnimFinished())
			{
				m_Phase = CMasterStalfon_Down::PHASE::LP;
				_pActor->Set_Animation("down_lp", true);
			}
		}
			break;
		case CMasterStalfon_Down::PHASE::LP:
		{
			auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
			pMonster->Collider_BodyEnable(true);

			m_tLoopTime.fAccTime += _fTimeDelta;
			if (m_tLoopTime.fAccTime >= m_tLoopTime.fAccDurationTime)
			{
				m_Phase = CMasterStalfon_Down::PHASE::ED;
				_pActor->Set_Animation("down_ed", false);
			}
		}
			break;
		case CMasterStalfon_Down::PHASE::ED:
		{
			if (!m_tBoneOnSFX.bActive)
			{
				/* SFX */
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_BoneOn.wav", SOUND::EFFECT);
				m_tBoneOnSFX.bActive = true;
			}

			if (_pActor->IsAnimFinished())
			{
				auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
				pMonster->Collider_BodyEnable(true);
				pMonster->Collider_SensorEnable(true);
				pMonster->Collider_WeaponEnable(true);
				pMonster->Collider_ShieldEnable(true);

				_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::IDLE));
			}
		}
			break;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		m_Phase = PHASE::ST;
		m_tLoopTime.fAccTime = 0.f;
	}
};
NS_END