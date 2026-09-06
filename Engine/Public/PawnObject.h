#pragma once
#include "WorldObject.h"
#include "IPawnState.h"

// IState, WorldObject ¸¸ ³Ö±â !!!
NS_BEGIN(Engine)
class ENGINE_DLL CPawnObject abstract : public CWorldObject
{
public:
	typedef struct tagActorObjectDesc
	{
	}ACTOR_DESC;

	typedef struct tagActorInitDesc
	{
		OBJECT_DESC tObjectDesc;
		WORLD_DESC tWorldDesc;
		ACTOR_DESC tActorDesc;
	}ACTOR_INIT_DESC;

protected:
	CPawnObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CPawnObject(const CPawnObject& _Prototype);
	virtual ~CPawnObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;

public:
	/* State */
	void Register_State(class IPawnState* _pState);
	void RequestToChangeState(_uint _StateID);
	void Change_State();

	/* Transform */
	void Set_SpeedMulti(_float _fMulti);
	CTransform* Get_TrasformForMove() { if (m_pTransformCom) return m_pTransformCom; }
	_bool MoveWithNav(_vector _vMoveDir, _float _fTimeDelta);
	_bool ReflectWithNav(_vector& _vMoveDir, _float _fTimeDelta);

	/* Animation */
	void Set_Animation(_string _AnimName, _bool _bLoop = true, _bool _isForce = false);
	void Reset_Animation_TrackPosition();
	void Set_AnimationSpeedMulti(_float _fAnimSpeedMulti);
	_bool IsAnimFinished() const;
	_float Get_PlayRatio() const;

	/* Bone */
	void Set_BoneOffset(const _char* _BoneName, _fmatrix _offset);
	void Clear_BoneOffset();
	void Set_BoneSnap(const _char* _TargetBoneName, const _char* _SourceBoneName);
	void Clear_BoneSnap();

protected:
	/* Component */
	class CShader* m_pShaderCom = { nullptr };
	class CModel* m_pModelCom = { nullptr };
	class CTexture* m_pTextureCom = { nullptr };

	/* State */
	class IPawnState* m_pState = { nullptr };
	map<_uint, class IPawnState*> m_StateMap;
	_uint m_NexState = { 0 };
	_bool m_bRequestStateChange = { false };

	/* EFFECT (Physics) */
	STATE_TIME m_tKnockBack = { false, 0.f, 0.f };
	_float m_fKnockBackSpeed = { 0.f };
	_vector m_vKnockBackDir = XMVectorZero();

	/* EFFECT (Shader) */
	_float m_fHitStrength = { 0.f };
	STATE_TIME m_tHitTime = { false, 0.f, 0.2f };
	STATE_TIME m_tDissovleTime = { false, 0.f, 3.f };
	_bool m_bDeleteVFX = { false };

	/* Com */
	class CNavigation* m_pNavCom = { nullptr };

	/* Collider */
	class CCollider* m_pBodyCollider = { nullptr };

protected:
	HRESULT Ready_Components(OBJECT_DESC* _Desc);

	/* KnockBack */
	void Start_KnockBack(_vector _vHitPos, _float _fPower, _float _fDuration);
	void MoveWithKnockBack(_float _fTimeDelta);

	/* Shader Effect */
	void Start_HitFlash();
	void Start_Dissolve();
	virtual void Start_DeleteVFX();
	void Update_ShaderEffect(_float _fTimeDelta);
	HRESULT Render_Binding();

public:
	virtual CGameObject* Clone(void* _pArg) = 0;
	virtual void Free() override;
};
NS_END