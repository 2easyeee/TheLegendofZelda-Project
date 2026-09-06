#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class ENGINE_DLL CCell final : public CBase
{
public:
	enum POINT { A, B, C, POINT_END };
	enum LINE { AB, BC, CA, LINE_END };

private:
	CCell(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CCell() = default;

public:
	HRESULT Initialize(const _float3* _pPoints, _int _iIndex);
	
	_bool IsIn(_fvector _vPosition, _int* _pNeighborIndex);
	_bool Compare_Points(_fvector _vSourPoint, _fvector _vDestPoint);
	_float Compute_Height(_fvector _vPosition);

public:
	_vector Get_Point(POINT _ePoint);
	void Set_Point(POINT _ePoint, _fvector _vPoint);

	void Set_Neighbor(LINE _eLine, CCell* _pNeighbor);
	void Set_Neighbor(const _int* _pNeighbors);
	_int* Get_NeighborIndices();

	_int Get_SharedPointCount(class CCell* _pOther);
	void Link_Neighbor(CCell* _pOther);

	void Set_Index(_int _iIndex);
	_int Get_Index() const;

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	_float3 m_vPoints[POINT::POINT_END] = {};
	_float3 m_vNormals[LINE::LINE_END] = {};

	_int m_iIndex = { -1 };

	_int m_iNeighborIndices[LINE::LINE_END] = { -1, -1, -1 };
	_float4 m_Plane = {};

private:
	_bool IsSamePoint(_fvector _vA, _fvector _vB);
	_bool IsSameEdge(POINT _a0, POINT _a1, CCell* _pOther);

public:
	static CCell* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		const _float3* _pPoints, _int _iIndex);
	virtual void Free() override;

#ifdef _DEBUG
private:
	class CVIBuffer_Cell* m_pVIBuffer = { nullptr };

public:
	HRESULT Render(D3D11_PRIMITIVE_TOPOLOGY _eType, _uint _iNumIndices);
#endif // _DEBUG

};
NS_END