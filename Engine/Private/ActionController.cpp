#include "ActionController.h"

CActionController::CActionController(CActorObject* _pOwner)
	: m_pOwner { _pOwner }
{
}

void CActionController::Priority_Update(_float _fTimeDelta)
{
	if (m_fActionRetry.fAccTime <= m_fActionRetry.fAccDurationTime)
		m_fActionRetry.fAccTime += _fTimeDelta;
}

void CActionController::Update(_float _fTimeDelta)
{
	Update_ActionQueue(_fTimeDelta);
	Update_Locomotion(_fTimeDelta);
}

void CActionController::Late_Update(_float _fTimeDelta)
{
}

void CActionController::Push_Action(IState::TRANSITION_INPUT _eInput)
{
	if (!m_pOwner || _eInput == IState::TRANSITION_INPUT::END)
		return;

	if (m_pOwner->Get_CurerntStateID() == ENUM_TO_UINT(IState::STATE_ID::COUNTER)
		&& _eInput == IState::TRANSITION_INPUT::ATTACK_PRESSED)
		return;

	if (_eInput == IState::TRANSITION_INPUT::DAMAGE_RECEVICED)
	{
		while (!m_ActionQueue.empty())
			m_ActionQueue.pop();
	}

	m_ActionQueue.push(_eInput);
}

void CActionController::Set_MoveInput(_vector _vDir)
{
	m_bMoveInput = true;
	m_vMoveDirection = _vDir;
}

void CActionController::Clear_MoveInput()
{
	m_bMoveInput = false;
	m_vMoveDirection = XMVectorZero();
}

_bool CActionController::Is_MoveInputEnable()
{
	return m_bMoveInput;
}

void CActionController::Set_AimDirection(_vector _vDir)
{
	m_vAimDirection = _vDir;
}

_vector CActionController::Get_AimDirection() const
{
	return m_vAimDirection;
}

void CActionController::Set_ActionPressed(IState::TRANSITION_INPUT _eInput, _bool _bPressed)
{
	if (_bPressed)
		m_HoldActionSet.insert(_eInput);
	else
		m_HoldActionSet.erase(_eInput);
}

_bool CActionController::Is_ActionPressed(IState::TRANSITION_INPUT _eInput) const
{
	return m_HoldActionSet.find(_eInput) != m_HoldActionSet.end();
}

void CActionController::Force_StopAction()
{
	while (!m_ActionQueue.empty())
		m_ActionQueue.pop();
	m_HoldActionSet.clear();
}

void CActionController::Update_ActionQueue(_float _fTimeDelta)
{
	if (!m_pOwner || m_ActionQueue.empty())
		return;
	if (m_fActionRetry.fAccTime < m_fActionRetry.fAccDurationTime)
		return;

	m_pOwner->Set_TransitionInput(m_ActionQueue.front());
	m_ActionQueue.pop();
	m_fActionRetry.fAccTime = 0.f;
}

void CActionController::Update_Locomotion(_float _fTimeDelta)
{
	if (!m_pOwner)
		return;

	auto StateID = m_pOwner->Get_CurerntStateID();
	if (StateID == ENUM_TO_UINT(IState::STATE_ID::SLEEP)
		|| StateID == ENUM_TO_UINT(IState::STATE_ID::SLEEP_INTRO)
		|| StateID == ENUM_TO_UINT(IState::STATE_ID::LADDER_UP_ST)
		|| StateID == ENUM_TO_UINT(IState::STATE_ID::WARP_UP))
		return;

	if (StateID == ENUM_TO_UINT(IState::STATE_ID::LADDER_UP_LP))
	{
		if (m_bMoveInput)
		{
			if (XMVectorGetY(m_vMoveDirection) > 0.f)
				m_pOwner->Go_Down(_fTimeDelta);
			else
				m_pOwner->Go_Up(_fTimeDelta);
		}
		return;
	}

	if (!m_bMoveInput)
	{
		m_pOwner->Set_TransitionInput(IState::TRANSITION_INPUT::MOVE_IDLE);
		m_fMoveHoldTime.fAccTime = 0.f;
		return;
	}

	m_fMoveHoldTime.fAccTime += _fTimeDelta;
	if (m_fMoveHoldTime.fAccTime >= m_fMoveHoldTime.fAccDurationTime)
		m_pOwner->Set_TransitionInput(IState::TRANSITION_INPUT::MOVE_RUN);
	else
		m_pOwner->Set_TransitionInput(IState::TRANSITION_INPUT::MOVE_WALK);

	if (m_pOwner->IsLocomotionEnable())
		m_pOwner->MovewithCollision(m_vMoveDirection, _fTimeDelta);
}

CActionController* CActionController::Create(CActorObject* _pOwner)
{
	return new CActionController(_pOwner);
}

void CActionController::Free()
{
	__super::Free();
}