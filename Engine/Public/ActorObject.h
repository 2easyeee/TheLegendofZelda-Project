#pragma once
#include "ContainerObject.h"
#include "IState.h"
#include "ActionController.h"

NS_BEGIN(Engine)
class ENGINE_DLL CActorObject abstract : public CContainerObject
{
protected:
	CActorObject(ID3D11Device * _pDevice, ID3D11DeviceContext * _pDeviceContext);
	CActorObject(const CActorObject& _Prototype);
	virtual ~CActorObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	/* State */
	void Register_State(class IState* _pState);
	virtual void RequestToChangeState(_uint _stateID) {};
	
	virtual void Set_TransitionInput(IState::TRANSITION_INPUT _eInput) {}
	virtual void Evaluate_Transitions() {}
	
	void Change_State();
	_bool IsLocomotionEnable();
	_uint Get_CurerntStateID() const { return m_pState->Get_StateID(); }

	/* Action */
	_bool Is_OnGround() const;
	_bool Is_ActionPressed(IState::TRANSITION_INPUT _eInput) const;
	_bool Is_MoveInputEnable();

	/* Transform */
	void MoveWithRotation(_vector _vMoveDir, _float _fTimeDelta);
	void MovewithCollision(_vector _vMoveDir, _float _fTimeDelta);
	virtual _vector Filter_KnockBackAxis(_vector _vDir);
	void Go_Up(_float _fTimeDelta);
	void Go_Down(_float _fTimeDelta);
	void Set_SpeedMulti(_float _fMulti);

	/* Animation (Child) */
	virtual void Set_Animation(_string _AnimName, _bool _bLoop = true, _bool _isForce = false) {}
	virtual void Reset_Animation_TrackPosition() {}
	virtual void Set_AnimationSpeedMulti(_float _fAnimSpeedMulti) {}
	virtual _bool IsAnimFinished() const { return S_OK; }
	virtual _float Get_PlayRatio() const { return 0.f; }

	/* Action (Detail) */
	virtual void Notify_WeaponThrow(_float3 _vDir) {}

protected:
	/* Component */
	class CShader* m_pShaderCom = { nullptr };
	class CModel* m_pModelCom = { nullptr };
	
	/* State */
	class IState* m_pState = { nullptr };
	class IState* m_pStateArr[ENUM_TO_UINT(IState::STATE_ID::END)];
	_uint m_iNextStatePriority = {};
	_uint  m_NextState = {};
	_bool m_bRequestStateChange = { false };

	/* Action */
	class CActionController* m_pActionController = { nullptr };

	/* Collider */
	class CCollider* m_pBodyCollider = { nullptr };

	/* EFFECT (Physics) */
	STATE_TIME m_tKnockBack = { false, 0.f, 0.f };
	_float m_fKnockBackSpeed = { 0.f };
	_vector m_vKnockBackDir = XMVectorZero();

	/* Com */
	class CNavigation* m_pNavCom = { nullptr };

protected:
	void Start_KnockBack(_vector _vHitPos, _float _fPower, _float _fDuration);
	void MoveWithKnockBack(_float _fTimeDelta);

public:
	virtual CGameObject* Clone(void* _pArg) = 0;
	virtual void Free() override;
};
NS_END