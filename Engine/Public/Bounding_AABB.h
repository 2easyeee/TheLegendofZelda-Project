#pragma once
#include "Bounding.h"

NS_BEGIN(Engine)
class CBounding_AABB final : public CBounding
{
public:
	typedef struct tagAABBDesc final : public CBounding::BOUNDING_DESC
	{
		_float3 vExtents;
	}AABB_DESC;

private:
	CBounding_AABB();
	virtual ~CBounding_AABB() = default;

public:
	virtual HRESULT Initialize(const BOUNDING_DESC* _pDesc) override;
	virtual void Update(_fmatrix _WorldMatrix) override;
	virtual _bool Intersect(COLLIDER _eType, CBounding* _pTarget) override;

	const BoundingBox* Get_Desc() const { return m_pDesc; }
	virtual void Set_Extents(_float3 _vExtents) override;
	virtual void Set_Center(_float3 _vCenter) override;

private:
	BoundingBox* m_pOriginalDesc = {};
	BoundingBox* m_pDesc = {};

public:
	static CBounding_AABB* Create(const BOUNDING_DESC* _pDesc);
	virtual void Free() override;

#ifdef _DEBUG
public:
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* _pBatch, _fvector _vColor) override;
#endif // _DEBUG

};
NS_END