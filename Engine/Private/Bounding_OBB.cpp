#include "Bounding_OBB.h"
#include "Debug/DebugDraw.h"
#include "Bounding_AABB.h"
#include "Bounding_Sphere.h"

CBounding_OBB::CBounding_OBB()
{
}

HRESULT CBounding_OBB::Initialize(const BOUNDING_DESC* _pDesc)
{
    const OBB_DESC* pOBBDesc = static_cast<const OBB_DESC*>(_pDesc);

    _float4 vQuaternion = {};
    XMStoreFloat4(&vQuaternion,
        XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pOBBDesc->vAngles.x),
            XMConvertToRadians(pOBBDesc->vAngles.y),
            XMConvertToRadians(pOBBDesc->vAngles.z)));

    m_pOriginalDesc = new BoundingOrientedBox(pOBBDesc->vCenter, pOBBDesc->vExtents, vQuaternion);
    m_pDesc = new BoundingOrientedBox(*m_pOriginalDesc);

    return S_OK;
}

void CBounding_OBB::Update(_fmatrix _WorldMatrix)
{
    m_pOriginalDesc->Transform(*m_pDesc, _WorldMatrix);
}

_bool CBounding_OBB::Intersect(COLLIDER _eType, CBounding* _pTarget)
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

CBounding_OBB* CBounding_OBB::Create(const BOUNDING_DESC* _pDesc)
{
    CBounding_OBB* pInstance = new CBounding_OBB();
    if (FAILED(pInstance->Initialize(_pDesc)))
    {
        MSG_BOX("FAILED TO CREATED : CBounding_OBB");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBounding_OBB::Free()
{
    __super::Free();

    Safe_Delete(m_pDesc);
    Safe_Delete(m_pOriginalDesc);
}

#ifdef _DEBUG
HRESULT CBounding_OBB::Render(PrimitiveBatch<VertexPositionColor>* _pBatch, _fvector _vColor)
{
    DX::Draw(_pBatch, *m_pDesc, _vColor);

    return S_OK;
}
#endif