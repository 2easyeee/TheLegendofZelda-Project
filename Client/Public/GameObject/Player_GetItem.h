#pragma once
#include "IState.h"
#include "GameObject/Player.h"
#include "GameInstance.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)
class CPlayer_GetItem : public IState
{
private:
	enum class PHASE { ST, LP, ED, END };
	PHASE m_ePhase = { PHASE::ST };

public:
	virtual _uint Get_StateID() const override
	{
		return ENUM_TO_UINT(STATE_ID::GET_ITEM);
	}
	virtual _bool Is_StateFinished() const override { return m_ePhase == PHASE::END; }

	virtual void OnStateEnter(class CActorObject* _pActor) override
	{
		m_ePhase = PHASE::ST;
		_pActor->Set_Animation("item_get_st", false);
		_pActor->Set_AnimationSpeedMulti(1.f);

		auto* pTransform = static_cast<CTransform*>(_pActor->Get_Component(TEXT("Com_Transform")));
		pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 0.f);
	}
	virtual void OnStateStay(class CActorObject* _pActor, _float _fTimeDelta) override
	{
		switch (m_ePhase)
		{
		case Client::CPlayer_GetItem::PHASE::ST:
		{
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::LP;
				_pActor->Set_Animation("item_get_lp", false);
			}
		}
			break;
		case Client::CPlayer_GetItem::PHASE::LP:
		{
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::ED;
				_pActor->Set_Animation("item_get_ed", false);
			}
		}
			break;

		case PHASE::ED:
		{
			if (_pActor->IsAnimFinished())
			{
				m_ePhase = PHASE::END;
			}
		}
		break;
		}
	}
	virtual void OnStateExit(class CActorObject* _pActor) override
	{
		_pActor->Set_AnimationSpeedMulti(1.f);
	}
};
NS_END