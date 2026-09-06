#pragma once
#include "Bounding.h"

NS_BEGIN(Engine)
class CBounding_Sphere final : public CBounding
{
public:
	typedef struct tagSphereDesc final : public CBounding::BOUNDING_DESC
	{
		_float fRadius;
	}SPHERE_DESC;

private:
	CBounding_Sphere();
	virtual ~CBounding_Sphere() = default;

public:
	virtual HRESULT Initialize(const BOUNDING_DESC* _pDesc) override;
	virtual void Update(_fmatrix _WorldMatrix) override;
	virtual _bool Intersect(COLLIDER _eType, CBounding* _pTarget) override;

	const BoundingSphere* Get_Desc() const { return m_pDesc; }

private:
	BoundingSphere* m_pOriginalDesc = {};
	BoundingSphere* m_pDesc = {};

public:
	static CBounding_Sphere* Create(const BOUNDING_DESC* _pDesc);
	virtual void Free() override;

#ifdef _DEBUG
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* _pBatch, _fvector _vColor) override;
#endif // _DEBUG

};
NS_END