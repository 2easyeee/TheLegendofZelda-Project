#pragma once
#include "IState.h"
#include "GameObject/Player.h"

NS_BEGIN(Client)
class CPlayer_Counter : public IState
{
private:
	enum class PHASE { SLASH, FINISH_LP, FINISH_ED, END };
	PHASE m_ePhase;
	_int m_iSlashCount = { 0 };
	_bool m_bAttackInput = { false };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::COUNTER);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }
    virtual _bool Block_Locomotion() const override
    {
        return false;
    }
	virtual void OnStateEnter(class CActorObject* _pActor)
	{
		_pActor->Set_Animation("slash", false);
		_pActor->Set_AnimationSpeedMulti(5.f);
		m_ePhase = PHASE::SLASH;
		m_iSlashCount = 0;
		m_bAttackInput = false;

		/* Jump_Dodge Y Reset */
		auto pTransform = static_cast<CTransform*>(_pActor->Get_Component(TEXT("Com_Transform")));
		if (!pTransform)
			return;
		_vector vPos = pTransform->Get_State(STATE::POSITION);
		vPos = XMVectorSetY(vPos, 0.f);
		pTransform->Set_State(STATE::POSITION, vPos);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta)
	{
		// Hit 연속 5대 때리고 -> "s_slaash_hold_lp -> s_slash_hold_ed" 이벤트 애니메이션 재생으로 끝내기
        auto pPlayer = static_cast<CPlayer*>(_pActor);

        if (pPlayer->Is_ActionPressed(IState::TRANSITION_INPUT::ATTACK_PRESSED))
        {
            m_bAttackInput = true;
        }

        if (!_pActor->IsAnimFinished())
            return;

        switch (m_ePhase)
        {
        case PHASE::SLASH:

            if (m_bAttackInput)
            {
                m_bAttackInput = false;
                m_iSlashCount++;

                if (m_iSlashCount >= 5)
                {
                    m_ePhase = PHASE::FINISH_LP;
                    _pActor->Set_Animation("s_slash_hold_lp", false);
                }
                else
                {
                    _pActor->Set_Animation("slash", false);
                }
            }
            else
            {
                m_ePhase = PHASE::END;
            }

            break;

        case PHASE::FINISH_LP:
        {
            m_ePhase = PHASE::FINISH_ED;
            _pActor->Set_Animation("s_slash_hold_ed", false);
        }
            break;

        case PHASE::FINISH_ED:
        {
            m_ePhase = PHASE::END;
        }
            break;
        }

	}
	virtual void OnStateExit(CActorObject* _pActor)
	{
	}
};
NS_END