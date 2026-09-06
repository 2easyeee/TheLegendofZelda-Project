#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Albatoss.h"

NS_BEGIN(Client)
class CAlbatoss_Rush : public IPawnState
{
private:
	enum class PHASE { ST, LP, RETURN, END };
	PHASE m_ePhase;
	
	/* Rush */
	_int m_iRushCount = 0;
	_int m_iMaxRush = 3;

	_float m_fDir = 1.f;
	_float m_fHeight = 0.f;
	_float m_fLimitX = 17.f; // 15.f

	_float height[4] = { 0.f, 0.f, 3.f, 5.f};

	/* SFX */
	STATE_TIME m_tRushSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::RUSH);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		/* State */
		_pActor->Set_Animation("rush_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		m_ePhase = PHASE::ST;
		m_iRushCount = 0;

		/* Dir (이전 Rush 방향 저장 기반) */
		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
		m_fDir = -pMonster->Get_LastRushDir();
		pMonster->Set_LastRushDir(m_fDir);

		auto pTransform = _pActor->Get_TrasformForMove();
		if (m_fDir > 0.f)
			pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), -90.f);
		else
			pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 90.f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		CTransform* pTransform = _pActor->Get_TrasformForMove();
		
		switch (m_ePhase)
		{
		case Client::CAlbatoss_Rush::PHASE::ST:
		{
			if (_pActor->IsAnimFinished())
			{
				/* Block Dup Y */
				_float newHeight;
				do
				{
					newHeight = height[rand() % 4];
				} while (newHeight == m_fHeight);

				m_fHeight = newHeight;

				m_ePhase = PHASE::LP;
				_pActor->Set_Animation("rush", true);
			}

			/* SFX */
			if (!m_tRushSFX.bActive)
			{
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_Albatoss_Rush.wav", SOUND::EFFECT, 2.f);
				m_tRushSFX.bActive = true;
			}
		}
			break;
		case Client::CAlbatoss_Rush::PHASE::LP:
		{
			_vector vPos = pTransform->Get_State(STATE::POSITION);
			vPos += XMVectorSet(m_fDir * 20.f * _fTimeDelta, 0.f, 0.f, 0.f);
			
			_float fX = XMVectorGetX(vPos);
			if (fX >= m_fLimitX)
			{
				fX = m_fLimitX;
				m_ePhase = PHASE::RETURN;
				_pActor->Set_Animation("rush_return", false);
			}
			else if (fX <= -m_fLimitX)
			{
				fX = -m_fLimitX;
				m_ePhase = PHASE::RETURN;
				_pActor->Set_Animation("rush_return", false);
			}

			vPos = XMVectorSetX(vPos, fX);
			vPos = XMVectorSetY(vPos, m_fHeight);
			pTransform->Set_State(STATE::POSITION, vPos);
		}
			break;
		case Client::CAlbatoss_Rush::PHASE::RETURN:
		{
			if (_pActor->IsAnimFinished())
			{
				m_iRushCount++;

				if (m_iRushCount >= m_iMaxRush)
				{
					auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
					pMonster->NextPattern();
					_pActor->RequestToChangeState(
						ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::HOVERING));
					return;
				}

				m_fDir *= -1.f;
				if (m_fDir > 0.f)
					pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), -90.f);
				else
					pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 90.f);

				m_ePhase = PHASE::ST;
				_pActor->Set_Animation("rush_st", false);

				m_tRushSFX.bActive = false;
			}
		}
			break;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
	}
};
NS_END