#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Albatoss.h"

NS_BEGIN(Client)
class CAlbatoss_ATK_Flapping : public IPawnState
{
private:
	enum class PHASE { ST, LP, ED, NONE, END };
	PHASE m_ePhase;

	/* Flapping */
	STATE_TIME m_tFlappingTime = { false, 0.f, 3.f };
	_float m_fDir = 1.f;

	/* Shoot */
	_int m_iShootCount = { 0 };
	_int m_iShootMaxCount = { 10 };
	STATE_TIME m_tShootIntervalTime = { false, 0.f, 0.2f };

	/* SFX */
	STATE_TIME m_tFlappingSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::ATK_FLAPPING);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("attack_flapping_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		m_ePhase = PHASE::ST;
		m_tFlappingTime.fAccTime = 0.f;

		/* Rotation */
		auto pTransform = _pActor->Get_TrasformForMove();
		_vector vPos = pTransform->Get_State(STATE::POSITION);
		_float fX = XMVectorGetX(vPos);

		m_fDir = (fX < 0.f) ? 1.f : -1.f;

		if (m_fDir > 0.f)
			pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), -90.f);
		else
			pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 90.f);

		/* Shoot */
		m_iShootCount = 0;
		m_tShootIntervalTime.fAccTime = 0.f;

		/* SFX */
		m_tFlappingSFX.Start(0.25f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		if (m_ePhase == PHASE::ST)
		{
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::LP;
				_pActor->Set_Animation("attack_flapping", true);
			}
		}
		else if (m_ePhase == PHASE::LP)
		{
			if (m_tFlappingSFX.Tick_Loop(_fTimeDelta))
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_Albatoss_Flapping.wav", SOUND::EFFECT);

			m_tShootIntervalTime.fAccTime += _fTimeDelta;
			if (m_iShootCount < m_iShootMaxCount && m_tShootIntervalTime.fAccTime >= m_tShootIntervalTime.fAccDurationTime)
			{
				m_tShootIntervalTime.fAccTime = 0.f;
				Shoot_Feather(_pActor);
				m_iShootCount++;
			}
			m_tFlappingTime.fAccTime += _fTimeDelta;
			if (m_tFlappingTime.fAccTime >= m_tFlappingTime.fAccDurationTime)
			{
				m_ePhase = PHASE::ED;
				_pActor->Set_Animation("attack_flapping_ed", false);
			}
		}
		else if (m_ePhase == PHASE::ED)
		{
			auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
			pMonster->NextPattern();
			_pActor->RequestToChangeState(
				ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::HOVERING));
			return;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{

	}
	void Shoot_Feather(class CPawnObject* _pActor)
	{
		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
		auto pTransform = _pActor->Get_TrasformForMove();

		_vector vMonsterPos = pTransform->Get_State(STATE::POSITION);
		_float fMonsterX = XMVectorGetX(vMonsterPos);

		_float fDir = (fMonsterX < 0.f) ? 1.f : -1.f; 

		_float fRandX = fDir * (3.f + static_cast<float>(rand()) / RAND_MAX * 5.f); 
		_float fRandY = -2.f - static_cast<float>(rand()) / RAND_MAX * 3.f; 
		
		_vector vSpawnPos = XMVectorSet(
			XMVectorGetX(vMonsterPos) + fRandX, 
			XMVectorGetY(vMonsterPos) + fRandY, 
			0.f, 
			1.f);

		pMonster->Respawn();
		pMonster->Shoot_Feather(vSpawnPos);
	}
};
NS_END