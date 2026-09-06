#include "Tool_Edit.h"
#include "GameInstance.h"
#include "MapObject.h"
#include "MapEditor.h"
#include "GameObject/MapObject_Light.h"

CTool_Edit::CTool_Edit(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CImGui_Object{ _pDevice, _pDeviceContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CTool_Edit::Initialize(CMapEditor* _pMapEditor)
{
	if (!_pMapEditor)
		return E_FAIL;

	m_pMapEditor = _pMapEditor;

	/* Load All XML TAGS */
	m_pGameInstance->Load_ResourceOnly(ENUM_TO_UINT(LEVEL::MAPEDITOR));
	Load_All_XML_TAGS();

	// Load_Container
	{
		CContainerObject::CONTAINERR_INIT_DESC tContainerInitDesc = {};
		m_pGameInstance->Load_ContainerXML(
			ENUM_TO_UINT(LEVEL::MAPEDITOR),
			TEXT("Player"),
			tContainerInitDesc, TEXT(""));
		m_vecContainerIDs.push_back(tContainerInitDesc.tObjectDesc.ObjectID);
		m_vecContainerTags.push_back(tContainerInitDesc);
	}

	{
		CContainerObject::CONTAINERR_INIT_DESC tContainerInitDesc = {};
		m_pGameInstance->Load_ContainerXML(
			ENUM_TO_UINT(LEVEL::MAPEDITOR),
			TEXT("MoriblinSpear"),
			tContainerInitDesc, TEXT(""));
		m_vecContainerIDs.push_back(tContainerInitDesc.tObjectDesc.ObjectID);
		m_vecContainerTags.push_back(tContainerInitDesc);
	}

	return S_OK;
}

void CTool_Edit::Update(_float fTimeDelta)
{
}

void CTool_Edit::LateUpdate(_float fTimeDelta)
{
}

HRESULT CTool_Edit::Render()
{
	ImGui::Begin("Edit (Clone)");
	Render_Preview_GameObjects();
	Render_Clone_Button();
	ImGui::End();
	
	ImGui::Begin("Edit (Remove)");
	Render_Remove_Button();
	ImGui::End();

	return S_OK;
}

HRESULT CTool_Edit::Render_Clone_Button()
{
	ImGui::SeparatorText("Clone Container");
	if (ImGui::Button("Clone Container"))
	{
		Clone_Container();
	}
	ImGui::BeginChild("ContainerList", ImVec2(0, 150), true);
	for (size_t i = 0; i < m_vecContainerIDs.size(); ++i)
	{
		bool selected = (m_iSelectedContainer == (_int)i);
		if (ImGui::Selectable(
			WSTRTOCHAR(m_vecContainerIDs[i]).c_str(),
			selected))
		{
			m_iSelectedContainer = (_int)i;
		}
	}
	ImGui::EndChild();

	return S_OK;
}

HRESULT CTool_Edit::Render_Remove_Button()
{
	if (ImGui::Button("Remove"))
	{
		Remove_GameObject();
	}

	return S_OK;
}

HRESULT CTool_Edit::Render_Preview_GameObjects()
{
	ImGui::SeparatorText("This is A Clone Button");
	if (ImGui::Button("Clone"))
	{
		Clone_GameObject();
	}
	ImGui::SeparatorText("Model List");
	ImGui::BeginChild("ModelList", ImVec2(0, 200), true);
	for (size_t i = 0; i < m_vecTags.size(); ++i)
	{
		_bool bSelected = (m_iSelectedIndex == (int)i);

		if (ImGui::Selectable(
			WSTRTOCHAR(m_vecTags[i].ObjectID).c_str(),
			bSelected))
		{
			m_iSelectedIndex = (int)i;

			/* Connet PreviewGameObject */
			m_pMapEditor->Set_ImGui_PreviewGameObject(m_vecTags[i]);
		}
	}
	ImGui::EndChild();

	return S_OK;
}

HRESULT CTool_Edit::Load_All_XML_TAGS()
{
	m_vecTags.clear();

	_wstring basePath = TEXT("../../Resources/Models");

	for (const auto& entry : filesystem::recursive_directory_iterator(basePath))
	{
		if (!entry.is_regular_file())
			continue;

		const auto& path = entry.path();

		if (path.extension() != L".xml")
			continue;

		if (path.wstring().find(L"_TAGS") == _wstring::npos)
			continue;

		EXPORT_TAGS tags;
		if (FAILED(m_pGameInstance->Parse_XML_TAGS(path.wstring(), tags)))
			continue;

		m_vecTags.push_back(tags);
	}

	return S_OK;
}

HRESULT CTool_Edit::Clone_GameObject()
{
	if (m_iSelectedIndex < 0)
		return S_OK;

	EXPORT_TAGS tags = m_vecTags[m_iSelectedIndex];

	CGameObject::OBJECT_DESC tObjectDesc = {};
	wcscpy_s(tObjectDesc.ObjectID, tags.ObjectID.c_str());
	tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::MAPEDITOR);
	wcscpy_s(tObjectDesc.ObjectTag, tags.GameObjectTag.c_str());
	wcscpy_s(tObjectDesc.LayerTag, tags.LayerTag.c_str());
	wcscpy_s(tObjectDesc.ShaderTag, tags.ShaderTag.c_str());
	wcscpy_s(tObjectDesc.ModelTag, tags.ModelTag.c_str());

	if (_wstring(tObjectDesc.LayerTag) == TEXT("Light"))
	{
		CMapObject_Light::LIGHT_INIT_DESC tInitDesc = {};
		tInitDesc.tObjectDesc = tObjectDesc;
		tInitDesc.tWorldDesc.vPosition =
			m_pMapEditor->Get_ImGui_PickedPosition();
		tInitDesc.tLightDesc.eType = LIGHT::POINT;
		tInitDesc.tLightDesc.vPosition = {tInitDesc.tWorldDesc.vPosition.x, tInitDesc.tWorldDesc.vPosition.y, tInitDesc.tWorldDesc.vPosition.z, 1.f};
		tInitDesc.tLightDesc.vDirection = { 0.f, -1.f, 0.f, 0.f };

		tInitDesc.tLightDesc.vDiffuse = { 1.f,1.f,1.f,1.f };
		tInitDesc.tLightDesc.vAmbient = { 0.2f,0.2f,0.2f,1.f };
		tInitDesc.tLightDesc.vSpecular = { 1.f,1.f,1.f,1.f };

		tInitDesc.tLightDesc.fRange = 5.f;

		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
			RESOURCE_LEVEL_STATIC,
			tObjectDesc.ObjectTag,
			ENUM_TO_UINT(LEVEL::MAPEDITOR),
			tObjectDesc.LayerTag,
			&tInitDesc, nullptr)))
			return S_OK;
	}
	else
	{
		CMapObject::MAP_INIT_DESC tInitDesc = {};
		tInitDesc.tObjectDesc = tObjectDesc;
		tInitDesc.tWorldDesc.vPosition =
			m_pMapEditor->Get_ImGui_PickedPosition();

		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
			RESOURCE_LEVEL_STATIC,
			tObjectDesc.ObjectTag,
			ENUM_TO_UINT(LEVEL::MAPEDITOR),
			tObjectDesc.LayerTag,
			&tInitDesc, nullptr)))
			return S_OK;
	}

	return S_OK;
}

HRESULT CTool_Edit::Clone_Container()
{
	if (m_iSelectedContainer < 0)
		return S_OK;

	CContainerObject::CONTAINERR_INIT_DESC tInitDesc = m_vecContainerTags[m_iSelectedContainer];

	/* Container Object */
	EXPORT_PART_TAGS tags = {};
	tags.ObjectID = tInitDesc.tObjectDesc.ObjectID;
	tags.GameObjectTag = tInitDesc.tObjectDesc.ObjectTag;
	tags.LayerTag = TEXT("Layer_Actor");
	
	/* Each Level Set */
	tInitDesc.tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::MAPEDITOR);
	for (auto& Part : tInitDesc.vecPartsDescs)
	{
		Part.tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::MAPEDITOR);
	}

	/* Clone */
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
		RESOURCE_LEVEL_STATIC,
		tags.GameObjectTag,
		ENUM_TO_UINT(LEVEL::MAPEDITOR),
		tags.LayerTag,
		&tInitDesc,
		nullptr)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTool_Edit::Remove_GameObject()
{
	CGameObject* pSelectedObject = m_pMapEditor->Get_ImGui_SelectedObject();
	if (!pSelectedObject)
		return S_OK;

	m_pGameInstance->Reserve_DeleteObject(pSelectedObject);
	m_pMapEditor->Set_ImGui_SelectedObject(nullptr);

	return S_OK;
}

CTool_Edit* CTool_Edit::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CMapEditor* _pMapEditor)
{
    CTool_Edit* pInstance = new CTool_Edit(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize(_pMapEditor)))
    {
        MSG_BOX("FAILED TO CREATED : CTool_Edit");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CTool_Edit::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
