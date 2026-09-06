#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Togezo.h"

NS_BEGIN(Client)
class CTogezo_Walk : public IPawnState
{
private:
	enum class DIR { LEFT, RIGHT, TOP, BOTTOM, END };
	CTogezo_Walk::DIR m_eDir;

	STATE_TIME m_MoveTime = { false, 0.f, 3.f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Togezo::STATE_ID::WALK);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("walk", true);
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);
		m_MoveTime.fAccTime = 0.f;
		m_MoveTime.fAccDurationTime = 1.5f + (rand() % 150) / 100.f; // 1.f ~ 3.f
		m_eDir = static_cast<DIR>(rand() % ENUM_TO_UINT(DIR::END));
		Play_WalkRot(_pActor);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_Togezo*>(_pActor);
		if (pMonster->Get_Target())
		{
			_float fDist = pMonster->Compute_Distance_To_Target();
			if (fDist < 5.f)
			{
				_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Togezo::STATE_ID::DISCOVER));
				return;
			}
		}

		m_MoveTime.fAccTime += _fTimeDelta;
		Play_WalkDir(_pActor, _fTimeDelta);
		if (m_MoveTime.fAccTime >= m_MoveTime.fAccDurationTime)
		{
			_pActor->RequestToChangeState(ENUM_TO_UINT(CMonster_Togezo::STATE_ID::IDLE));
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);
	}
	void Play_WalkDir(class CPawnObject* _pActor, _float _fTimeDelta)
	{
		_vector vDir = XMVectorZero();

		switch (m_eDir)
		{
		case DIR::LEFT:
			vDir = XMVectorSet(-1.f, 0.f, 0.f, 0.f);
			break;
		case DIR::RIGHT:
			vDir = XMVectorSet(1.f, 0.f, 0.f, 0.f);
			break;
		case DIR::TOP:
			vDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);
			break;
		case DIR::BOTTOM:
			vDir = XMVectorSet(0.f, 0.f, -1.f, 0.f);
			break;
		}

		_pActor->MoveWithNav(vDir, _fTimeDelta);
	}
	void Play_WalkRot(class CPawnObject* _pActor)
	{
		auto* pTransform = _pActor->Get_TrasformForMove();
		if (!pTransform)
			return;

		_float fAngle = 0.f;
		switch (m_eDir)
		{
		case CTogezo_Walk::DIR::LEFT:
			fAngle = 90.f;
			break;
		case CTogezo_Walk::DIR::RIGHT:
			fAngle = -90.f;
			break;
		case CTogezo_Walk::DIR::TOP:
			fAngle = 180.f;
			break;
		case CTogezo_Walk::DIR::BOTTOM:
			fAngle = 0.f;
			break;
		}

		pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), fAngle);
	}
};
NS_END