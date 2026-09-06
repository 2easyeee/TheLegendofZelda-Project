#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Albatoss.h"

NS_BEGIN(Client)
class CAlbatoss_Hovering : public IPawnState
{
private:
	/* Common */
	enum class PHASE { ST, LP, END };
	PHASE m_ePhase;
	CMonster_Albatoss::PATTERN m_eNextPattern;

	/* Rush */
	_vector m_vTargetPos = XMVectorZero();
	_float m_fDir = 1.f;

	/* Flapping */

	/* Claw */

	/* SFX */
	STATE_TIME m_tFlapSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::HOVERING);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);

		_pActor->Set_Animation("hovering_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);

		/* Pattern */
		m_ePhase = PHASE::ST;
		m_eNextPattern = pMonster->Get_CurrentPattern();

		/* Collider */
		pMonster->Collider_BodyEnable(false);

		/* SFX */
		m_tFlapSFX.Start(0.8f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		CTransform* pTransform = _pActor->Get_TrasformForMove();
		switch (m_ePhase)
		{
		case CAlbatoss_Hovering::PHASE::ST:
		{
			if (_pActor->IsAnimFinished())
			{
				_vector vPos = pTransform->Get_State(STATE::POSITION);
				_float fX = XMVectorGetX(vPos);
				_bool bLeft = (fX > 0.f);

				_float fY = 0.f;

				switch (m_eNextPattern)
				{
				case Client::CMonster_Albatoss::PATTERN::RUSH: 
				{
					fX = bLeft ? -15.f : 15.f;
					fY = 8.f;
				}
					break;
				case Client::CMonster_Albatoss::PATTERN::FLAPPING:
				{
					fX = bLeft ? -7.f : 7.f;
					fY = 5.f;
				}
					break;
				case Client::CMonster_Albatoss::PATTERN::CLAW:
				{
					fX = bLeft ? -3.f : 3.f;
					fY = 0.25f;
				}
					break;
				}

				m_vTargetPos = XMVectorSet(fX, fY, 0.f, 1.f);

				/* Rotation */
				if (fX < 0.f)
					pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 90.f);
				else
					pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), -90.f);

				/* State */
				_pActor->Set_Animation("hovering_lp", true);
				_pActor->Set_AnimationSpeedMulti(1.5f);
				m_ePhase = PHASE::LP;
			}
		}
			break;
		case CAlbatoss_Hovering::PHASE::LP:
		{
			/* SFX */
			if (m_tFlapSFX.Tick_Loop(_fTimeDelta))
				CGameInstance::GetInstance()->Play_RandomSound(L"ENEMY_Boss_Albatoss_Fly", 3, 1.5f);

			_vector vPos = pTransform->Get_State(STATE::POSITION);
			_vector vDir = XMVector3Normalize(m_vTargetPos - vPos);

			_float fSpeed = 6.f;
			if (m_eNextPattern == CMonster_Albatoss::PATTERN::RUSH)
				fSpeed = 10.f;
			else if (m_eNextPattern == CMonster_Albatoss::PATTERN::CLAW)
				fSpeed = 8.f;

			vPos += vDir * fSpeed * _fTimeDelta;
			pTransform->Set_State(STATE::POSITION, vPos);

			_float fDist = XMVectorGetX(XMVector3Length(m_vTargetPos - vPos));

			if (fDist < 0.3f)
			{
				switch (m_eNextPattern)
				{
				case Client::CMonster_Albatoss::PATTERN::RUSH:
				{
					_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::RUSH));
				}
					break;
				case Client::CMonster_Albatoss::PATTERN::FLAPPING:
				{
					_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::ATK_FLAPPING));
				}
					break;
				case Client::CMonster_Albatoss::PATTERN::CLAW:
				{
					_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Albatoss::STATE_ID::ATK_CLAW));
				}
					break;
				}
			}
		}
			break;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		auto pMonster = static_cast<CMonster_Albatoss*>(_pActor);
		pMonster->Collider_BodyEnable(true);
	}
};
NS_END