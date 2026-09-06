#include "Inspector_Mesh.h"
#include "AssimpEditor.h"

CInspector_Mesh::CInspector_Mesh(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CAssimpEditor* _pEditor)
    : CImGui_Object{ _pDevice, _pDeviceContext }
    , m_pAssimpEditor{ _pEditor }
{
}

HRESULT CInspector_Mesh::Initialize()
{
    m_strTitle = TEXT("Inspector(Mesh)");

    return S_OK;
}

void CInspector_Mesh::Update(_float fTimeDelta)
{
    m_paiNode = m_pAssimpEditor->Get_SelectedNode();
    m_paiScene = m_pAssimpEditor->Get_aiScene();

    m_iMeshIndex = m_pAssimpEditor->Get_SelectedMeshIndex();
    if (m_iMeshIndex < 0)
        return;
    m_pMesh = m_paiScene->mMeshes[m_iMeshIndex];
}

void CInspector_Mesh::LateUpdate(_float fTimeDelta)
{
}

HRESULT CInspector_Mesh::Render()
{
    if (!m_pAssimpEditor)
        return E_FAIL;

    ImGui::Begin("Inspector(Mesh)");
    Render_Node();
    ImGui::End();

    return S_OK;
}

void CInspector_Mesh::Render_Node()
{
    if (!m_paiNode)
    {
        ImGui::TextDisabled("No Node");
        return;
    }

    ImGui::SeparatorText("Node Name");
    ImGui::Text("%s", m_paiNode->mName.C_Str());

    ImGui::SeparatorText("Count (Mesh/Children)");
    ImGui::Text("Meshes   : %d", m_paiNode->mNumMeshes);
    ImGui::Text("Children : %d", m_paiNode->mNumChildren);

    if (m_pMesh && m_iMeshIndex >= 0)
    {
        ImGui::SeparatorText("VIBuffer");

        ImGui::Text("Mesh[%d]", m_iMeshIndex);
        ImGui::Text("Vertices : %u", m_pMesh->mNumVertices);
        ImGui::Text("Faces    : %u", m_pMesh->mNumFaces);
        ImGui::Text("Indices  : %u", m_pMesh->mNumFaces * 3);
        ImGui::Text("Primitive: Triangles");
    }

    /* Bound */
    if (m_pMesh && m_iMeshIndex >= 0)
    {
        ImGui::SeparatorText("Bounds");

        aiVector3D min, max;
        min = max = m_pMesh->mVertices[0];

        for (unsigned int i = 1; i < m_pMesh->mNumVertices; ++i)
        {
            min.x = std::min(min.x, m_pMesh->mVertices[i].x);
            min.y = std::min(min.y, m_pMesh->mVertices[i].y);
            min.z = std::min(min.z, m_pMesh->mVertices[i].z);

            max.x = max(max.x, m_pMesh->mVertices[i].x);
            max.y = max(max.y, m_pMesh->mVertices[i].y);
            max.z = max(max.z, m_pMesh->mVertices[i].z);
        }

        ImGui::Text("AABB Min : %.2f %.2f %.2f", min.x, min.y, min.z);
        ImGui::Text("AABB Max : %.2f %.2f %.2f", max.x, max.y, max.z);
    }

    /* BONE */
    if (m_pMesh && m_iMeshIndex >= 0)
    {
        ImGui::SeparatorText("NONANIM / SKINNED");
        _bool bIsHasBones = m_pMesh->HasBones();
        ImGui::Text("HasBone : %s", bIsHasBones ? "YES" : "NO");
        ImGui::Text("aiMesh->mNumBones : %u", m_pMesh->mNumBones);
    }

    /* Check Limit Weight */

    /* Weight */

    /* Scale */
    /* Scale (Node Transform) */
    if (m_paiNode)
    {
        ImGui::SeparatorText("Scale");

        aiVector3D scale, pos;
        aiQuaternion rot;
        m_paiNode->mTransformation.Decompose(scale, rot, pos);

        ImGui::Text("Node Name : %s", m_paiNode->mName.C_Str());
        ImGui::Text("Scale     : %.4f %.4f %.4f", scale.x, scale.y, scale.z);
    }
}

CInspector_Mesh* CInspector_Mesh::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CAssimpEditor* _pEditor)
{
    CInspector_Mesh* pInstance = new CInspector_Mesh(_pDevice, _pDeviceContext, _pEditor);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CInspector_Mesh");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CInspector_Mesh::Free()
{
    __super::Free();
}
