#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_BuzzBlob.h"

NS_BEGIN(Client)
class CBuzzBlob_Walk : public IPawnState
{
private:
	STATE_TIME m_MoveTime = { false, 0.f, 0.f };
	_vector m_MoveDir = XMVectorZero();

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_BuzzBlob::STATE_ID::WALK);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("walk", true);
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);

		/* LootAt */
		m_MoveTime.fAccTime = 0.f; // For. reuse
		m_MoveTime.fAccDurationTime = 3.f + (rand() % 100) / 100.f; // 1.f ~ 2.f

		auto* pTransform = _pActor->Get_TrasformForMove();
		if (!pTransform)
			return;
		_float randYaw = -180.f + (rand() % 360); // -45 ~ +45
		pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), randYaw);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		m_MoveTime.fAccTime += _fTimeDelta;
		if (m_MoveTime.fAccTime >= m_MoveTime.fAccDurationTime)
		{
			//_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_BuzzBlob::STATE_ID::IDLE));
			return;
		}
		auto* pTransform = _pActor->Get_TrasformForMove();
		if (!pTransform)
			return;

		_vector vDir = -pTransform->Get_State(STATE::LOOK);
		_bool bMoving = _pActor->MoveWithNav(vDir, _fTimeDelta);

		if (!bMoving)
		{
			_float randYaw = -120.f + (rand() % 240);
			pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), randYaw);

			m_MoveTime.fAccTime = 0.f;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);
	}
};
NS_END