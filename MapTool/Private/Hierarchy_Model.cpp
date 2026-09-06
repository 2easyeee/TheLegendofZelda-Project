#include "Hierarchy_Model.h"
#include "AssimpEditor.h"

CHierarchy_Model::CHierarchy_Model(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CAssimpEditor* _pEditor)
    : CImGui_Object{ _pDevice, _pDeviceContext }
    , m_pAssimpEditor { _pEditor }
{
}

HRESULT CHierarchy_Model::Initialize()
{
    m_strTitle = TEXT("Hierarchy");

    return S_OK;
}

void CHierarchy_Model::Update(_float fTimeDelta)
{
    m_paiScene = m_pAssimpEditor->Get_aiScene();
}

void CHierarchy_Model::LateUpdate(_float fTimeDelta)
{
}

HRESULT CHierarchy_Model::Render()
{
    if (!m_pAssimpEditor)
        return E_FAIL;

    ImGui::Begin("Hierarchy");
    if (!m_paiScene || !m_paiScene->mRootNode)
    {
        ImGui::TextDisabled("No Asset");
    }
    else
    {
        Render_Node(m_paiScene->mRootNode);
    }
    ImGui::End();

    return S_OK;
}

void CHierarchy_Model::Render_Node(const aiNode* _pNode)
{
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanFullWidth;

    if (m_pAssimpEditor->Get_SelectedNode() == _pNode)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (_pNode->mNumChildren == 0)
        flags |= ImGuiTreeNodeFlags_Leaf;

    _bool bOpened = ImGui::TreeNodeEx(_pNode->mName.C_Str(), flags);

    /* Child Node */
    if (ImGui::IsItemFocused() || ImGui::IsItemClicked() || ImGui::IsItemToggledOpen())
    {
        m_pAssimpEditor->Set_SelectedNode(_pNode);
    }

    if (bOpened)
    {
        for (_uint i = 0; i < _pNode->mNumChildren; ++i)
        {
            Render_Node(_pNode->mChildren[i]);
        }
        ImGui::TreePop();
    }
}

CHierarchy_Model* CHierarchy_Model::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CAssimpEditor* _pEditor)
{
    CHierarchy_Model* pInstance = new CHierarchy_Model(_pDevice, _pDeviceContext, _pEditor);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CHierarchy_Model");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CHierarchy_Model::Free()
{
    __super::Free();
}
