#include "ImGui_Manager.h"
#include "GameInstance.h"

CImGui_Manager::CImGui_Manager()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CImGui_Manager::Initialize(HWND _hWnd, ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);
	style.FontScaleDpi = main_scale;
	io.ConfigDpiScaleFonts = true;
	io.ConfigDpiScaleViewports = true;
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(_hWnd);
	ImGui_ImplDX11_Init(_pDevice, _pDeviceContext);

	return S_OK;
}

void CImGui_Manager::Update(_float _fTimeDelta)
{
	for (auto& Pair : m_umImGuiObjects)
	{
		Pair.second->Update(_fTimeDelta);
	}
}

void CImGui_Manager::Late_Update(_float _fTimeDelta)
{
	for (auto& Pair : m_umImGuiObjects)
	{
		Pair.second->LateUpdate(_fTimeDelta);
	}
}

void CImGui_Manager::Clear()
{
	for (auto& Pair : m_umImGuiObjects)
		Safe_Release(Pair.second);
	m_umImGuiObjects.clear();
}

HRESULT CImGui_Manager::Render_Begin()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	return S_OK;
}

HRESULT CImGui_Manager::Render()
{
	for (auto& Pair : m_umImGuiObjects)
	{
		Pair.second->Render();
	}
	return S_OK;
}

HRESULT CImGui_Manager::Render_End()
{
	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();

	return S_OK;
}

CImGui_Manager* CImGui_Manager::Create(HWND _hWnd, ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CImGui_Manager* pInstance = new CImGui_Manager();
	if (FAILED(pInstance->Initialize(_hWnd, _pDevice, _pDeviceContext)))
	{
		MSG_BOX("FAILED TO CREATED : CImGui_Manager");
		Safe_Release(pInstance);
		return nullptr;
	}
	return pInstance;
}

void CImGui_Manager::Free()
{
	__super::Free();

	for (auto& Pair : m_umImGuiObjects)
		Safe_Release(Pair.second);
	m_umImGuiObjects.clear();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	Safe_Release(m_pGameInstance);
}
