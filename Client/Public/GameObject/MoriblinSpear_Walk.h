#pragma once
#include "IActorState.h"
#include "GameObject/MoriblinSpear.h"

NS_BEGIN(Client)
class CMoriblinSpear_WALK : public IActorState
{
private:
    STATE_TIME m_MoveTime = { false, 0.f, 0.f };
    _vector m_MoveDir = XMVectorZero();

public:
    virtual _uint Get_StateID() const override
    {
        return ENUM_TO_UINT(CMoriblinSpear::STATE_ID::WALK);
    }
    virtual void OnStateEnter(class CActorObject* _pActor) override
    {
        _pActor->Set_Animation("walk", true);
        _pActor->Set_AnimationSpeedMulti(1.f);
        _pActor->Set_SpeedMulti(1.f);

        m_MoveTime.fAccTime = 0.f;
        m_MoveTime.fAccDurationTime = 3.f + (rand() % 100) / 100.f;

        /* LookAt */
        _float fAngle = XMConvertToRadians(static_cast<_float>(rand() % 360));
        m_MoveDir = XMVectorSet(cosf(fAngle), 0.f, sinf(fAngle), 0.f);
        m_MoveDir = XMVector3Normalize(m_MoveDir);
    }
    virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
    {
        //_pActor->MoveWithRotation(m_MoveDir, _fTimeDelta); // deprecated
        _pActor->MovewithCollision(m_MoveDir, _fTimeDelta);

        m_MoveTime.fAccTime += _fTimeDelta;
        if (m_MoveTime.fAccTime >= m_MoveTime.fAccDurationTime)
        {
            _pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::IDLE));
            return;
        }
    }
    virtual void OnStateExit(class CActorObject* _pActor) override
    {
        _pActor->Set_AnimationSpeedMulti(1.f);
        _pActor->Set_SpeedMulti(1.f);
    }
};
NS_END