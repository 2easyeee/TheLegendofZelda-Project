#include "Mesh.h"
#include "Bone.h"
#include "Shader.h"
#include "Model.h"

CMesh::CMesh(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CVIBuffer{ _pDevice, _pDeviceContext }
{
}

CMesh::CMesh(const CMesh& _Prototype)
    : CVIBuffer(_Prototype)
    , m_iMaterialIndex {_Prototype.m_iMaterialIndex}
    , m_strMaterialName {_Prototype.m_strMaterialName}
    , m_NonAnimVertices { _Prototype.m_NonAnimVertices }
    , m_SkinnedVertices { _Prototype.m_SkinnedVertices }
    , m_Indices { _Prototype.m_Indices }
{
}

HRESULT CMesh::Initialize_Prototype(const MESH_DESC& _tMeshDesc, _fmatrix _PreTransformMatrix)
{
    /* Material */
    m_strMeshName = _tMeshDesc.meshName;
    m_strMaterialName = _tMeshDesc.materialName;
    
    /* For.Picking */
    if (_tMeshDesc.eModelType == MODEL::SKINNED)
    {
        m_PickingVertices.reserve(_tMeshDesc.skinnedVertices.size());
        for (auto& Vertex : _tMeshDesc.skinnedVertices)
        {
            VERTEXTYPE pick = {};
            pick.vPosition = Vertex.vPosition;
            pick.vNormal = Vertex.vNormal;
            pick.vTexcoord = Vertex.vTexcoord;
            pick.vTangent = Vertex.vTangent;
            pick.vBinormal = Vertex.vBinormal;

            m_PickingVertices.push_back(pick);
        }
    }
    else
    {
        m_PickingVertices = _tMeshDesc.nonAnimVertices;
    }

    /* Vertex Buffer */
    m_iNumVertexBuffers = 1;
    m_eModelType = _tMeshDesc.eModelType;
    if (m_eModelType == MODEL::SKINNED)
    {
        /* SKINNED */
        m_SkinnedVertices = _tMeshDesc.skinnedVertices;
        m_iNumVertices = static_cast<_uint>(m_SkinnedVertices.size());
        Ready_VertexBuffer_Skinned(_tMeshDesc, _PreTransformMatrix);
    }
    else
    {
        /* NONANIM */
        m_NonAnimVertices = _tMeshDesc.nonAnimVertices;
        m_iNumVertices = static_cast<_uint>(m_NonAnimVertices.size());
        Ready_VertexBuffer_NonAnim(_tMeshDesc, _PreTransformMatrix);
    }

    /* Index Buffer */
    m_Indices = _tMeshDesc.commonIndices;
    m_ePrimitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_iNumIndices = static_cast<_uint>(m_Indices.size());
    m_iIndexStride = 4;
    m_eIndexFormat = DXGI_FORMAT_R32_UINT;

    D3D11_BUFFER_DESC IBDesc = {};
    IBDesc.ByteWidth            = m_iIndexStride * m_iNumIndices;
    IBDesc.Usage                = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags            = D3D11_BIND_INDEX_BUFFER;
    IBDesc.CPUAccessFlags       = 0;
    IBDesc.MiscFlags            = 0;
    IBDesc.StructureByteStride  = 0;

    D3D11_SUBRESOURCE_DATA InitialIndexData = {};
    InitialIndexData.pSysMem = m_Indices.data();

    {
#ifdef  _DEBUG MESHINFO
        //_uint maxIndex = 0;

        //for (auto idx : m_Indices)
        //{
        //    if (idx > maxIndex)
        //        maxIndex = idx;
        //}

        //char buf[256];

        //sprintf_s(buf, "MaxIndex: %u\n", maxIndex);
        //OutputDebugStringA(buf);

        //sprintf_s(buf, "==== Mesh Debug ====\n");
        //OutputDebugStringA(buf);

        //sprintf_s(buf, "Mesh Name: %s\n", m_strMeshName.c_str());
        //OutputDebugStringA(buf);

        //sprintf_s(buf, "Material Name: %s\n", m_strMaterialName.c_str());
        //OutputDebugStringA(buf);

        //sprintf_s(buf, "Vertex Count: %u\n", m_iNumVertices);
        //OutputDebugStringA(buf);

        //sprintf_s(buf, "Vertex Stride: %u\n", m_iVertexStride);
        //OutputDebugStringA(buf);

        //sprintf_s(buf, "VertexBuffer Size: %u\n", m_iVertexStride * m_iNumVertices);
        //OutputDebugStringA(buf);

        //sprintf_s(buf, "Index Count: %u\n", m_iNumIndices);
        //OutputDebugStringA(buf);

        //sprintf_s(buf, "IndexBuffer Size: %u\n", m_iIndexStride * m_iNumIndices);
        //OutputDebugStringA(buf);

        //sprintf_s(buf, "====================\n");
        //OutputDebugStringA(buf);
#endif //  _DEBUG
    }

    if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &InitialIndexData, &m_pIB)))
        return E_FAIL;

    return S_OK;
}

HRESULT CMesh::Initialize(void* _pArg)
{
    return S_OK;
}

ID3D11Buffer* CMesh::Get_VertexBuffer() const
{
    return m_pVB;
}

_uint CMesh::Get_VertexStride() const
{
    return m_iVertexStride;
}

_uint CMesh::Get_IndexCount() const
{
    return m_iNumIndices;
}

HRESULT CMesh::Bind_BoneMatrices(CShader* _pShader, const _char* _pConstantName, const vector<class CBone*>& _Bones)
{
    for (size_t i = 0; i < m_iEffectTotalBoneCnt; i++)
    {
        XMStoreFloat4x4(&m_BoneMatrices[i], XMLoadFloat4x4(&m_OffsetMatrices[i]) * _Bones[m_vecEffectBoneIndices[i]]->Get_CombinedTransformationMatrix());
    }
    return _pShader->Bind_Matrices(_pConstantName, m_BoneMatrices, m_iEffectTotalBoneCnt);
}

HRESULT CMesh::Bind_Resources()
{
    UINT stride = m_iVertexStride;
    UINT offset = 0;

    m_pDeviceContext->IASetVertexBuffers(
        0,
        1,
        &m_pVB,
        &stride,
        &offset
    );

    m_pDeviceContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
    m_pDeviceContext->IASetPrimitiveTopology(m_ePrimitive);

    return S_OK;
}

HRESULT CMesh::Ready_VertexBuffer_NonAnim(const MESH_DESC& _tMeshDesc, _fmatrix _PreTransformMatrix)
{
    /* NONANIM */
    m_iVertexStride = sizeof(VTXMESH);

    D3D11_BUFFER_DESC VBDesc = {};
    VBDesc.ByteWidth            = m_iVertexStride * m_iNumVertices;
    VBDesc.Usage                = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags       = 0;
    VBDesc.MiscFlags            = 0;
    VBDesc.StructureByteStride  = m_iVertexStride;

    /* 사전 행렬 변환 */
    for (auto& vertex : m_NonAnimVertices)
    {
        _vector vPos = XMLoadFloat3(&vertex.vPosition);
        vPos = XMVector3TransformCoord(vPos, _PreTransformMatrix);
        XMStoreFloat3(&vertex.vPosition, vPos);

        _vector vNormal = XMLoadFloat3(&vertex.vNormal);
        vNormal = XMVector3TransformNormal(vNormal, _PreTransformMatrix);
        XMStoreFloat3(&vertex.vNormal, vNormal);

        _vector vTangent = XMLoadFloat3(&vertex.vTangent);
        vTangent = XMVector3TransformNormal(vTangent, _PreTransformMatrix);
        vTangent = XMVector3Normalize(vTangent);
        XMStoreFloat3(&vertex.vTangent, vTangent);

        _vector vBinormal = XMLoadFloat3(&vertex.vBinormal);
        vBinormal = XMVector3TransformNormal(vBinormal, _PreTransformMatrix);
        vBinormal = XMVector3Normalize(vBinormal);
        XMStoreFloat3(&vertex.vBinormal, vBinormal);
    }

    D3D11_SUBRESOURCE_DATA InitialVertexData = {};
    InitialVertexData.pSysMem = m_NonAnimVertices.data();

    if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &InitialVertexData, &m_pVB)))
        return E_FAIL;

    return S_OK;
}

HRESULT CMesh::Ready_VertexBuffer_Skinned(const MESH_DESC& _tMeshDesc, _fmatrix _PreTransformMatrix)
{
    /* SKINNED */
    m_iVertexStride = sizeof(VTXANIMMESH);

    D3D11_BUFFER_DESC VBDesc = {};
    VBDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = m_iVertexStride;

    /* 사전 행렬 변환 */
    for (auto& vertex : m_SkinnedVertices)
    {
        _vector vPos = XMLoadFloat3(&vertex.vPosition);
        vPos = XMVector3TransformCoord(vPos, _PreTransformMatrix);
        XMStoreFloat3(&vertex.vPosition, vPos);

        _vector vNormal = XMLoadFloat3(&vertex.vNormal);
        vNormal = XMVector3TransformNormal(vNormal, _PreTransformMatrix);
        XMStoreFloat3(&vertex.vNormal, vNormal);

        _vector vTangent = XMLoadFloat3(&vertex.vTangent);
        vTangent = XMVector3TransformNormal(vTangent, _PreTransformMatrix);
        vTangent = XMVector3Normalize(vTangent);
        XMStoreFloat3(&vertex.vTangent, vTangent);

        _vector vBinormal = XMLoadFloat3(&vertex.vBinormal);
        vBinormal = XMVector3TransformNormal(vBinormal, _PreTransformMatrix);
        vBinormal = XMVector3Normalize(vBinormal);
        XMStoreFloat3(&vertex.vBinormal, vBinormal);
    }

    /* BoneIndex, BoneWeight */
    m_iEffectTotalBoneCnt = static_cast<_int>(_tMeshDesc.effectTotalBoneCnt);
    m_vecEffectBoneIndices.resize(m_iEffectTotalBoneCnt);
    m_OffsetMatrices.resize(m_iEffectTotalBoneCnt);

    for (_int i = 0; i < m_iEffectTotalBoneCnt; ++i)
    {
        m_vecEffectBoneIndices[i] = _tMeshDesc.effectBoneIndices[i];
        m_OffsetMatrices[i] = _tMeshDesc.offsetMatrices[i];
    }

    D3D11_SUBRESOURCE_DATA InitialVertexData = {};
    InitialVertexData.pSysMem = m_SkinnedVertices.data();

    //if (!m_SkinnedVertices.empty())
    //{
    //    auto& v = m_SkinnedVertices[0];

    //    char buf[256];
    //    sprintf_s(buf,
    //        "BlendIndex: %u %u %u %u\nBlendWeight: %f %f %f %f\n",
    //        v.vBlendIndex.x,
    //        v.vBlendIndex.y,
    //        v.vBlendIndex.z,
    //        v.vBlendIndex.w,
    //        v.vBlendWeight.x,
    //        v.vBlendWeight.y,
    //        v.vBlendWeight.z,
    //        v.vBlendWeight.w);

    //    OutputDebugStringA(buf);
    //}

    if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &InitialVertexData, &m_pVB)))
        return E_FAIL;

    return S_OK;
}

CMesh* CMesh::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const MESH_DESC& _tMeshDesc, _fmatrix _PreTransformMatrix)
{
    CMesh* pInstance = new CMesh(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype(_tMeshDesc,_PreTransformMatrix)))
    {
        MSG_BOX("FAILED TO CREATED : CMesh");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CMesh::Clone(void* _pArg)
{
    CMesh* pInstance = new CMesh(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CMesh");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMesh::Free()
{
    __super::Free();
}
