#include "VIBuffer_Trail.h"
#include "GameInstance.h"

CVIBuffer_Trail::CVIBuffer_Trail(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CVIBuffer{ _pDevice, _pDeviceContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

CVIBuffer_Trail::CVIBuffer_Trail(const CVIBuffer_Trail& Prototype)
    : CVIBuffer(Prototype)
    , m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CVIBuffer_Trail::Initialize_Prototype()
{
    m_iMaxPoint = 128; // 최대 trail point
    //m_iMaxPoint = 256; // 최대 trail point

    m_iNumVertexBuffers = 1;
    m_iVertexStride = sizeof(VTXTRAIL);
    m_iNumVertices = m_iMaxPoint * 2;

    m_iNumIndices = (m_iMaxPoint - 1) * 6;
    m_iIndexStride = sizeof(_ushort);
    m_eIndexFormat = DXGI_FORMAT_R16_UINT;
    m_ePrimitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    m_Vertices.resize(m_iNumVertices);
    m_Indices.resize(m_iNumIndices);

    /* Index 생성 */
    _uint idx = 0;
    for (_uint i = 0; i < m_iMaxPoint - 1; ++i)
    {
        _ushort base = i * 2;

        m_Indices[idx++] = base + 0;
        m_Indices[idx++] = base + 1;
        m_Indices[idx++] = base + 2;

        m_Indices[idx++] = base + 1;
        m_Indices[idx++] = base + 3;
        m_Indices[idx++] = base + 2;
    }

    /* Vertex Buffer (Dynamic) */
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
    vbDesc.Usage = D3D11_USAGE_DYNAMIC;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(m_pDevice->CreateBuffer(&vbDesc, nullptr, &m_pVB)))
        return E_FAIL;

    /* Index Buffer */
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = m_Indices.data();

    if (FAILED(m_pDevice->CreateBuffer(&ibDesc, &ibData, &m_pIB)))
        return E_FAIL;

    return S_OK;
}

HRESULT CVIBuffer_Trail::Initialize(void* _pArg)
{
    return S_OK;
}

HRESULT CVIBuffer_Trail::Render()
{
    wchar_t buffer[128];
    swprintf_s(buffer, L"IndexCount: %d\n", m_iCurrentIndexCount);
    OutputDebugString(buffer);

    if (m_iCurrentIndexCount == 0)
        return S_OK;

    m_pDeviceContext->DrawIndexed(m_iCurrentIndexCount, 0, 0);

    return S_OK;
}

void CVIBuffer_Trail::Update_Trail(const vector<_vector>& _tips, const vector<_vector>& _bases)
{
    if (_tips.size() < 2 || _tips.size() != _bases.size())
    {
        m_iCurrentIndexCount = 0;
        return;
    }

    vector<_vector> smoothedTips;
    vector<_vector> smoothedBases;
    int segments = 4;

    for (_uint i = 0; i < (_uint)_tips.size(); ++i)
    {
        _uint i0 = max((int)i - 1, 0);
        _uint i1 = i;
        _uint i2 = min(i + 1, (int)_tips.size() - 1);
        _uint i3 = min(i + 2, (int)_tips.size() - 1);

        for (int s = 0; s < segments; ++s)
        {
            float t = (float)s / segments;
            smoothedTips.push_back(CatmullRom(_tips[i0], _tips[i1], _tips[i2], _tips[i3], t));
            smoothedBases.push_back(CatmullRom(_bases[i0], _bases[i1], _bases[i2], _bases[i3], t));
        }
    }
    smoothedTips.push_back(_tips.back());
    smoothedBases.push_back(_bases.back());

    _uint iCount = min((_uint)smoothedTips.size(), m_iMaxPoint);

    if (iCount < 2)
    {
        m_iCurrentIndexCount = 0;
        return;
    }

    vector<float> lengths(iCount);
    lengths[0] = 0.f;
    float totalLength = 0.f;

    for (_uint i = 1; i < iCount; ++i)
    {
        float dist = XMVectorGetX(XMVector3Length(smoothedTips[i] - smoothedTips[i - 1]));
        totalLength += dist;
        lengths[i] = totalLength;
    }

    D3D11_MAPPED_SUBRESOURCE mapped{};
    m_pDeviceContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    VTXTRAIL* pVertices = reinterpret_cast<VTXTRAIL*>(mapped.pData);
    memset(pVertices, 0, sizeof(VTXTRAIL) * m_iNumVertices);

    for (_uint i = 0; i < iCount; ++i)
    {
        float tLife = (float)i / (iCount - 1);
        float v = (totalLength > 0.f) ? (lengths[i] / totalLength) : 0.f;

        XMStoreFloat3(&pVertices[i * 2 + 0].vPosition, smoothedTips[i]);
        pVertices[i * 2 + 0].vTexcoord = { 0.f, v };
        pVertices[i * 2 + 0].fLife = tLife;

        XMStoreFloat3(&pVertices[i * 2 + 1].vPosition, smoothedBases[i]);
        pVertices[i * 2 + 1].vTexcoord = { 1.f, v };
        pVertices[i * 2 + 1].fLife = tLife;
    }

    m_pDeviceContext->Unmap(m_pVB, 0);

    m_iCurrentIndexCount = (iCount - 1) * 6;
}

void CVIBuffer_Trail::Set_MaxPoint(_uint _iMaxPoint)
{
    m_iMaxPoint = _iMaxPoint;
}

_vector CVIBuffer_Trail::CatmullRom(_vector p0, _vector p1, _vector p2, _vector p3, float t)
{
    return XMVectorCatmullRom(p0, p1, p2, p3, t);
}

CVIBuffer_Trail* CVIBuffer_Trail::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CVIBuffer_Trail* pInstance = new CVIBuffer_Trail(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CVIBuffer_Trail");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CVIBuffer_Trail::Clone(void* _pArg)
{
    CVIBuffer_Trail* pInstnace = new CVIBuffer_Trail(*this);
    if (FAILED(pInstnace->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CVIBuffer_Trail");
        Safe_Release(pInstnace);
    }
    return pInstnace;
}

void CVIBuffer_Trail::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
