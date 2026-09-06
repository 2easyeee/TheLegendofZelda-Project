#include "Bounding_Sphere.h"
#include "Debug/DebugDraw.h"
#include "Bounding_AABB.h"
#include "Bounding_OBB.h"

CBounding_Sphere::CBounding_Sphere()
{
}

HRESULT CBounding_Sphere::Initialize(const BOUNDING_DESC* _pDesc)
{
    const SPHERE_DESC* pSphereDesc = static_cast<const SPHERE_DESC*>(_pDesc);

    m_pOriginalDesc = new BoundingSphere(pSphereDesc->vCenter, pSphereDesc->fRadius);
    m_pDesc = new BoundingSphere(*m_pOriginalDesc);

    return S_OK;
}

void CBounding_Sphere::Update(_fmatrix _WorldMatrix)
{
    m_pOriginalDesc->Transform(*m_pDesc, _WorldMatrix);
}

_bool CBounding_Sphere::Intersect(COLLIDER _eType, CBounding* _pTarget)
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

CBounding_Sphere* CBounding_Sphere::Create(const BOUNDING_DESC* _pDesc)
{
    CBounding_Sphere* pInstance = new CBounding_Sphere();
    if (FAILED(pInstance->Initialize(_pDesc)))
    {
        MSG_BOX("FAILED TO CREATED : CBounding_Sphere");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBounding_Sphere::Free()
{
    __super::Free();

    Safe_Delete(m_pDesc);
    Safe_Delete(m_pOriginalDesc);
}

#ifdef _DEBUG
HRESULT CBounding_Sphere::Render(PrimitiveBatch<VertexPositionColor>* _pBatch, _fvector _vColor)
{
    DX::Draw(_pBatch, *m_pDesc, _vColor);

    return S_OK;
}
#endif