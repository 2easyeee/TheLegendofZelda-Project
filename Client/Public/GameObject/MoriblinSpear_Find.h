#pragma once
#include "IActorState.h"
#include "GameObject/MoriblinSpear.h"

NS_BEGIN(Client)
class CMoriblinSpear_Find : public IActorState
{
public:
    virtual _uint Get_StateID() const override
    {
        return ENUM_TO_UINT(CMoriblinSpear::STATE_ID::FIND);
    }
    virtual void OnStateEnter(class CActorObject* _pActor) override
    {
        _pActor->Set_Animation("find", false);
    }
    virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
    {
        if (_pActor->IsAnimFinished())
        {
            _pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::STANCE_WALK));
            return;
        }
    }
    virtual void OnStateExit(class CActorObject* _pActor) override {}
};
NS_END