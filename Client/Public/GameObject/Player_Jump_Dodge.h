#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Jump_Dodge : public IState
{
private:
	enum class PHASE { AIR, LAND, END };
	PHASE m_ePhase;

	/* Jump */
	_float m_fJumpPower = { 8.f };
	_float m_fGravity = { -22.f };
	_float m_fVelocityY = { 0.f };

	_float m_fFixedZ = { 0.f };
	_float m_fGroundY = { 0.f };

	/* Forward Impulse */
	_float m_fForwardSpeed = { 2.f };
	_vector m_vForwardDir = XMVectorZero();

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::JUMP_DODGE);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }
	virtual _bool Block_Locomotion() const override
	{
		return false;
	}
	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_ePhase = PHASE::AIR;
		_pActor->Set_Animation("jump_slash", false);
		_pActor->Reset_Animation_TrackPosition();
		_pActor->Set_AnimationSpeedMulti(2.5f);
		_pActor->Set_SpeedMulti(1.f);

		/* Jump */
		auto pTransform = static_cast<CTransform*>(_pActor->Get_Component(TEXT("Com_Transform")));
		if (!pTransform)
			return;
		
		/* Jump Pos */
		_vector vPos = pTransform->Get_State(STATE::POSITION);
		m_fFixedZ = XMVectorGetZ(vPos);
		m_fGroundY = XMVectorGetY(vPos);

		m_fVelocityY = m_fJumpPower;
		m_fForwardSpeed = 2.f;

		/* Forward Impulse */
		 m_vForwardDir = XMVector3Normalize(pTransform->Get_State(STATE::LOOK)) * -1.f;
		
		 auto pPlayer = static_cast<CPlayer*>(_pActor);
		 pPlayer->Set_DodgePhase(CPlayer::DODGE::NONE);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		auto pPlayer = static_cast<CPlayer*>(_pActor);
		auto pTransform = static_cast<CTransform*>(_pActor->Get_Component(TEXT("Com_Transform")));
		if (!pTransform)
			return;

		if (m_ePhase == PHASE::AIR)
		{
			
			_float fRatio = _pActor->Get_PlayRatio();
			if (fRatio <= 0.2f)
			{
				pPlayer->Set_DodgePhase(CPlayer::DODGE::SLOW);
			}
			else
			{
				pPlayer->Set_DodgePhase(CPlayer::DODGE::ATTACK);
			}

			m_fVelocityY += m_fGravity * _fTimeDelta;
			_vector vPos = pTransform->Get_State(STATE::POSITION);
			_float fNewY = XMVectorGetY(vPos) + m_fVelocityY * _fTimeDelta;

			/* Check Ground */
			if (fNewY <= m_fGroundY && fRatio >= 0.9f)
			{
				fNewY = m_fGroundY;
				m_fVelocityY = 0.f;
				m_ePhase = PHASE::LAND;

				_pActor->Set_Animation("land", false);
			}

			/* X °íÁ¤ ver. */ 	/* Forward Impulse */
			_vector vNewPos = XMVectorSetY(vPos, fNewY);
			vNewPos += m_vForwardDir * m_fForwardSpeed * _fTimeDelta;
			pTransform->Set_State(STATE::POSITION, vNewPos);
			m_fForwardSpeed *= 0.98f;
		}
		else if (m_ePhase == PHASE::LAND)
		{
			if (_pActor->IsAnimFinished())
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
};
NS_END