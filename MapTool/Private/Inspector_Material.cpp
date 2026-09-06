#include "Inspector_Material.h"
#include "AssimpEditor.h"
#include "Texture_Preview.h"

CInspector_Material::CInspector_Material(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CAssimpEditor* _pEditor)
    : CImGui_Object{ _pDevice, _pDeviceContext }
    , m_pAssimpEditor{ _pEditor }
{
}

HRESULT CInspector_Material::Initialize()
{
    m_strTitle = TEXT("Inspector(Material)");

    return S_OK;
}

void CInspector_Material::Update(_float fTimeDelta)
{
    m_paiNode = m_pAssimpEditor->Get_SelectedNode();
    m_paiScene = m_pAssimpEditor->Get_aiScene();

    m_iMeshIndex = m_pAssimpEditor->Get_SelectedMeshIndex();
    if (m_iMeshIndex < 0)
        return;
    m_pMesh = m_paiScene->mMeshes[m_iMeshIndex];

    m_iMaterialIndex = m_pMesh->mMaterialIndex;
    m_pMaterial = m_paiScene->mMaterials[m_iMaterialIndex];
}

void CInspector_Material::LateUpdate(_float fTimeDelta)
{
}

HRESULT CInspector_Material::Render()
{
    if (!m_pAssimpEditor)
        return E_FAIL;

    ImGui::Begin("Inspector(Material)");
    Render_Node();
    ImGui::End();

    return S_OK;
}

void CInspector_Material::Render_Node()
{
    if (!m_pMaterial)
    {
        ImGui::TextDisabled("No Material");
        return;
    }
    ImGui::Text("Material Index : %d", m_pMesh->mMaterialIndex);
    aiString name;
    if (AI_SUCCESS == m_pMaterial->Get(AI_MATKEY_NAME, name))
        ImGui::Text("Material Name  : %s", name.C_Str());
    ImGui::Separator();

    /* Texture */
    ImGui::SeparatorText("Textures");
    Render_Texture(aiTextureType_DIFFUSE, "Diffuse");
    Render_Texture(aiTextureType_NORMALS, "Normal");
    Render_Texture(aiTextureType_SPECULAR, "Specular");
    Render_Texture(aiTextureType_EMISSIVE, "Emissive");

    /* Color */
    ImGui::SeparatorText("Colors");
    aiColor4D color;
    if (AI_SUCCESS == m_pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color))
        ImGui::Text("Ambient  : %.2f %.2f %.2f %.2f", color.r, color.g, color.b, color.a);
    else
        ImGui::TextDisabled("Ambient  : None");

    if (AI_SUCCESS == m_pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color))
        ImGui::Text("Diffuse  : %.2f %.2f %.2f %.2f", color.r, color.g, color.b, color.a);
    else
        ImGui::TextDisabled("Diffuse  : None");

    if (AI_SUCCESS == m_pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color))
        ImGui::Text("Specular : %.2f %.2f %.2f %.2f", color.r, color.g, color.b, color.a);
    else
        ImGui::TextDisabled("Specular : None");

    if (AI_SUCCESS == m_pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color))
        ImGui::Text("Emissive : %.2f %.2f %.2f %.2f", color.r, color.g, color.b, color.a);
    else
        ImGui::TextDisabled("Ambient  : None");

    /* Shininess */
    ImGui::SeparatorText("Shading");
    float shininess = 0.f;
    if (AI_SUCCESS == m_pMaterial->Get(AI_MATKEY_SHININESS, shininess))
        ImGui::Text("Shininess : %.2f", shininess);
    else
        ImGui::TextDisabled("Shininess : None");

}

void CInspector_Material::Render_Texture(aiTextureType _aiTextureType, const char* _label)
{
    _uint iCount = m_pMaterial->GetTextureCount(_aiTextureType);

    if (iCount == 0)
        return;

    filesystem::path fbxPath(m_pAssimpEditor->Get_TextureFullPath());
    filesystem::path fbxDir = fbxPath.parent_path();

    CTexture_Preview* pPreview = m_pAssimpEditor->Get_TexturePreview();

    for (_uint i = 0; i < iCount; ++i)
    {
        aiString path;
        if (AI_SUCCESS != m_pMaterial->GetTexture(_aiTextureType, i, &path))
            continue;

        ImGui::Text("%s / %u / %s", _label, iCount, path.C_Str());

        /* Image */
        if (!pPreview)
            continue;

        filesystem::path texPath = fbxDir / path.C_Str();
        filesystem::path ddsPath = texPath;
        ddsPath.replace_extension(L".dds");

        filesystem::path finalPath;
        if (filesystem::exists(ddsPath))
            finalPath = ddsPath;
        else if (filesystem::exists(texPath))
            finalPath = texPath;
        else
            continue;

        ID3D11ShaderResourceView* pSRV =
            pPreview->Get_SRV(finalPath.string());

        if (pSRV)
        {
            ImGui::Indent();
            ImGui::Image((ImTextureID)pSRV, ImVec2(64, 64));

            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Image((ImTextureID)pSRV, ImVec2(256, 256));
                ImGui::EndTooltip();
            }
            ImGui::Unindent();
        }
    }
}

CInspector_Material* CInspector_Material::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CAssimpEditor* _pEditor)
{
    CInspector_Material* pInstance = new CInspector_Material(_pDevice, _pDeviceContext, _pEditor);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CInspector_Material");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CInspector_Material::Free()
{
    __super::Free();
}
