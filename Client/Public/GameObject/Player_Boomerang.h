#pragma once
#include "IState.h"
#include "GameObject/Player.h"
#include "Transform.h"

NS_BEGIN(Client)
class CPlayer_Boomerang : public IState
{
private:
	enum class PHASE { ST, ED, END };
	PHASE m_ePhase;
	STATE_TIME m_tBoomerangTime = { false, 0.f, 0.25f };
	STATE_TIME m_tBoomerangEDTime = { false, 0.f, 0.25f };
	_float3 m_vThrowDir = {};

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::BOOMERANG);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_ePhase = PHASE::ST;
		m_tBoomerangTime.fAccTime = 0.f;
		m_tBoomerangEDTime.fAccTime = 0.f;

		auto* pPlayer = static_cast<CPlayer*>(_pActor);
		auto* pTransform = static_cast<CTransform*>(_pActor->Get_Component(TEXT("Com_Transform")));

		_vector vAim = pPlayer->Get_AimDirection();
		_vector vFinalDir = XMVectorZero();

		if (XMVectorGetY(vAim) < -0.7f)
		{
			vFinalDir = XMVector3Normalize(vAim);
		}
		else
		{
			_vector vLook = pTransform->Get_State(STATE::LOOK);
			vFinalDir = XMVector3Normalize(vLook);
		}

		XMStoreFloat3(&m_vThrowDir, vFinalDir);

		Set_BoomerangAnim(_pActor);

		/* SFX */
		CGameInstance::GetInstance()->Play_Sound(L"Player_Boomerang.wav", SOUND::EFFECT, 0.7f);
		CGameInstance::GetInstance()->Play_Sound(L"Player_Boomerang_Talk.wav", SOUND::EFFECT, 0.7f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		pPlayer->Apply_Gravity(_fTimeDelta);

		if (m_ePhase == PHASE::ST)
		{
			/* Animation Too Short ! */
			m_tBoomerangTime.fAccTime += _fTimeDelta;
			if (m_tBoomerangTime.fAccTime >= m_tBoomerangTime.fAccDurationTime)
			{
				m_ePhase = PHASE::ED;
				_pActor->Set_Animation("boomerang_ed", false); // s_boomerang_ed_u
				_pActor->Set_AnimationSpeedMulti(1.f);
				_pActor->Set_SpeedMulti(1.f);

				/* Throw */
				_pActor->Notify_WeaponThrow(m_vThrowDir);
			}
		}
		else if (m_ePhase == PHASE::ED)
		{
			m_tBoomerangEDTime.fAccTime += _fTimeDelta;
			if (m_tBoomerangEDTime.fAccTime >= m_tBoomerangEDTime.fAccDurationTime)
			{
				m_ePhase = PHASE::END;
			}
			return;
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
	void Set_BoomerangAnim(class CActorObject* _pActor)
	{
		_float fX = m_vThrowDir.x;
		_float fY = m_vThrowDir.y;

		if (fY > 0.7f)
			_pActor->Set_Animation("s_boomerang_st_u", false);
		else
			_pActor->Set_Animation("boomerang_st", false);
	}
};
NS_END