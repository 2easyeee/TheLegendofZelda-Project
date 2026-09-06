#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MasterStalfon.h"

NS_BEGIN(Client)
class CMasterStalfon_Attack : public IPawnState
{
private:
	_int m_iAttackCount = { 0 };
	_int m_iAttackMAXCount = { 0 };
	_bool m_bWaitingNextAttack = { false };

	/* SFX */
	STATE_TIME m_tSwordSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::ATTACK);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);

		_pActor->Set_Animation("sword_attack_l", false);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(2.5f);
		m_iAttackCount = 1;
		m_bWaitingNextAttack = false;

		/* Max Count (Attack) */
		m_iAttackMAXCount = 1 + rand() % 3;

		/* Collider */
		pMonster->Collider_WeaponEnable(true);
		pMonster->Collider_BodyEnable(true);

		/* SFX */
		m_tSwordSFX.bActive = false;
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		/* Collider */
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);

		_float fRatio = _pActor->Get_PlayRatio();
		if (!m_tSwordSFX.bActive && fRatio >= 0.3f)
		{
			/* SFX */
			CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_Sword_1.wav", SOUND::EFFECT, 2.f);

			m_tSwordSFX.bActive = true;
		}

		if (fRatio > 0.3f && fRatio < 0.8f)
			pMonster->Collider_WeaponEnable(true);
		else
			pMonster->Collider_WeaponEnable(false);

		/* Perfect Dodge Window */
		if (fRatio > 0.f && fRatio < 0.65f)
			pMonster->Collider_AttackWindowEnable(true);
		else
			pMonster->Collider_AttackWindowEnable(false);

		if (fRatio < 0.9f)
			return;

		if (m_bWaitingNextAttack)
			return;

		m_bWaitingNextAttack = true;

		if (m_iAttackCount < m_iAttackMAXCount)
		{
			Play_AttackLR(_pActor);
			m_iAttackCount++;
			m_bWaitingNextAttack = false;
		}
		else
		{
			pMonster->NextPattern();
			pMonster->Run_CurrentPattern();
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);

		_pActor->Set_AnimationSpeedMulti(1.f);
		m_bWaitingNextAttack = false;

		/* Collider */
		pMonster->Collider_WeaponEnable(false);
		pMonster->Collider_BodyEnable(false);
	}
	void Play_AttackLR(class CPawnObject* _pActor)
	{
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		if (m_iAttackCount % 2 == 0)
			_pActor->Set_Animation("sword_attack_l", false);
		else
			_pActor->Set_Animation("sword_attack_r", false);

		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(2.5f);

		m_tSwordSFX.bActive = false;

		pMonster->Collider_WeaponEnable(false);
	}
};
NS_END