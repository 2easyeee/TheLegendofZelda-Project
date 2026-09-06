#pragma once
#include "Base.h"
#include "ActorObject.h"

NS_BEGIN(Engine)
class ENGINE_DLL CActionController final : public CBase
{
public:
	CActionController(class CActorObject* _pOwner);
	virtual ~CActionController() = default;

public:
	void Priority_Update(_float _fTimeDelta);
	void Update(_float _fTimeDelta);
	void Late_Update(_float _fTimeDelta);
	void Push_Action(IState::TRANSITION_INPUT _eInput);

public:
	void Set_MoveInput(_vector _vDir);
	void Clear_MoveInput();
	_bool Is_MoveInputEnable();
	void Set_AimDirection(_vector _vDir);
	_vector Get_AimDirection() const;
	void Set_ActionPressed(IState::TRANSITION_INPUT _eInput, _bool _bPressed);
	_bool Is_ActionPressed(IState::TRANSITION_INPUT _eInput) const;
	void Force_StopAction();

private:
	class CActorObject* m_pOwner = { nullptr };
	queue<IState::TRANSITION_INPUT> m_ActionQueue;
	_bool m_bMoveInput = { false };
	_vector m_vMoveDirection = XMVectorZero();
	STATE_TIME m_fMoveHoldTime = { false, 0.f, 0.25f };
	STATE_TIME m_fActionRetry = { false, 0.f, 0.05f };
	_vector m_vAimDirection = XMVectorZero();
	unordered_set<IState::TRANSITION_INPUT> m_HoldActionSet;

private:
	void Update_ActionQueue(_float _fTimeDelta);
	void Update_Locomotion(_float _fTimeDelta);

public:
	static CActionController* Create(class CActorObject* _pOwner);
	virtual void Free() override;
};
NS_END