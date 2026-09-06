#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Bomber.h"

NS_BEGIN(Client)
class CBomber_Browoff : public IPawnState
{
private:
	STATE_TIME m_tMoveTime = { false, 0.f, 5.f };
	_vector m_vDir = XMVectorZero();

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Bomber::STATE_ID::BROWOFF);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("browoff", true);
		_pActor->Set_AnimationSpeedMulti(1.f);

		m_tMoveTime.fAccTime = 0.f;

		/* Init KnockBack Dir */
		auto pMonster = static_cast<CMonster_Bomber*>(_pActor);
		auto pTarget = pMonster->Get_Target();
		if (pTarget)
		{
			auto pMyTransform = static_cast<CTransform*>(pMonster->Get_Component(TEXT("Com_Transform")));
			auto pTargetTransfrom = static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));

			_vector vMyPos = pMyTransform->Get_State(STATE::POSITION);
			_vector vTargetPos = pTargetTransfrom->Get_State(STATE::POSITION);

			m_vDir = vMyPos - vTargetPos;
			m_vDir = XMVectorSetY(m_vDir, 0.f);

			if (XMVector3LengthSq(m_vDir).m128_f32[0] < 0.00001f)
				m_vDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);

			m_vDir = XMVector3Normalize(m_vDir);

			/* Rotate 45µµ */
			_matrix RotMatrix = XMMatrixRotationY(XMConvertToRadians(45.f));
			m_vDir = XMVector3TransformNormal(m_vDir, RotMatrix);
			m_vDir = XMVector3Normalize(m_vDir);
		}
		

		_pActor->Set_SpeedMulti(7.f);
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_Bomber*>(_pActor);
		m_tMoveTime.fAccTime += _fTimeDelta;
		
		_float step = _fTimeDelta / 3.f;
		for (_int i = 0; i < 3; i++)
		{
			if (_pActor->ReflectWithNav(m_vDir, step))
			{
				/* SFX */
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Bomber_Damage.wav", SOUND::EFFECT);
			}
		}


		if (m_tMoveTime.fAccTime >= m_tMoveTime.fAccDurationTime)
		{
			pMonster->Notify_Delete();
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
		_pActor->Set_SpeedMulti(1.f);
	}
};
NS_END