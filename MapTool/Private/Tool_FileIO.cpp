#include "Tool_FileIO.h"
#include "GameInstance.h"

CTool_FileIO::CTool_FileIO(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CImGui_Object{ _pDevice, _pDeviceContext }
	, m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CTool_FileIO::Initialize()
{
	m_strTitle = TEXT("FileIO");

	return S_OK;
}

void CTool_FileIO::Update(_float fTimeDelta)
{
}

void CTool_FileIO::LateUpdate(_float fTimeDelta)
{
}

HRESULT CTool_FileIO::Render()
{
	ImGui::Begin("File System");
	Render_Save_Button();
	Render_Load_Button();
	ImGui::End();

	return S_OK;
}

HRESULT CTool_FileIO::Render_Save_Button()
{
	ImGui::Text("Save Level:");
	ImGui::SameLine();

	ImGui::SetNextItemWidth(30);
	ImGui::InputInt("##SaveLevel", &m_iFileIndex_Save, 0);

	ImGui::SameLine();
	if (ImGui::Button("Click!##Save"))
	{
		int iResult = MessageBox(nullptr, L"저장 잘못누른거 아니지 ???", L"Last Chance.", MB_OKCANCEL | MB_ICONQUESTION);
		if (iResult == IDOK)
		{
			m_pGameInstance->Save_MapObjects(ENUM_TO_UINT(LEVEL::END), m_iFileIndex_Save);
		}
	}

	return S_OK;
}

HRESULT CTool_FileIO::Render_Load_Button()
{
	ImGui::Text("Load Level:");
	ImGui::SameLine();

	ImGui::SetNextItemWidth(30);
	ImGui::InputInt("##LoadLevel", &m_iFileIndex_Load, 0);

	ImGui::SameLine();
	if (ImGui::Button("Click!##Load"))
	{
		int iResult = MessageBox(nullptr, L"로드하기 전에 저장 했나 ???", L"Last Chance.", MB_OKCANCEL | MB_ICONQUESTION);
		if (iResult == IDOK)
		{
			m_pGameInstance->Load_InstancesOnly(ENUM_TO_UINT(LEVEL::MAPEDITOR), m_iFileIndex_Load);
		}
	}

	return S_OK;
}

CTool_FileIO* CTool_FileIO::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CTool_FileIO* pInstance = new CTool_FileIO(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("FAILED TO CREATED : CTool_FileIO");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTool_FileIO::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}
