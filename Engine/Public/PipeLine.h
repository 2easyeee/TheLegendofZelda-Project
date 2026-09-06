#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CPipeLine final : public CBase
{
private:
	CPipeLine();
	virtual ~CPipeLine() = default;

public:
	HRESULT Initialize();
	void Update();

	HRESULT Bind_CameraPosition(class CShader* _pShader, const _char* _pConstantName);
	HRESULT Bind_TransformState(class CShader* _pShader, const _char* _pConstantName, D3DTS _eTransformState);
	HRESULT Bind_InvTransformState(class CShader* _pShader, const _char* _pConstantName, D3DTS _eTransformState);

	void Set_Transform(D3DTS _eTransformState, _fmatrix _TransformMatrix);
	const _float4x4* Get_Transform(D3DTS _eTransformState);
	const _float4x4* Get_InvTransform(D3DTS _eTransformState);
	const _float4* Get_CameraPosition();

private:
	_float4		m_vCameraPosition = {};
	_float4x4	m_Transform[ENUM_TO_UINT(D3DTS::END)] = {};
	_float4x4	m_InvTransform[ENUM_TO_UINT(D3DTS::END)] = {};

public:
	static CPipeLine* Create();
	virtual void Free() override;
};
NS_END