#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MasterStalfon.h"

NS_BEGIN(Client)
class CMasterStalfon_Jump : public IPawnState
{
private:
	enum class PHASE { ST, LP, ED, END };
	PHASE m_tPhase;
	_float m_fJumpHeight = { 5.f };
	STATE_TIME m_tJumpTime = { false, 0.f, 0.5f };

	_vector m_vStartPos = XMVectorZero();
	_vector m_vTargetPos = XMVectorZero();

	/* SFX */
	STATE_TIME m_tJumpLPSFX;
	STATE_TIME m_tJumpEDSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::JUMP);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
 		_pActor->Set_Animation("jump_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		m_tPhase = CMasterStalfon_Jump::PHASE::ST;

		m_tJumpTime.fAccTime = 0.f;

		/**/
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		auto pTarget = pMonster->Get_Target();
		CTransform* pTransform_My = _pActor->Get_TrasformForMove();
		CTransform* pTransform_Target = static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));
		m_vStartPos = pTransform_My->Get_State(STATE::POSITION);
		m_vTargetPos = pTransform_Target->Get_State(STATE::POSITION);

		pMonster->Collider_BodyEnable(false);

		/* SFX */
		m_tJumpLPSFX.bActive = false;
		m_tJumpEDSFX.bActive = false;
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);

		switch (m_tPhase)
		{
		case CMasterStalfon_Jump::PHASE::ST:
		{
			if (_pActor->IsAnimFinished())
			{
				/* LookAt */
				auto pTransform = _pActor->Get_TrasformForMove();
				_vector vPos = pTransform->Get_State(STATE::POSITION);
				_vector vReverse = vPos - (m_vTargetPos - vPos);
				pTransform->LookAt(vReverse);

				///* State */
				m_tPhase = CMasterStalfon_Jump::PHASE::LP;
				_pActor->Set_Animation("jump_lp", false);
			}
		}
		break;
		case CMasterStalfon_Jump::PHASE::LP:
		{
			/* SFX */
			if (!m_tJumpLPSFX.bActive)
			{
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_Jump_LP.wav", SOUND::EFFECT, 2.5f);
				m_tJumpLPSFX.bActive = true;
			}

			/* LP */
			m_tJumpTime.fAccTime += _fTimeDelta;
			if (m_tJumpTime.fAccTime >= m_tJumpTime.fAccDurationTime)
			{
				/* Move To Target */
				auto pTransform = _pActor->Get_TrasformForMove();

				_float fRatio = _pActor->Get_PlayRatio();
				_float ease = EaseInOutCubic(fRatio);

				_vector vStartXZ = XMVectorSetY(m_vStartPos, 0.f);
				_vector vTargetXZ = XMVectorSetY(m_vTargetPos, 0.f);

				_vector vXZ = XMVectorLerp(vStartXZ, vTargetXZ, ease);

				/* Jump Height */
				//float height = sinf(fRatio * XM_PI) * 2.5f;
				_float height = 4.f * m_fJumpHeight * fRatio * (1.f - fRatio);

				_vector vPos = XMVectorSet(
					XMVectorGetX(vXZ),
					XMVectorGetY(m_vStartPos) + height,
					XMVectorGetZ(vXZ),
					1.f);

				pTransform->Set_State(STATE::POSITION, vPos);

				_vector vCur = pTransform->Get_State(STATE::POSITION);
				_vector vDiff = XMVectorSetY(m_vTargetPos - vCur, 0.f);

				if (XMVectorGetX(XMVector3LengthSq(vDiff)) < 0.01f || fRatio >= 0.99f)
				{
					/* Á¤È®È÷ ÂøÁö */
					pTransform->Set_State(STATE::POSITION, m_vTargetPos);

					m_tPhase = PHASE::ED;
					_pActor->Set_Animation("jump_ed", false);
				}
			}
		}
		break;
		case CMasterStalfon_Jump::PHASE::ED:
		{
			/* SFX */
			if (!m_tJumpEDSFX.bActive)
			{
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_Jump_ED.wav", SOUND::EFFECT, 2.5f);
				m_tJumpEDSFX.bActive = true;
			}

			if (_pActor->IsAnimFinished())
			{
				pMonster->NextPattern();
				pMonster->Run_CurrentPattern();
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