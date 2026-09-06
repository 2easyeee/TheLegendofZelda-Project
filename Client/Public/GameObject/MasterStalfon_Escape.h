#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MasterStalfon.h"
#include "GameObject/MasterStalfon_ShadowIndicator.h"

NS_BEGIN(Client)
class CMasterStalfon_Escape : public IPawnState
{
private:
	/* Escape */
	STATE_TIME m_tEscapeTime = { false, 0.f, 1.2f };
	enum class PHASE { ESCAPE, POP, END };
	PHASE m_ePhase;

	/* Tracking */
	STATE_TIME m_tTrackTime = { false, 0.f, 3.f };

	/* MoveTo */
	_vector m_vStartPos = XMVectorZero();
	_vector m_vTargetPos = XMVectorZero();

	/* Target */
	_bool m_bDamage = { false };

	/* Shadow */
	CMasterStalfon_ShadowIndicator* m_pShadow = { nullptr };

	/* SFX */
	STATE_TIME m_tDownLPSFX;
	STATE_TIME m_tDownEDSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::ESCAPE);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("escape", false);
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);

		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		pMonster->Collider_BodyEnable(false);

		auto pTransform = _pActor->Get_TrasformForMove();
		m_vStartPos = pTransform->Get_State(STATE::POSITION);

		m_tEscapeTime.fAccTime = 0.f;
		m_tEscapeTime.bActive = false;
		m_ePhase = PHASE::ESCAPE;
		m_bDamage = false;

		/* Shadow */
		auto pTarget = pMonster->Get_Target();
		if (pTarget)
		{
			auto pTargetTransform = static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));
			m_vTargetPos = pTargetTransform->Get_State(STATE::POSITION);

			CMapObject::MAP_INIT_DESC tDesc = {};
			SET_DESC(tDesc.tObjectDesc.ObjectID, TEXT("Shadow"));

			CGameObject* pGameObject = { nullptr };
			if (FAILED(CGameInstance::GetInstance()->Add_GameObject_ToLayer(
				RESOURCE_LEVEL_STATIC,
				TEXT("Prototype_GameObject_Map_MasterStalfon_ShadowIndicator"),
				ENUM_TO_UINT(LEVEL::DUNGEON),
				TEXT("Layer_Object"),
				&tDesc, &pGameObject)))
				return;
			m_pShadow = static_cast<CMasterStalfon_ShadowIndicator*>(pGameObject);
		
			if (m_pShadow)
				m_pShadow->Set_Position(m_vTargetPos);
		}

		/* SFX */
		m_tDownLPSFX.bActive = false;
		m_tDownEDSFX.bActive = false;
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		auto pTransform = _pActor->Get_TrasformForMove();

		auto pTarget = pMonster->Get_Target();
		if (pTarget && m_pShadow)
		{
			auto pTargetTransform = static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));
			m_vTargetPos = pTargetTransform->Get_State(STATE::POSITION);
			m_pShadow->Set_Position(m_vTargetPos);
		}

		switch (m_ePhase)
		{
		case CMasterStalfon_Escape::PHASE::ESCAPE:
		{
			m_tEscapeTime.fAccTime += _fTimeDelta;

			_vector vPos = pTransform->Get_State(STATE::POSITION);
			vPos += XMVectorSet(0.f, 1.f, 0.f, 0.f) * 5.f * _fTimeDelta;
			pTransform->Set_State(STATE::POSITION, vPos);

			auto pTarget = pMonster->Get_Target();
			if (pTarget)
			{
				auto pTargetTransform = static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));
				m_vTargetPos = pTargetTransform->Get_State(STATE::POSITION);
			}
			if (m_tEscapeTime.fAccTime >= m_tEscapeTime.fAccDurationTime)
			{
				_pActor->Set_Animation("pop", false);
				_pActor->Reset_Animation_TrackPosition();
				m_ePhase = PHASE::POP;

				/* SFX */
				if (!m_tDownLPSFX.bActive)
				{
					CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_Down.wav", SOUND::EFFECT);
					m_tDownLPSFX.bActive = true;
				}
			}
			return;
		}
			break;
		case CMasterStalfon_Escape::PHASE::POP:
		{
			_float fRatio = _pActor->Get_PlayRatio();

			if (fRatio >= 0.5f && fRatio <= 0.6f)
			{
				/* 위치 보정 */
				pTransform->Set_State(STATE::POSITION, m_vTargetPos);

				/* LookAt 보정 */
				auto pTargetTransform = static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));
				_vector vPlayer = pTargetTransform->Get_State(STATE::POSITION);
				_vector vCurrent = pTransform->Get_State(STATE::POSITION);

				_vector vDir = vPlayer - vCurrent;
				_vector vReverseTarget = vCurrent - vDir;
				pTransform->LookAt(vReverseTarget);

				if (m_pShadow)
				{
					m_pShadow->Destroy();
					m_pShadow = nullptr;
				}
			}

			if (!m_bDamage && fRatio >= 0.75f)
			{
				/* SFX */
				if (!m_tDownEDSFX.bActive)
				{
					pMonster->Notify_CameraShake();
					CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_MasterStalfon_Down_ED.wav", SOUND::EFFECT, 2.5f);
					m_tDownEDSFX.bActive = true;
				}

				auto pTarget = pMonster->Get_Target();
				if (pTarget)
				{
					auto pTargetTransform = static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));

					_vector vPlayer = pTargetTransform->Get_State(STATE::POSITION);
					_vector vCurrent = pTransform->Get_State(STATE::POSITION);

					_vector vDist = XMVectorSetY(vPlayer - vCurrent, 0.f);
					_float fDist =XMVectorGetX(XMVector3Length(vDist));

					if (fDist < 2.5f)
					{
						CEventManager::GAME_EVENT tEvent;
						tEvent.eType = CEventManager::GAME_EVENT_TYPE::PLAYER_DAMAGE;
						tEvent.vPos = vCurrent;
						tEvent.fValue1 = 1.f;
						CGameInstance::GetInstance()->Push_GameEVENT(tEvent);
					}
				}

				m_bDamage = true;
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
		if (m_pShadow)
		{
			m_pShadow->Destroy();
			m_pShadow = nullptr;
		}
	}
};
NS_END