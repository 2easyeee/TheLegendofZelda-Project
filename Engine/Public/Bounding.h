#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CBounding abstract : public CBase
{
public:
	typedef struct tagBoundingDesc
	{
		_float3 vCenter;
	}BOUNDING_DESC;

protected:
	CBounding();
	virtual ~CBounding() = default;

public:
	virtual HRESULT Initialize(const BOUNDING_DESC* _pDesc);
	virtual void Update(_fmatrix _WorldMatrix) = 0;
	virtual _bool Intersect(COLLIDER _eType, CBounding* _pTarget) = 0;

public:
	virtual void Set_Extents(_float3 _vExtents) {};
	virtual void Set_Center(_float3 _vCenter) {};

public:
	virtual void Free() override;

#ifdef  _DEBUG
public:
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* _pBatch, _fvector _vColor) = 0;
#endif //  _DEBUG
};
NS_END
