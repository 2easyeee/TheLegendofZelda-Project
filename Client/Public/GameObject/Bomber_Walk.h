#pragma once
#include "IPawnState.h"
#include "GameObject/Monster_Bomber.h"

NS_BEGIN(Client)
class CBomber_Walk : public IPawnState
{
private:
	STATE_TIME m_tDirTime = { false, 0.f, 2.f };
	_vector m_vDir = XMVectorZero();

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(CMonster_Bomber::STATE_ID::WALK);
	}
	virtual void OnStateEnter(class CPawnObject* _pActor) override
	{
		auto pMonster = static_cast<CMonster_Bomber*>(_pActor);
		Set_RandomDir(pMonster);

		m_tDirTime.fAccTime = 0.f;
	}
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) override
	{
		auto pMonster = static_cast<CMonster_Bomber*>(_pActor);

		_bool bMoveing = pMonster->MoveWithNav(m_vDir, _fTimeDelta);

		if (!bMoveing)
		{
			Set_RandomDir(pMonster);
		}

		m_tDirTime.fAccTime += _fTimeDelta;
		if (m_tDirTime.fAccTime >= m_tDirTime.fAccDurationTime)
		{
			m_tDirTime.fAccTime = 0.f;
			Set_RandomDir(pMonster);
		}
	}
	virtual void OnStateExit(class CPawnObject* _pActor) override
	{
	}

private:
	void Set_RandomDir(class CMonster_Bomber* _pMonster)
	{
		_int iDir = rand() % 4;
		switch (iDir)
		{
			case 0: m_vDir = XMVectorSet(0.f,0.f, +1.f, 0.f); break;
			case 1: m_vDir = XMVectorSet(0.f,0.f, -1.f, 0.f); break;
			case 2: m_vDir = XMVectorSet(-1.f,0.f, 0.f, 0.f); break;
			case 3: m_vDir = XMVectorSet(+1.f,0.f, 0.f, 0.f); break;
		}
		Update_Animation(_pMonster);
	}
	void Update_Animation(class CMonster_Bomber* _pMonster)
	{
		_float fX = XMVectorGetX(m_vDir);
		_float fZ = XMVectorGetZ(m_vDir);

		if (fZ > 0.f) _pMonster->Set_Animation("walk_f", true);
		else if (fZ < 0.f) _pMonster->Set_Animation("walk_b", true);
		else if (fX < 0.f) _pMonster->Set_Animation("walk_l", true);
		else if (fX > 0.f) _pMonster->Set_Animation("walk_r", true);

		_pMonster->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END