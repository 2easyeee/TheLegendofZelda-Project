#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class ENGINE_DLL CShader final : public CComponent
{
private:
	CShader(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CShader(const CShader& _Prototype);
	virtual ~CShader() = default;

public:
	virtual HRESULT Initialize_Prototype(const _tchar* _pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* _pElemnets, _uint _iNumElements);
	virtual HRESULT Initialize(void* _pArg);

	HRESULT Begin(_uint _iPassIndex);

	HRESULT Bind_RawValue(const _char* _pConstantName, const void* _pValue, _uint _iLength);
	HRESULT Bind_Matrix(const _char* _pConstantName, const _float4x4* _pMatrix);
	HRESULT Bind_Matrices(const _char* _pConstantName, const _float4x4* _pMatrix, _uint _iNumMatrices);
	HRESULT Bind_ShaderResourceView(const _char* _pConstantName, ID3D11ShaderResourceView* _pSRV);

private:
	ID3DX11Effect*				m_pEffect = { nullptr };
	_uint						m_iNumPasses = {};

	vector<ID3D11InputLayout*>	m_vecInputLayouts;

public:
	static CShader* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		const _tchar* _pShaderFilePath, 
		const D3D11_INPUT_ELEMENT_DESC* _pElements,
		_uint _iNumElements);
	virtual CComponent* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END