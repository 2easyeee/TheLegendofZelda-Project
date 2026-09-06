#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_MasterStalfon.h"

NS_BEGIN(Client)
class CMasterStalfon_Teleport : public IPawnState
{
private:
	enum class PHASE { DISAPPEAR, WAIT, APPEAR, END };
	PHASE m_ePhase;
	STATE_TIME m_tWaitTime = { false, 0.f, 0.5f };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_MasterStalfon::STATE_ID::TELEPORT);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		_pActor->Set_Animation("idle", true);
		_pActor->Set_AnimationSpeedMulti(1.f);
		_pActor->Set_SpeedMulti(1.f);

		m_ePhase = PHASE::DISAPPEAR;
		m_tWaitTime.fAccTime = 0.f;
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_MasterStalfon*>(_pActor);
		auto pTransform = _pActor->Get_TrasformForMove();

		switch (m_ePhase)
		{
		case CMasterStalfon_Teleport::PHASE::DISAPPEAR:
		{
			pMonster->Render_Enable(false);
			pMonster->Collider_BodyEnable(false);
			m_ePhase = PHASE::WAIT;
		}
			break;
		case CMasterStalfon_Teleport::PHASE::WAIT:
		{
			m_tWaitTime.fAccTime += _fTimeDelta;
			if (m_tWaitTime.fAccTime >= m_tWaitTime.fAccDurationTime)
			{
				_float3 vMin = { 85.f, 0.f, 8.f };
				_float3 vMax = { 95.f, 0.f, 16.f };

				auto pTarget = pMonster->Get_Target();
				CTransform* pTransform_Target = static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));
				_vector pPlayerPos = pTransform_Target->Get_State(STATE::POSITION);

				_vector vTeleportPos = Get_RandomRoomPos(vMin, vMax, pPlayerPos);
				pTransform->Set_State(STATE::POSITION, vTeleportPos);

				pMonster->Render_Enable(true);
				pMonster->Collider_BodyEnable(true);

				m_ePhase = PHASE::APPEAR;
			}
		}
			break;
		case CMasterStalfon_Teleport::PHASE::APPEAR:
		{
			pMonster->NextPattern();
			pMonster->Run_CurrentPattern();
		}
			break;
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
	}

private:
	_vector Get_RandomRoomPos(_float3 _vMin, _float3 _vMax, _vector _vPlayerPos)
	{
		for (int i = 0; i < 10; ++i)
		{
			float fRandX = _vMin.x + static_cast<float>(rand()) / RAND_MAX * (_vMax.x - _vMin.x);
			float fRandZ = _vMin.z + static_cast<float>(rand()) / RAND_MAX * (_vMax.z - _vMin.z);

			_vector vPos = XMVectorSet(fRandX, 0.f, fRandZ, 1.f);
			_vector vDist = XMVectorSetY(vPos - _vPlayerPos, 0.f);
			if (XMVectorGetX(XMVector3Length(vDist)) > 3.f)
				return vPos;
		}

		return XMVectorSet(_vMin.x, 0.f, _vMin.z, 1.f);
	}
};
NS_END