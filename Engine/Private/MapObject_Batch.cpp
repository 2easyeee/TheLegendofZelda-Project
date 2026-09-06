#include "MapObject_Batch.h"
#include "GameInstance.h"

CMapObject_Batch::CMapObject_Batch(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CGameObject{ _pDevice, _pDeviceContext }
{
}

CMapObject_Batch::CMapObject_Batch(const CMapObject_Batch& _Prototype)
    : CGameObject (_Prototype)
{
}

HRESULT CMapObject_Batch::Initialize_Prototype()
{
    if (FAILED(CGameObject::Initialize(nullptr)))
        return E_FAIL;

    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Shader_VtxMesh"),
        TEXT("Com_Shader"),
        reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    return S_OK;
}

HRESULT CMapObject_Batch::Initialize(void* _pArg)
{
    return S_OK;
}

void CMapObject_Batch::Priority_Update(_float _fTimeDelta)
{
}

void CMapObject_Batch::Update(_float _fTimeDelta)
{
}

void CMapObject_Batch::Late_Update(_float _fTimeDelta)
{
    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CMapObject_Batch::Render()
{
    if (!m_pVB || !m_pIB)
        return S_OK;

    UINT stride = sizeof(BATCH_VERTEX);
    UINT offset = 0;

    m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVB, &stride, &offset);
    m_pDeviceContext->IASetIndexBuffer(m_pIB, DXGI_FORMAT_R32_UINT, 0);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    /* Transform */
    if (FAILED(m_pTransformCom->Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    /* Perspective */
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Begin(0)))
        return E_FAIL;

    m_pDeviceContext->DrawIndexed(m_iIndexCount, 0, 0);

    return S_OK;
}

void CMapObject_Batch::Add_StaticInstance(CModel* pModel, _matrix worldMatrix)
{
    auto meshes = pModel->Get_Meshes();

    for (auto* pMesh : meshes)
    {
        const auto& vertices = pMesh->Get_Vertices();
        const auto& indices = pMesh->Get_Indices();

        _uint baseIndex = (_uint)m_MergedVertices.size();

        for (auto& v : vertices)
        {
            BATCH_VERTEX newV;

            _vector pos = XMVector3TransformCoord(
                XMLoadFloat3(&v.vPosition),
                worldMatrix);

            XMStoreFloat3(&newV.vPosition, pos);
            newV.vNormal = v.vNormal;
            newV.vTexcoord = v.vTexcoord;

            m_MergedVertices.push_back(newV);
        }

        for (auto idx : indices)
            m_MergedIndices.push_back(idx + baseIndex);
    }
}

HRESULT CMapObject_Batch::Build_Buffers()
{
    if (m_MergedVertices.empty())
        return E_FAIL;

    m_iIndexCount = (_uint)m_MergedIndices.size();

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = UINT(sizeof(BATCH_VERTEX) * m_MergedVertices.size());
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = m_MergedVertices.data();

    m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_pVB);

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = UINT(sizeof(_uint) * m_MergedIndices.size());
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = m_MergedIndices.data();

    m_pDevice->CreateBuffer(&ibDesc, &ibData, &m_pIB);

    return S_OK;
}

CMapObject_Batch* CMapObject_Batch::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CMapObject_Batch* pInstance = new CMapObject_Batch(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CMapObject_Batch");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CMapObject_Batch::Clone(void* _pArg)
{
    return nullptr;
}

void CMapObject_Batch::Free()
{
    __super::Free();

    Safe_Release(m_pVB);
    Safe_Release(m_pIB);
    Safe_Release(m_pShaderCom);
}
