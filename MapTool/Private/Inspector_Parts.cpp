#include "Inspector_Parts.h"
#include "GameInstance.h"

CInspector_Parts::CInspector_Parts(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CImGui_Object{ _pDevice, _pDeviceContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CInspector_Parts::Initialize()
{
    return S_OK;
}

void CInspector_Parts::Update(_float fTimeDelta)
{
}

void CInspector_Parts::LateUpdate(_float fTimeDelta)
{
}

HRESULT CInspector_Parts::Render()
{
    ImGui::Begin("Parts Builder");
    Render_Parts_List();
    ImGui::Separator();
    Render_Add_Part_Button();
    ImGui::Separator();
    Render_Export_Button();
    ImGui::End();

    return S_OK;
}

HRESULT CInspector_Parts::Render_Parts_List()
{
    /* Container */
    if (ImGui::BeginTable("ContainerInfoTable", 2,
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableSetupColumn("[CONTAINER]", ImGuiTableColumnFlags_WidthFixed, 160.f);
        ImGui::TableSetupColumn("[VALUE]", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        /* Container ID */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Container ID");

        ImGui::TableSetColumnIndex(1);
        Render_ImGui_InputText("##ContainerID", m_ContainerID);

        /* Container GameObjectTag */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Container GameObjectTag");

        ImGui::TableSetColumnIndex(1);
        vector<_wstring> vecPrototypeTags;
        m_pGameInstance->Get_PrototypeTag(ENUM_TO_UINT(LEVEL::STATIC), vecPrototypeTags);

        vector<_wstring> vecContainerTags;
        Filter_Tags(vecPrototypeTags, TEXT("Prototype_GameObject_Actor_"), vecContainerTags);
        Render_ImGui_Combo("##ContainerObjectTag", vecContainerTags,m_tComboDesc_Container, m_ContainerObjectTag);

        ImGui::EndTable();
    }

    ImGui::Separator();

    /* PART LIST */
    for (size_t i = 0; i < m_vecParts.size(); ++i)
    {
        ImGui::PushID((_int)i);
        ImGui::Text("Part %d", (_int)i);
        ImGui::Separator();

        Render_ImGui_Part_Type_Combo(m_vecParts[i]);
        Render_Input_Tags(i);

        if (m_vecParts[i].PartType != PART::BODY)
            Render_ImGui_InputText("Socket Name", m_vecParts[i].SocketName);

        if (ImGui::Button("Remove Part"))
        {
            m_vecParts.erase(m_vecParts.begin() + i);
            ImGui::PopID();
            break;
        }

        ImGui::Spacing();
        ImGui::PopID();
    }

    return S_OK;
}

HRESULT CInspector_Parts::Render_Input_Tags(_uint _iIndex)
{
    vector<_wstring> vecPrototypeTags;
    m_pGameInstance->Get_PrototypeTag(ENUM_TO_UINT(LEVEL::STATIC), vecPrototypeTags);

    if (ImGui::BeginTable("CreateObjectTable", 2,
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableSetupColumn("[TAG]", ImGuiTableColumnFlags_WidthFixed, 160.f);
        ImGui::TableSetupColumn("[VALUE]", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        /* ObjectID */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("ObjectID");

        ImGui::TableSetColumnIndex(1);
        if (Render_ImGui_InputText("##ObjectID", m_vecParts[_iIndex].ObjectID))
        {
            m_vecParts[_iIndex].ModelTag =
                TEXT("Prototype_Component_Model_") + m_vecParts[_iIndex].ObjectID;
        }

        /* Layer Tag (직접 입력) */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Layer Tag");

        ImGui::TableSetColumnIndex(1);
        Render_ImGui_InputText("##LayerTag", m_vecParts[_iIndex].LayerTag);

        /* GameObject Tag */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("GameObject Tag");

        ImGui::TableSetColumnIndex(1);
        vector<_wstring> vecGameObjectTags;
        Filter_Tags(vecPrototypeTags, TEXT("Prototype_GameObject_Actor_"), vecGameObjectTags);
        Render_ImGui_Combo("##GameObjectTag",vecGameObjectTags, m_tComboDesc_GameObject,m_vecParts[_iIndex].GameObjectTag);

        /* Shader Tag */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Shader Tag");

        ImGui::TableSetColumnIndex(1);
        vector<_wstring> vecShaderTags;
        Filter_Tags(vecPrototypeTags, TEXT("Prototype_Component_Shader_"), vecShaderTags);
        Render_ImGui_Combo("##ShaderTag", vecShaderTags, m_tComboDesc_Shader, m_vecParts[_iIndex].ShaderTag);

        /* Model Tag (수정 불가) */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Model Tag");

        ImGui::TableSetColumnIndex(1);
        ImGui::BeginDisabled();
        {
            _string modelTag =
                string(m_vecParts[_iIndex].ModelTag.begin(), m_vecParts[_iIndex].ModelTag.end());
            ImGui::InputText("##ModelTag", modelTag.data(), modelTag.size() + 1);
        }
        ImGui::EndDisabled();

        /* Mesh Path */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Mesh Path");

        ImGui::TableSetColumnIndex(1);
        Render_ImGui_InputText("##MeshPath", m_vecParts[_iIndex].MeshPath);

        ImGui::EndTable();
    }

    return S_OK;
}

HRESULT CInspector_Parts::Render_Add_Part_Button()
{
    if (ImGui::Button("+"))
    {
        EXPORT_PART_TAGS tPart = {};
        tPart.PartType = PART::BODY;
        tPart.LayerTag = TEXT("Layer_Actor");
        m_vecParts.push_back(tPart);
    }
    return S_OK;
}

HRESULT CInspector_Parts::Render_Export_Button()
{
    if (ImGui::Button("Export _CONTAINER.xml"))
        Export_TAGS_ForXML();

    return S_OK;
}

HRESULT CInspector_Parts::Export_TAGS_ForXML()
{
    if (m_ContainerID.empty() || m_ContainerObjectTag.empty())
        return E_FAIL;

    if (m_vecParts.empty())
        return E_FAIL;

    tinyxml2::XMLDocument document;
    auto* pRoot = document.NewElement("Container");
    pRoot->SetAttribute("id", WSTRTOCHAR(m_ContainerID).c_str());
    pRoot->SetAttribute("objectTag",
        WSTRTOCHAR(m_ContainerObjectTag).c_str());

    document.InsertFirstChild(pRoot);

    /* PARTS */
    for (const auto& part : m_vecParts)
    {
        auto* pPart = document.NewElement("Part");

        pPart->SetAttribute("type", ENUM_TO_UINT(part.PartType)); 
        pPart->SetAttribute("ObjectID", WSTRTOCHAR(part.ObjectID).c_str());
        pPart->SetAttribute("LayerTag", WSTRTOCHAR(part.LayerTag).c_str());
        pPart->SetAttribute("GameObjectTag", WSTRTOCHAR(part.GameObjectTag).c_str());
        pPart->SetAttribute("ShaderTag", WSTRTOCHAR(part.ShaderTag).c_str());
        pPart->SetAttribute("ModelTag", WSTRTOCHAR(part.ModelTag).c_str());
        pPart->SetAttribute("MeshPath", WSTRTOCHAR(part.MeshPath).c_str());

        if (!part.SocketName.empty())
            pPart->SetAttribute("Socket", WSTRTOCHAR(part.SocketName).c_str());

        pRoot->InsertEndChild(pPart);
    }

    _wstring savePath = TEXT("../../Resources/Models/Container/") + m_ContainerID + TEXT("_CONTAINER.xml");
    _string path = WSTRTOCHAR(savePath).c_str();
    Replace(path, "\\", "/");

    document.SaveFile(path.c_str());
    MSG_BOX("SUCCESS TO SAVE : _CONTAINER.xml");

    return S_OK;
}

void CInspector_Parts::Render_ImGui_Part_Type_Combo(EXPORT_PART_TAGS& _tPart)
{
    static const char* PartNames[] = { "BODY", "WEAPON", "EFFECT" };
    _int iPartEnumIndex = ENUM_TO_UINT(_tPart.PartType);

    if (ImGui::Combo("Part Type", &iPartEnumIndex, PartNames, IM_ARRAYSIZE(PartNames)))
        _tPart.PartType = (PART)iPartEnumIndex;
}

void CInspector_Parts::Render_ImGui_Combo(const _char* _strLabel, vector<_wstring> _vecTags, COMBO_DESC& _tComboDesc, _wstring& _outTag)
{
    if (_vecTags.empty())
        return;

    ImGui::SetNextItemWidth(300.f);

    _string strCurrentTag = string().assign(_vecTags[_tComboDesc.iSelectedIndex].begin(), _vecTags[_tComboDesc.iSelectedIndex].end());
    if (ImGui::BeginCombo(_strLabel, strCurrentTag.c_str()))
    {
        for (_uint i = 0; i < _vecTags.size(); i++)
        {
            bool isSelected = (_tComboDesc.iSelectedIndex == i);
            string strSelectedTag = string().assign(_vecTags[i].begin(), _vecTags[i].end());
            if (ImGui::Selectable(strSelectedTag.c_str(), isSelected))
            {
                _tComboDesc.iSelectedIndex = i;
                _outTag = _vecTags[i];
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    _outTag = _vecTags[_tComboDesc.iSelectedIndex];
}

_bool CInspector_Parts::Render_ImGui_InputText(const _char* _strLabel, _wstring& _output)
{
    _char buffer[256] = {};
    WideCharToMultiByte(CP_UTF8, 0, _output.c_str(), -1, buffer, sizeof(buffer), nullptr, nullptr);

    ImGui::SetNextItemWidth(250.f);

    bool bChanged = { false };

    if (ImGui::InputText(_strLabel, buffer, IM_ARRAYSIZE(buffer)))
    {
        wchar_t wbuffer[256] = {};
        MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wbuffer, IM_ARRAYSIZE(wbuffer));

        _output = wbuffer;
        bChanged = true;
    }
    return bChanged;
}

void CInspector_Parts::Filter_Tags(const vector<_wstring>& _vecTags, const _wstring& _prefix, vector<_wstring>& _output)
{
    for (const auto& Tag : _vecTags)
    {
        if (Tag.compare(0, _prefix.length(), _prefix) == 0)
        {
            _output.push_back(Tag);
        }
    }
}

CInspector_Parts* CInspector_Parts::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CInspector_Parts* pInstance = new CInspector_Parts(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CInspector_Parts");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CInspector_Parts::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
