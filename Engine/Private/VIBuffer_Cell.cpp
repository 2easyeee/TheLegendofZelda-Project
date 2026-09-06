#include "VIBuffer_Cell.h"

CVIBuffer_Cell::CVIBuffer_Cell(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CVIBuffer { _pDevice, _pDeviceContext }
{
}

CVIBuffer_Cell::CVIBuffer_Cell(const CVIBuffer_Cell& _Prototype)
	: CVIBuffer (_Prototype)
{
}

HRESULT CVIBuffer_Cell::Initialize_Prototype(const _float3* _pPoints)
{
	m_iNumVertexBuffers = 1;
	m_iVertexStride = sizeof(VTXPOS);
	m_iNumVertices = 3;
	m_iIndexStride = 2;
	m_iNumIndices = 4;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitive = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

#pragma region VERTEXBUFFER
	D3D11_BUFFER_DESC VBDesc = {};
	VBDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	VTXPOS* pVertices = new VTXPOS[m_iNumVertices];
	memcpy(pVertices, _pPoints, sizeof(_float3) * m_iNumVertices);

	D3D11_SUBRESOURCE_DATA InitialVertexData = {};
	InitialVertexData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &InitialVertexData, &m_pVB)))
		return E_FAIL;
#pragma endregion

#pragma region INDEXBUFFER
	D3D11_BUFFER_DESC IBDesc = {};
	IBDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = 0;

	_ushort* pIndices = new _ushort[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;
	pIndices[3] = 0;

	D3D11_SUBRESOURCE_DATA InitialIndexData = {};
	InitialIndexData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &InitialIndexData, &m_pIB)))
		return E_FAIL;
#pragma endregion

	Safe_Delete_Array(pIndices);
	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CVIBuffer_Cell::Initialize(void* _pArg)
{
	return S_OK;
}

HRESULT CVIBuffer_Cell::Bind_Resources(D3D11_PRIMITIVE_TOPOLOGY _eType)
{
	ID3D11Buffer* pVertexBuffers[] = { m_pVB };
	_uint iVertexStrides[] = { m_iVertexStride };
	_uint iOffsets[] = { 0 };

	m_ePrimitive = _eType;

	m_pDeviceContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pDeviceContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	m_pDeviceContext->IASetPrimitiveTopology(m_ePrimitive);

	return S_OK;
}

HRESULT CVIBuffer_Cell::Render(_uint _iNumIndices)
{
	m_iNumIndices = _iNumIndices;
	m_pDeviceContext->DrawIndexed(m_iNumIndices, 0, 0);

	return S_OK;
}

CVIBuffer_Cell* CVIBuffer_Cell::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _float3* _pPoints)
{
	CVIBuffer_Cell* pInstance = new CVIBuffer_Cell(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype(_pPoints)))
	{
		MSG_BOX("FAILED TO CREATED : CVIBuffer_Cell");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CVIBuffer_Cell::Clone(void* _pArg)
{
	CVIBuffer_Cell* pInstance = new CVIBuffer_Cell(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("FAILED TO CLONED : CVIBuffer_Cell");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CVIBuffer_Cell::Free()
{
	__super::Free();
}
