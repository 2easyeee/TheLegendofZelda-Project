#include "Inspector_Shader.h"
#include "GameInstance.h"

extern wstring g_DroppedFilePath;

CInspector_Shader::CInspector_Shader(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CImGui_Object{ _pDevice, _pDeviceContext }
	, m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CInspector_Shader::Initialize()
{
    /* Create ShaderPreivewObject */
    if (FAILED(Create_ShaderPreviewObject()))
        return E_FAIL;

    /* Load Mesh List */
    m_pGameInstance->Register_EffectMeshes(ENUM_TO_UINT(LEVEL::STATIC));
    Load_Mesh_List();

	return S_OK;
}

void CInspector_Shader::Update(_float fTimeDelta)
{
}

void CInspector_Shader::LateUpdate(_float fTimeDelta)
{
}

HRESULT CInspector_Shader::Render()
{
	ImGui::Begin("Shader Editor");
	if (FAILED(Render_Shader_Param()))
		return E_FAIL;
	ImGui::End();

    ImGui::Begin("Shader FileIO");
    if (FAILED(Render_Save()))
        return E_FAIL;
    if (FAILED(Render_Load()))
        return E_FAIL;
    ImGui::End();

    return S_OK;
}

HRESULT CInspector_Shader::Render_Shader_Param()
{
    /* Texture */
    if (!m_MeshTags.empty())
    {
        _wstring selectedTag;

        Render_ImGui_Combo("##CreateGameObjectTag", m_MeshTags, m_tComboDesc_Mesh, selectedTag);

        if (m_tComboDesc_Mesh.bIsSelected)
        {
            m_pShaderPreviewObject->Change_Model(selectedTag);
            m_tComboDesc_Mesh.bIsSelected = false;
        }
    }

    /* Slot Buttons Column (Left) */
    ImGui::BeginGroup();
    const char* slotNames[] = {
        "Main",
        "Noise_0",
        "Noise_1",
        "Mask",
        "Distortion",
        "Dissolve"
    };

    const _int slotCount = IM_ARRAYSIZE(slotNames);

    /* Cal Size */
    _float totalWidth = ImGui::GetContentRegionAvail().x;
    _float spacing = ImGui::GetStyle().ItemSpacing.x;
    _float buttonWidth = (totalWidth - spacing * (slotCount - 1)) / slotCount;
    _float buttonHeight = 26.f;

    for (_int i = 0; i < slotCount; ++i)
    {
        _bool selected = (m_eSelectedSlot == (CShaderPreview::TEXTURE_SLOT)i);

        if (selected)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.6f, 1.f, 1.f));

        if (ImGui::Button(slotNames[i], ImVec2(buttonWidth, buttonHeight)))
            m_eSelectedSlot = (CShaderPreview::TEXTURE_SLOT)i;

        if (selected)
            ImGui::PopStyleColor();

        if (i < slotCount - 1)
            ImGui::SameLine();
    }
    ImGui::EndGroup();

    /* Drop Area */
    if (!g_DroppedFilePath.empty())
    {
        Load_Texture(g_DroppedFilePath.c_str());
        g_DroppedFilePath.clear();
    }

    /* Texture */
    _float slotWidth = buttonWidth;
    _float slotImageSize = slotWidth - 16.f;
    if (ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto DrawSlot = [&](const char* name, CShaderPreview::TEXTURE_SLOT slot, int id)
            {
                ImGui::PushID(id);

                ImGui::BeginGroup();
                ImGui::Text(name);

                ImGui::BeginChild("slot", ImVec2(slotWidth, slotWidth), true);

                ID3D11ShaderResourceView* texture = m_pShaderPreviewObject->Get_Texture(slot);

                if (texture)
                {
                    ImGui::SetCursorPos(ImVec2(8, 8));
                    ImGui::Image((ImTextureID)texture, ImVec2(slotImageSize, slotImageSize));
                }
                else
                {
                    ImGui::Dummy(ImVec2(slotImageSize, slotImageSize));
                }

                ImGui::EndChild();
                ImGui::EndGroup();

                ImGui::PopID();
            };

        for (int i = 0; i < slotCount; ++i)
        {
            DrawSlot(slotNames[i], (CShaderPreview::TEXTURE_SLOT)i, i);

            if (i < slotCount - 1)
                ImGui::SameLine();
        }
    }

    /* Color */
    if (ImGui::CollapsingHeader("Color##Detail", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit4("Color", (float*)&m_pShaderPreviewObject->m_tShaderDesc.vColor);
        ImGui::SliderFloat("Alpha", &m_pShaderPreviewObject->m_tShaderDesc.fAlpha, 0.f, 1.f);
    }

    /* UV */
    if (ImGui::CollapsingHeader("UV##Detail", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat2("UV Scale", (float*)&m_pShaderPreviewObject->m_tShaderDesc.vUVScale, 0.01f);
        ImGui::DragFloat2("UV Speed", (float*)&m_pShaderPreviewObject->m_tShaderDesc.vUVSpeed, 0.01f);
    }

    /* Noise_0 */
    if (ImGui::CollapsingHeader("Noise_0##Detail", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat2("Noise Scale##0", (float*)&m_pShaderPreviewObject->m_tShaderDesc.vNoiseScale_0, 0.01f);
        ImGui::DragFloat2("Noise Speed##0", (float*)&m_pShaderPreviewObject->m_tShaderDesc.vNoiseSpeed_0, 0.01f);
    }

    /* Noise_1 */
    if (ImGui::CollapsingHeader("Noise_1##Detail", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat2("Noise Scale##1", (float*)&m_pShaderPreviewObject->m_tShaderDesc.vNoiseScale_1, 0.01f);
        ImGui::DragFloat2("Noise Speed##1", (float*)&m_pShaderPreviewObject->m_tShaderDesc.vNoiseSpeed_1, 0.01f);
    }

    /* Distortion */
    if (ImGui::CollapsingHeader("Distortion##Detail", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat("Strength", &m_pShaderPreviewObject->m_tShaderDesc.fDistortion, 0.f, 1.f);
        ImGui::DragFloat2("Direction", (float*)&m_pShaderPreviewObject->m_tShaderDesc.vDistortionDir, 0.01f);
        ImGui::SliderFloat("Speed", &m_pShaderPreviewObject->m_tShaderDesc.fDistortionSpeed, 0.f, 10.f);
    }

    /* Dissolve */
    if (ImGui::CollapsingHeader("Dissolve##Detail", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat("Amount", &m_pShaderPreviewObject->m_tShaderDesc.fDissolve, 0.f, 1.f);
        ImGui::SliderFloat("Edge", &m_pShaderPreviewObject->m_tShaderDesc.fDissolveEdge, 0.f, 0.5f);
        ImGui::ColorEdit4("Edge Color", (float*)&m_pShaderPreviewObject->m_tShaderDesc.vDissolveColor);
    }

    /* Emissive */
    if (ImGui::CollapsingHeader("Emissive##Detail", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat("Intensity", &m_pShaderPreviewObject->m_tShaderDesc.fEmissive, 0.f, 10.f);
    }

    /* Flipbook */
    if (ImGui::CollapsingHeader("Flipbook##Detail", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::InputInt("Frame X", &m_pShaderPreviewObject->m_tShaderDesc.iFrameX);
        ImGui::InputInt("Frame Y", &m_pShaderPreviewObject->m_tShaderDesc.iFrameY);
        ImGui::SliderFloat("Frame Speed", &m_pShaderPreviewObject->m_tShaderDesc.fFrameSpeed, 0.f, 20.f);
    }

    /* Timeline */
    if (ImGui::CollapsingHeader("Timeline", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Checkbox("Use Timeline", &m_pShaderPreviewObject->m_bUseTimeline))
        {
            m_pShaderPreviewObject->m_tTime.fAccTime = 0.f;
        }
        ImGui::DragFloat3("Scale Start", (float*)&m_pShaderPreviewObject->m_tTimeline.vScaleStart, 0.01f);
        ImGui::DragFloat3("Scale End", (float*)&m_pShaderPreviewObject->m_tTimeline.vScaleEnd, 0.01f);
        ImGui::DragFloat3("Rotation Axis", (float*)&m_pShaderPreviewObject->m_tTimeline.vRotationAxis, 0.01f);
        ImGui::SliderFloat("Rotation Speed", &m_pShaderPreviewObject->m_tTimeline.fRotationSpeed, 0.f, 1.f);
        ImGui::SliderFloat("Alpha Start", &m_pShaderPreviewObject->m_tTimeline.fAlphaStart, 0.f, 1.f);
        ImGui::SliderFloat("Alpha End", &m_pShaderPreviewObject->m_tTimeline.fAlphaEnd, 0.f, 1.f);
        ImGui::SliderFloat("Dissolve Start", &m_pShaderPreviewObject->m_tTimeline.fDissolveStart, 0.f, 1.f);
        ImGui::SliderFloat("Dissolve End", &m_pShaderPreviewObject->m_tTimeline.fDissolveEnd, 0.f, 1.f);
        ImGui::SliderFloat("Duration", &m_pShaderPreviewObject->m_tTime.fAccDurationTime, 0.1f, 10.f);
    }

	return S_OK;
}

HRESULT CInspector_Shader::Render_Save()
{
    if (ImGui::Button("Save Effect"))
    {
        m_pGameInstance->Save_EffectXML(
            m_pShaderPreviewObject->m_tShaderDesc,
            m_pShaderPreviewObject->m_tTimeline,
            m_pShaderPreviewObject->m_tTextures
        );
    }

    return S_OK;
}

HRESULT CInspector_Shader::Render_Load()
{
    ImGui::InputText("Effect Name", EffectName, 128);
    if (ImGui::Button("Load Effect"))
    {
        SHADER_PARAM_DESC param;
        SHADER_TIMELINE timeline;
        SHADER_TEXTURE_DESC textures;

        wstring name = CHARTOWSTR(EffectName);

        m_pGameInstance->Load_EffectXML(name, param, timeline, textures);
        m_pShaderPreviewObject->Apply_Effect(param, timeline, textures);
    }

    return S_OK;
}

HRESULT CInspector_Shader::Load_Texture(const wchar_t* _Path)
{
    if (!_Path)
        return E_FAIL;

    ID3D11ShaderResourceView* pNewSRV = nullptr;

    std::wstring path(_Path);
    std::wstring ext = path.substr(path.find_last_of(L'.') + 1);

    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    HRESULT hr;

    if (ext == L"dds")
    {
        hr = DirectX::CreateDDSTextureFromFile(m_pDevice, _Path, nullptr, &pNewSRV);
    }
    else
    {
        hr = DirectX::CreateWICTextureFromFile(m_pDevice, m_pDeviceContext, _Path, nullptr, &pNewSRV);
    }

    if (FAILED(hr))
        return E_FAIL;

    /* ShaderPreviewObject */
    m_pShaderPreviewObject->Set_Texture((CShaderPreview::TEXTURE_SLOT)m_eSelectedSlot, pNewSRV, _Path);

    Safe_Release(pNewSRV);

    return S_OK;
}

HRESULT CInspector_Shader::Create_ShaderPreviewObject()
{
    /* Clone */
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("Shader Preview Object"));
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Map_ShaderPreview"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("Layer_ShaderPreviewObject"));

    CMapObject::MAP_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;

    CGameObject* pGameObject = { nullptr };
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        tObjectDesc.ObjectTag,
        ENUM_TO_UINT(LEVEL::EFFECTEDITOR),
        tObjectDesc.LayerTag,
        &tInitDesc, &pGameObject)))
        return E_FAIL;

    m_pShaderPreviewObject = static_cast<CShaderPreview*>(pGameObject);

    return S_OK;
}

void CInspector_Shader::Load_Mesh_List()
{
    m_MeshTags.clear();

    vector<_wstring> vecPrototypeTag;
    m_pGameInstance->Get_PrototypeTag(ENUM_TO_UINT(LEVEL::STATIC), vecPrototypeTag);

    Filter_Tags(vecPrototypeTag, TEXT("Prototype_Component_Model_Map_"), m_MeshTags);
}

void CInspector_Shader::Render_ImGui_Combo(const _char* _strLabel, vector<_wstring> _vecTags, COMBO_DESC& _tComboDesc, _wstring& _outTag)
{
    if (_vecTags.empty())
        return;

    ImGui::SetNextItemWidth(500.f);

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
                _tComboDesc.bIsSelected = true; /* Selected */
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    _outTag = _vecTags[_tComboDesc.iSelectedIndex];
}

void CInspector_Shader::Filter_Tags(const vector<_wstring>& _vecTags, const _wstring& _prefix, vector<_wstring>& _output)
{
    for (const auto& Tag : _vecTags)
    {
        if (Tag.compare(0, _prefix.length(), _prefix) == 0)
        {
            _output.push_back(Tag);
        }
    }
}

CInspector_Shader* CInspector_Shader::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CInspector_Shader* pInstance = new CInspector_Shader(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("FAILED TO CREATED : CInspector_Shader");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CInspector_Shader::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}
