#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)
class ENGINE_DLL CVIBuffer_Cell final : public CVIBuffer
{
private:
	CVIBuffer_Cell(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CVIBuffer_Cell(const CVIBuffer_Cell& _Prototype);
	virtual ~CVIBuffer_Cell() = default;

public:
	virtual HRESULT Initialize_Prototype(const _float3* _pPoints);
	virtual HRESULT Initialize(void* _pArg) override;

	HRESULT Bind_Resources(D3D11_PRIMITIVE_TOPOLOGY _eType);
	HRESULT Render(_uint _iNumIndices);

public:
	static CVIBuffer_Cell* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		const _float3* _pPoints);
	virtual CComponent* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END