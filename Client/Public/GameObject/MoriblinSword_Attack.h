#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MoriblinSword.h"

NS_BEGIN(Client)
class CMoriblinSword_Attack : public IPawnState
{
private:
	_int m_iAttackCount = { 0 };
	_int m_iAttackMAXCount = { 1 }; // 3

	STATE_TIME m_tSwordSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MoriblinSword::STATE_ID::ATTACK);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		auto pMonster = static_cast<CMonster_MoriblinSword*>(_pActor);

		_pActor->Set_Animation("stance_walk", true);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(1.5f);
		m_iAttackCount = 0;

		/* Collider */
		pMonster->Collider_WeaponEnable(true);

		/* SFX */
		m_tSwordSFX.Start(0.25f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		/* Collider */
		auto pMonster = static_cast<CMonster_MoriblinSword*>(_pActor);

		_float fRatio = _pActor->Get_PlayRatio();
		if (fRatio > 0.3f && fRatio < 0.8f)
		{
			pMonster->Collider_WeaponEnable(true);

			/* SFX */
			if (m_tSwordSFX.Tick_Loop(_fTimeDelta))
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Common_Sword_Hit_ToPlayer.wav", SOUND::EFFECT);
		}
		else
		{
			pMonster->Collider_WeaponEnable(false);
		}

		if (fRatio < 0.9f)
			return;

		if (m_iAttackCount < m_iAttackMAXCount)
		{
			// 애니메이션 다시 재생
			m_iAttackCount++;
			_pActor->Reset_Animation_TrackPosition();
		}
		else
		{
			_pActor->RequestToChangeState(
				ENUM_TO_UINT(CMonster_MoriblinSword::STATE_ID::IDLE));
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto pMonster = static_cast<CMonster_MoriblinSword*>(_pActor);
		pMonster->Collider_WeaponEnable(false);
	}
};
NS_END