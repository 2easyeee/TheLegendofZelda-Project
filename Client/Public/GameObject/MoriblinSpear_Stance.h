#pragma once
#include "IActorState.h"
#include "GameObject/MoriblinSpear.h"

NS_BEGIN(Client)
class CMoriblinSpear_Stance : public IActorState
{
public:
    virtual _uint Get_StateID() const override
    {
        return ENUM_TO_UINT(CMoriblinSpear::STATE_ID::STANCE);
    }
    virtual void OnStateEnter(class CActorObject* _pActor) override
    {
        _pActor->Set_Animation("stance_walk_f", false);
        _pActor->Set_AnimationSpeedMulti(1.f);
        _pActor->Set_SpeedMulti(1.f);
    }
    virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
    {
        if (_pActor->IsAnimFinished())
        {
            _pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::ATTACK));
            return;
        }
    }
    virtual void OnStateExit(class CActorObject* _pActor) override {}
};
NS_END