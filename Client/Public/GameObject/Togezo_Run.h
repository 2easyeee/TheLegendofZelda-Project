#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Togezo.h"

NS_BEGIN(Client)
class CTogezo_Run : public IPawnState
{
private:
	enum class PAHSE { ST, LP, ED, END };
	CTogezo_Run::PAHSE m_ePhase;

	enum class DIR { LEFT, RIGHT, TOP, BOTTOM, END };
	CTogezo_Run::DIR m_eDir;

	STATE_TIME m_MoveTime = { false, 0.f, 0.f };
	_vector m_MoveDir = XMVectorZero();

	/* SFX */
	STATE_TIME m_tRunSFX;

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Togezo::STATE_ID::RUN);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("run", true);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(1.5f);
		_pActor->Set_SpeedMulti(2.f);
		m_ePhase = CTogezo_Run::PAHSE::LP;

		m_MoveTime.fAccTime = 0.f;

		/* SFX */
		m_tRunSFX.Start(0.2f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_Togezo*>(_pActor);
		auto pTarget = pMonster->Get_Target();
		if (!pTarget)
			return;

		CTransform* pTransform_My = static_cast<CTransform*>(_pActor->Get_Component(TEXT("Com_Transform")));
		CTransform* pTransform_Target = static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));

		_vector vMy = pTransform_My->Get_State(STATE::POSITION);
		_vector vTarget = pTransform_Target->Get_State(STATE::POSITION);

		_vector DeleteY = XMVectorSetY(vTarget - vMy, 0.f);
		_float fDist = XMVectorGetX(XMVector3Length(DeleteY));

		switch (m_ePhase)
		{
		case Client::CTogezo_Run::PAHSE::LP: 
		{
			/* SFX */
			if (m_tRunSFX.Tick_Loop(_fTimeDelta))
			{
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Togezo_Run.wav", SOUND::EFFECT, 0.8f);
			}

			_vector vDir = XMVector3Normalize(DeleteY);
			if (!_pActor->MoveWithNav(vDir, _fTimeDelta))
			{
				m_ePhase = PAHSE::ED;
				_pActor->Set_Animation("run_ed", false);
				_pActor->Reset_Animation_TrackPosition();
				_pActor->Set_AnimationSpeedMulti(1.f);
			}

			/* Distance */
			if (fDist < 1.2f)
			{
				m_ePhase = CTogezo_Run::PAHSE::ED;
				_pActor->Set_Animation("run_ed", false);
				_pActor->Reset_Animation_TrackPosition();
				_pActor->Set_AnimationSpeedMulti(1.f);

				/* Attack To Player */
				CEventManager::GAME_EVENT tEvent;
				tEvent.eType = CEventManager::GAME_EVENT_TYPE::PLAYER_DAMAGE;
				tEvent.vPos = vMy;
				tEvent.fValue1 = 1.f;
				CGameInstance::GetInstance()->Push_GameEVENT(tEvent);
			}
		}
			break;
		case Client::CTogezo_Run::PAHSE::ED:
		{
			if (_pActor->IsAnimFinished())
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Togezo::STATE_ID::IDLE));
		}
			break;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);
	}
	void Play_RunDir(class CPawnObject* _pActor, _float _fTimeDelta)
	{
		auto* pTransform = _pActor->Get_TrasformForMove();
		if (!pTransform)
			return;

		switch (m_eDir)
		{
		case DIR::LEFT:
			pTransform->Go_Backward(_fTimeDelta);
			break;
		case DIR::RIGHT:
			pTransform->Go_Backward(_fTimeDelta);
			break;
		case DIR::TOP:
			pTransform->Go_Backward(_fTimeDelta);
			break;
		case DIR::BOTTOM:
			pTransform->Go_Backward(_fTimeDelta);
			break;
		}
	}
	void Play_RunRot(class CPawnObject* _pActor)
	{
		auto* pTransform = _pActor->Get_TrasformForMove();
		if (!pTransform)
			return;

		_float fAngle = 0.f;
		switch (m_eDir)
		{
		case Client::CTogezo_Run::DIR::LEFT:
			fAngle = 90.f;
			break;
		case Client::CTogezo_Run::DIR::RIGHT:
			fAngle = -90.f;
			break;
		case Client::CTogezo_Run::DIR::TOP:
			fAngle = 180.f;
			break;
		case Client::CTogezo_Run::DIR::BOTTOM:
			fAngle = 0.f;
			break;
		}

		pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), fAngle);
	}
};
NS_END