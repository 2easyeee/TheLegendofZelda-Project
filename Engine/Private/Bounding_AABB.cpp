#include "Bounding_AABB.h"
#include "Debug/DebugDraw.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"

CBounding_AABB::CBounding_AABB()
{
}

HRESULT CBounding_AABB::Initialize(const BOUNDING_DESC* _pDesc)
{
    const AABB_DESC* pAABBDesc = static_cast<const AABB_DESC*>(_pDesc);

    m_pOriginalDesc = new BoundingBox(pAABBDesc->vCenter, pAABBDesc->vExtents);
    m_pDesc = new BoundingBox(*m_pOriginalDesc);

    return S_OK;
}

void CBounding_AABB::Update(_fmatrix _WorldMatrix)
{
    _matrix TransformMatrix = _WorldMatrix;

    TransformMatrix.r[0] = XMVectorSet(1.f, 0.f, 0.f, 0.f) * XMVector3Length(TransformMatrix.r[0]);
    TransformMatrix.r[1] = XMVectorSet(0.f, 1.f, 0.f, 0.f) * XMVector3Length(TransformMatrix.r[1]);
    TransformMatrix.r[2] = XMVectorSet(0.f, 0.f, 1.f, 0.f) * XMVector3Length(TransformMatrix.r[2]);

    m_pOriginalDesc->Transform(*m_pDesc, TransformMatrix);
}

_bool CBounding_AABB::Intersect(COLLIDER _eType, CBounding* _pTarget)
{
    _bool isCollision = { false };

    switch (_eType)
    {
    case Engine::COLLIDER::AABB:
        isCollision = m_pDesc->Intersects(
            *dynamic_cast<CBounding_AABB*>(_pTarget)->Get_Desc());
        break;
    case Engine::COLLIDER::OBB:
        isCollision = m_pDesc->Intersects(
            *dynamic_cast<CBounding_OBB*>(_pTarget)->Get_Desc());
        break;
    case Engine::COLLIDER::SPHERE:
        isCollision = m_pDesc->Intersects(
            *dynamic_cast<CBounding_Sphere*>(_pTarget)->Get_Desc());
        break;
    }
    return isCollision;
}

void CBounding_AABB::Set_Extents(_float3 _vExtents)
{   
    if (!m_pOriginalDesc)
        return;

    m_pOriginalDesc->Extents = _vExtents;
}

void CBounding_AABB::Set_Center(_float3 _vCenter)
{
    if (!m_pOriginalDesc)
        return;

    m_pOriginalDesc->Center = _vCenter;
}

CBounding_AABB* CBounding_AABB::Create(const BOUNDING_DESC* _pDesc)
{
    CBounding_AABB* pInstance = new CBounding_AABB();
    if (FAILED(pInstance->Initialize(_pDesc)))
    {
        MSG_BOX("FAILED TO CREATED : CBounding_AABB");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBounding_AABB::Free()
{
    __super::Free();

    Safe_Delete(m_pDesc);
    Safe_Delete(m_pOriginalDesc);
}

#ifdef _DEBUG
HRESULT CBounding_AABB::Render(PrimitiveBatch<VertexPositionColor>* _pBatch, _fvector _vColor)
{
    DX::Draw(_pBatch, *m_pDesc, _vColor);

    return S_OK;
}
#endif // _DEBUG

