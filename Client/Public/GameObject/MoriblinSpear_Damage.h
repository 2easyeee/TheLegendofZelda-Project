#pragma once
#include "IActorState.h"
#include "GameObject/MoriblinSpear.h"

NS_BEGIN(Client)
class CMoriblinSpear_Damage : public IActorState
{
public:
    virtual _uint Get_StateID() const override
    {
        return ENUM_TO_UINT(CMoriblinSpear::STATE_ID::DAMAGE);
    }
    virtual void OnStateEnter(class CActorObject* _pActor) override
    {
        _pActor->Set_Animation("damage_f", false);

        /* SFX */
        CGameInstance::GetInstance()->Play_RandomSound(L"ENEMY_Moriblin_Damage", 3, 2.f);
    }
    virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
    {
        if (_pActor->IsAnimFinished())
        {
            _pActor->RequestToChangeState(ENUM_TO_UINT(CMoriblinSpear::STATE_ID::IDLE));
            return;
        }
    }
    virtual void OnStateExit(class CActorObject* _pActor) override {}
};
NS_END