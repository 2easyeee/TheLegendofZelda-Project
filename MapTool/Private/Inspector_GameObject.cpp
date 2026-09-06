#include "Inspector_GameObject.h"
#include "GameInstance.h"

CInspector_GameObject::CInspector_GameObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CImGui_Object{ _pDevice, _pDeviceContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CInspector_GameObject::Initialize()
{
    /* TEST */
    m_tExportTags.LayerTag = TEXT("Layer_Unknwon");
    return S_OK;
}

void CInspector_GameObject::Update(_float fTimeDelta)
{
}

void CInspector_GameObject::LateUpdate(_float fTimeDelta)
{
}

HRESULT CInspector_GameObject::Render()
{
	ImGui::Begin("GameObject");
	Render_Input_Tags();
	ImGui::End();

	return S_OK;
}

HRESULT CInspector_GameObject::Render_Input_Tags()
{
    /* Get Prototypes */
    vector<_wstring> vecPrototypeTags;
    vector<_wstring> vecPrototypeTags_Static;
    m_pGameInstance->Get_PrototypeTag(ENUM_TO_UINT(LEVEL::MAPEDITOR), vecPrototypeTags);
    m_pGameInstance->Get_PrototypeTag(ENUM_TO_UINT(LEVEL::STATIC), vecPrototypeTags_Static);
    for (const auto& tag : vecPrototypeTags_Static)
    {
    	vecPrototypeTags.push_back(tag);
    }

    if (ImGui::BeginTable("CreateObjectTable", 2,
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableSetupColumn("[TAG]", ImGuiTableColumnFlags_WidthFixed, 160.f);
        ImGui::TableSetupColumn("[VALUE]", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        /* Layer Tag */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Layer Tag");

        ImGui::TableSetColumnIndex(1);
        Render_ImGui_InputText("##CreateNewLayerTag", m_tExportTags.LayerTag);

        /* GameObject Tag */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("GameObject Tag");

        ImGui::TableSetColumnIndex(1);
        vector<_wstring> vecGameObjectTags;
        Filter_Tags(vecPrototypeTags, TEXT("Prototype_GameObject_MapObject_"), vecGameObjectTags);
        Filter_Tags(vecPrototypeTags, TEXT("Prototype_GameObject_Actor_"), vecGameObjectTags);
        Filter_Tags(vecPrototypeTags, TEXT("Prototype_GameObject_Map_"), vecGameObjectTags);
        Render_ImGui_Combo("##CreateGameObjectTag", vecGameObjectTags, m_tComboDesc_Object, m_tExportTags.GameObjectTag);

        /* Shader Tag */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Shader Tag");

        ImGui::TableSetColumnIndex(1);
        vector<_wstring> vecShaderTags;
        Filter_Tags(vecPrototypeTags, TEXT("Prototype_Component_Shader_VtxEffect"), vecShaderTags);
        Filter_Tags(vecPrototypeTags, TEXT("Prototype_Component_Shader_VtxMesh"), vecShaderTags);
        Filter_Tags(vecPrototypeTags, TEXT("Prototype_Component_Shader_VtxAnimMesh"), vecShaderTags);
        Filter_Tags(vecPrototypeTags, TEXT("Prototype_Component_Shader_VtxWater"), vecShaderTags);
        Render_ImGui_Combo("##CreateShaderTag", vecShaderTags, m_tComboDesc_Shader, m_tExportTags.ShaderTag);

        ImGui::EndTable();
    }

	return S_OK;
}

void CInspector_GameObject::Render_ImGui_Combo(const _char* _strLabel, vector<_wstring> _vecTags, COMBO_DESC& _tComboDesc, _wstring& _outTag)
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
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

	_outTag = _vecTags[_tComboDesc.iSelectedIndex];
}

void CInspector_GameObject::Render_ImGui_InputText(const _char* _strLabel, _wstring& _output)
{
    _char buffer[256] = {};
    WideCharToMultiByte(CP_UTF8, 0, _output.c_str(), -1, buffer, sizeof(buffer), nullptr, nullptr);

    ImGui::SetNextItemWidth(250.f);

    if (ImGui::InputText(_strLabel, buffer, IM_ARRAYSIZE(buffer)))
    {
        wchar_t wbuffer[256] = {};
        MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wbuffer, IM_ARRAYSIZE(wbuffer));

        _output = wbuffer;
    }
}

void CInspector_GameObject::Filter_Tags(const vector<_wstring>& _vecTags, const _wstring& _prefix, vector<_wstring>& _output)
{
    for (const auto& Tag : _vecTags)
    {
        if (Tag.compare(0, _prefix.length(), _prefix) == 0)
        {
            _output.push_back(Tag);
        }
    }
}

CInspector_GameObject* CInspector_GameObject::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CInspector_GameObject* pInstance = new CInspector_GameObject(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CInspector_GameObject");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CInspector_GameObject::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
