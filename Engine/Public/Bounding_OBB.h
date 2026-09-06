#pragma once
#include "Bounding.h"

NS_BEGIN(Engine)
class CBounding_OBB final : public CBounding
{
public:
	typedef struct tagOBBDesc final : public CBounding::BOUNDING_DESC
	{
		_float3 vExtents;
		_float3 vAngles;
	}OBB_DESC;

private:
	CBounding_OBB();
	virtual ~CBounding_OBB() = default;

public:
	virtual HRESULT Initialize(const BOUNDING_DESC* _pDesc) override;
	virtual void Update(_fmatrix _WorldMatrix) override;
	virtual _bool Intersect(COLLIDER _eType, CBounding* _pTarget) override;

	const BoundingOrientedBox* Get_Desc() const { return m_pDesc; }

private:
	BoundingOrientedBox* m_pOriginalDesc = {};
	BoundingOrientedBox* m_pDesc = {};

public:
	static CBounding_OBB* Create(const BOUNDING_DESC* _pDesc);
	virtual void Free() override;

#ifdef _DEBUG
public:
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* _pBatch, _fvector _vColor) override;
#endif // _DEBUG

};
NS_END