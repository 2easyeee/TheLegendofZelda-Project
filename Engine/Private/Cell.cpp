#include "Cell.h"

#ifdef _DEBUG
#include "VIBuffer_Cell.h"
#endif // _DEBUG

CCell::CCell(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : m_pDevice { _pDevice }
    , m_pDeviceContext { _pDeviceContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pDeviceContext);
}

HRESULT CCell::Initialize(const _float3* _pPoints, _int _iIndex)
{
    /* 1. 삼각형의 A,B,C 좌표 저장*/
    m_iIndex = _iIndex;
    memcpy(m_vPoints, _pPoints, sizeof(_float3) * POINT::POINT_END);

    /* 2. 각 변의 법선 벡터 저장 */
    _vector vLine = {};
    vLine = XMLoadFloat3(&m_vPoints[POINT::B]) - XMLoadFloat3(&m_vPoints[POINT::A]);
    m_vNormals[LINE::AB] = _float3(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));
    vLine = XMLoadFloat3(&m_vPoints[POINT::C]) - XMLoadFloat3(&m_vPoints[POINT::B]);
    m_vNormals[LINE::BC] = _float3(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));
    vLine = XMLoadFloat3(&m_vPoints[POINT::A]) - XMLoadFloat3(&m_vPoints[POINT::C]);
    m_vNormals[LINE::CA] = _float3(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));

    for (size_t i = 0; i < LINE::LINE_END; i++)
    {
        XMStoreFloat3(&m_vNormals[i], XMVector3Normalize(XMLoadFloat3(&m_vNormals[i])));
    }

    /* 3. 평면 방정식 생성 (ax + by + cz + d = 0) */
    XMStoreFloat4(&m_Plane, XMPlaneFromPoints(
        XMLoadFloat3(&m_vPoints[POINT::A]),
        XMLoadFloat3(&m_vPoints[POINT::B]),
        XMLoadFloat3(&m_vPoints[POINT::C])));

#ifdef _DEBUG
    m_pVIBuffer = CVIBuffer_Cell::Create(m_pDevice, m_pDeviceContext, m_vPoints);
    if (nullptr == m_pVIBuffer)
        return E_FAIL;
#endif // _DEBUG

    return S_OK;
}

_bool CCell::IsIn(_fvector _vPosition, _int* _pNeighborIndex)
{
    /* 이 셀 내부에 있는 가 ? */
    for (size_t i = 0; i < LINE::LINE_END; i++)
    {
        /* 1. 현재 위치에서 각 변의 시작점으로 향하는 방향 벡터 계산 */
        _vector vDir = XMVector3Normalize(_vPosition - XMLoadFloat3(&m_vPoints[i]));
        /* 2. 그 방향과 변의 법선 벡터 내적 */
        if (0 < XMVectorGetX(XMVector3Dot(XMLoadFloat3(&m_vNormals[i]), vDir)))
        {
            /* 내적이 양수 일 때 == 바깥에 있을 때 */
            /* 그 변과 연결된 이웃 셀 인덱스를 알려준다. */
            *_pNeighborIndex = m_iNeighborIndices[i];
            return false;
        }
    }
    return true;
}

_bool CCell::Compare_Points(_fvector _vSourPoint, _fvector _vDestPoint)
{
    /* 두 점이 같은 변인가 ? == 두 셀이 이웃인가 ? */
    /* A */
    if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT::A]), _vSourPoint))
    {
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT::B]), _vDestPoint))
            return true;
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT::C]), _vDestPoint))
            return true;
    }

    /* B */
    if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT::B]), _vSourPoint))
    {
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT::C]), _vDestPoint))
            return true;
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT::A]), _vDestPoint))
            return true;
    }

    /* C */
    if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT::C]), _vSourPoint))
    {
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT::A]), _vDestPoint))
            return true;
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT::B]), _vDestPoint))
            return true;
    }

    return false;
}

_float CCell::Compute_Height(_fvector _vPosition)
{
    /* 평면 방정식 (y = -ax - cz - d / b) */
    return ((-m_Plane.x * XMVectorGetX(_vPosition)) - (m_Plane.z * XMVectorGetZ(_vPosition)) - m_Plane.w) / (m_Plane.y);
}

_vector CCell::Get_Point(POINT _ePoint)
{
    return XMLoadFloat3(&m_vPoints[_ePoint]);
}

void CCell::Set_Point(POINT _ePoint, _fvector _vPoint)
{
    XMStoreFloat3(&m_vPoints[_ePoint], _vPoint);

    /* 2. 각 변의 법선 벡터 저장 */
    _vector vLine = {};
    vLine = XMLoadFloat3(&m_vPoints[POINT::B]) - XMLoadFloat3(&m_vPoints[POINT::A]);
    m_vNormals[LINE::AB] = _float3(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));
    vLine = XMLoadFloat3(&m_vPoints[POINT::C]) - XMLoadFloat3(&m_vPoints[POINT::B]);
    m_vNormals[LINE::BC] = _float3(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));
    vLine = XMLoadFloat3(&m_vPoints[POINT::A]) - XMLoadFloat3(&m_vPoints[POINT::C]);
    m_vNormals[LINE::CA] = _float3(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));

    for (size_t i = 0; i < LINE::LINE_END; i++)
    {
        XMStoreFloat3(&m_vNormals[i], XMVector3Normalize(XMLoadFloat3(&m_vNormals[i])));
    }

    /* 3. 평면 방정식 생성 (ax + by + cz + d = 0) */
    XMStoreFloat4(&m_Plane, XMPlaneFromPoints(
        XMLoadFloat3(&m_vPoints[POINT::A]),
        XMLoadFloat3(&m_vPoints[POINT::B]),
        XMLoadFloat3(&m_vPoints[POINT::C])));

#ifdef _DEBUG
    // refactor
    Safe_Release(m_pVIBuffer);
    m_pVIBuffer = CVIBuffer_Cell::Create(
        m_pDevice,
        m_pDeviceContext,
        m_vPoints);
#endif
}

void CCell::Set_Neighbor(LINE _eLine, CCell* _pNeighbor)
{
    m_iNeighborIndices[_eLine] = _pNeighbor->m_iIndex;
}

void CCell::Set_Neighbor(const _int* _pNeighbors)
{
    memcpy(m_iNeighborIndices, _pNeighbors, sizeof(_int) * 3);
}

_int* CCell::Get_NeighborIndices()
{
    return m_iNeighborIndices;
}

_bool CCell::IsSamePoint(_fvector _vA, _fvector _vB)
{
    const float EPS = 0.01f;

    /* Y 비교 제거 */
    _vector fOffset = _vA - _vB;
    _float dX = XMVectorGetX(fOffset);
    _float dZ = XMVectorGetZ(fOffset);

    return (dX * dX + dZ * dZ) < EPS * EPS;
}

_bool CCell::IsSameEdge(POINT _a0, POINT _a1, CCell* _pOther)
{
    _vector p0 = Get_Point(_a0);
    _vector p1 = Get_Point(_a1);

    _int iShared = 0;

    for (_int i = 0; i < 3; ++i)
    {
        _vector other = _pOther->Get_Point((POINT)i);
        if (IsSamePoint(p0, other) || IsSamePoint(p1, other))
            iShared++;
    }
    return iShared == 2;
}

_int CCell::Get_SharedPointCount(CCell* _pOther)
{
    if (!_pOther)
        return 0;

    _int iShared = 0;

    for (_int i = 0; i < POINT_END; ++i)
    {
        _vector vA = XMLoadFloat3(&m_vPoints[i]);
        for (_int j = 0; j < POINT::POINT_END; ++j)
        {
            _vector vB = XMLoadFloat3(&_pOther->m_vPoints[j]);
            
            if (IsSamePoint(vA, vB))
            {
                iShared++;
                break;
            }
        }
    }
    return iShared;
}

void CCell::Link_Neighbor(CCell* _pOther)
{
    if (!_pOther)
        return;

    for (_int i = 0; i < 3; ++i)
    {
        if (m_iNeighborIndices[i] != -1)
            continue;
        
        POINT p0 = (POINT)i;
        POINT p1 = (POINT)((i + 1) % 3);

        if (IsSameEdge(p0, p1, _pOther))
        {
            m_iNeighborIndices[i] = _pOther->Get_Index();
            return;
        }
    }
}

void CCell::Set_Index(_int _iIndex)
{
    m_iIndex = _iIndex;
}

_int CCell::Get_Index() const
{
    return m_iIndex;
}

CCell* CCell::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _float3* _pPoints, _int _iIndex)
{
    CCell* pInstance = new CCell(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize(_pPoints, _iIndex)))
    {
        MSG_BOX("FAILED TO CREATED : CCell");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCell::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pDeviceContext);

#ifdef _DEBUG
    Safe_Release(m_pVIBuffer);
#endif // _DEBUG

}

#ifdef _DEBUG
HRESULT CCell::Render(D3D11_PRIMITIVE_TOPOLOGY _eType, _uint _iNumIndices)
{
    m_pVIBuffer->Bind_Resources(_eType);
    m_pVIBuffer->Render(_iNumIndices);


    return S_OK;
}
#endif // _DEBUG
