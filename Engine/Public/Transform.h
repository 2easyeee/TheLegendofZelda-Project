#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class ENGINE_DLL CTransform final : public CComponent
{
public:
	typedef struct tagTransformDesc
	{
		_float fSpeedPerSec = { 1.f };
		_float fRotationPerSec = { 1.f };
	}TRANSFORM_DESC;

private:
	CTransform(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CTransform(const CTransform& _Prototype);
	virtual ~CTransform() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* _pArg);
	
	HRESULT Bind_WorldMatrix(class CShader* _pShader, const _char* _pConstantName);

public:
	_float3 Get_Scaled();
	_vector Get_State(STATE _eState);
	const _float4x4* Get_WorldMatrixPtr() const { return &m_WorldMatrix; }
	_float Get_MoveSpeed() const { return m_tTransformDesc.fSpeedPerSec * m_fSpeedMulti; }
	_float Get_RotationSpeed() const { return m_tTransformDesc.fRotationPerSec * m_fSpeedMulti; }

	void Set_Scale(_float _fX = 1.f, _float _fY = 1.f, _float _fZ = 1.f);
	void Set_State(STATE _eState, _fvector _vState);
	void Set_SpeedMulti(_float _fMulti) { m_fSpeedMulti = _fMulti; }
	void Set_RotationMulti(_float _fMulti) { m_fRotationMulti = _fMulti; }

	void Scaling(_float _fX = 1, _float _fY = 1.f, _float _fZ = 1.f);

public:
	void Go_Straight(_float _fTimeDelta);
	void Go_Backward(_float _fTimeDelta);
	void Go_Left(_float _fTimeDelta);
	void Go_Right(_float _fTimeDelta);
	void Go_Up(_float _fTimeDelta);
	void Go_Down(_float _fTimeDelta);

	void Rotation(_fvector _vAxis, _float _fDegree);
	void Turn(_fvector _vAxis, _float _fTimeDelta);

	void LookAt(_fvector _vAt);
	void MoveTo(_fvector _vDestPos, _float _fTimeDelta, _float _fMinDistance = 0.5f);
	void MoveWithRotation(_vector _vMoveDir, _float _fTimeDelta);

private:
	TRANSFORM_DESC m_tTransformDesc = {}; // Base
	_float4x4 m_WorldMatrix = {};

	_float m_fSpeedMulti = { 1.f };
	_float m_fRotationMulti = { 1.f };

public:
	_float3 GetEular(const _float4& q);
	void Update_ImGui_Inspector_Rotation(_float3 vEuler);

public:
	static CTransform* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CComponent* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END