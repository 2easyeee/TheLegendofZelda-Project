#include "Loader.h"
#include "GameInstance.h"
#include "UI_Image.h"
#include "UI_Heart.h"
#include "UI_Number.h"
#include "UI_Item.h"
#include "UI_Slot.h"
#include "UI_Spinner.h"
#include "UI_Transition.h"

CLoader::CLoader(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: m_pGameInstance { CGameInstance::GetInstance() }
	, m_pDevice { _pDevice }
	, m_pDeviceContext { _pDeviceContext }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pDeviceContext);
}

/* Multi Thread */
_uint APIENTRY ThreadMain(void* pArg)
{
	HRESULT hrCoInitialize = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hrCoInitialize))
		return 1;

	CLoader* pLoader = static_cast<CLoader*>(pArg);
	HRESULT hrLoading = pLoader->Loading();

	/* 성공한 COM 초기화 호출은 같은 스레드에서 해제 */
	CoUninitialize();

	if (FAILED(hrLoading))
		return 1;

	return 0;
}

HRESULT CLoader::Initialize(LEVEL _eNextLevel)
{
	/* Set Level */
	m_eNextLevel = _eNextLevel;
	
	/* memory_order_relaxed : 단순 초기값 사용, 별도의 acquire/release 동기화 불필요 */
	m_isFinish.store(false, std::memory_order_relaxed);

	/* Multi Thread */
	InitializeCriticalSection(&m_CriticalSectionKey);
	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, ThreadMain, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Loading()
{
	/* Debug Timer (START) */
	m_fLoadingAccTime = 0.f;
	m_pGameInstance->Add_Timer(TEXT("Loading_Timer"));
	m_pGameInstance->Compute_TimeDelta(TEXT("Loading_Timer"));

	HRESULT hr = {};
	switch (m_eNextLevel)
	{
	case Client::LEVEL::LOGO:
		hr = Loading_For_Level_Logo();
		break;
	case Client::LEVEL::MARINHOUSE:
		hr = Loading_For_Level_MarinHouse();
		break;
	case Client::LEVEL::FIELD:
		hr = Loading_For_Level_Field();
		break;
	case Client::LEVEL::CREDIT:
		hr = Loading_For_Level_Creadit();
		break;
	case Client::LEVEL::SANDBOX:
		hr = Loading_For_Level_SandBox();
		break;
	case Client::LEVEL::MARTHAsBAY:
		hr = Loading_For_Level_MarthasBay();
		break;
	case Client::LEVEL::DUNGEON:
		hr = Loading_For_Level_Dungeon();
		break;
	case Client::LEVEL::TOWER:
		hr = Loading_For_Level_Tower();
		break;
	case Client::LEVEL::TELEPHONEBOX:
		hr = Loading_For_Level_TelephoneBox();
		break;
	default:
		hr = E_FAIL;
		break;
	}
	
	if (FAILED(hr))
		return E_FAIL;

	/* Debug Timer (END) */
	m_pGameInstance->Compute_TimeDelta(TEXT("Loading_Timer"));
	m_fLoadingAccTime = m_pGameInstance->Get_TimeDelta(TEXT("Loading_Timer"));
	Debug_Timer();

	/* 이전 로딩 작업을 main thread 에 공개한 뒤 완료 상태를 설정 */
	m_isFinish.store(true, std::memory_order_release);

	return S_OK;
}

HRESULT CLoader::Loading_For_Level_Logo()
{
	Set_LoadingText(TEXT("텍스쳐를(을) 로딩중입니다."));
	/* For.Prototype_Component_Texture_Logo_Background */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::LOGO),
		TEXT("Prototype_Component_Texture_Logo_Background"),
		CTexture::Create(m_pDevice, m_pDeviceContext, 
			TEXT("../../Resources/Textures2D/UI/Logo_Background.dds"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Logo_Title_0 */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::LOGO),
		TEXT("Prototype_Component_Texture_Logo_Title_0"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/Logo_Title_0.dds"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Logo_Title_1 */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::LOGO),
		TEXT("Prototype_Component_Texture_Logo_Title_1"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/Logo_Title_1.dds"), 1))))
		return E_FAIL;

	Set_LoadingText(TEXT("모델를(을) 로딩중입니다."));

	Set_LoadingText(TEXT("객체원형를(을) 로딩중입니다."));
	/* For.Prototype_GameObject_UI_Image */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::LOGO),
		TEXT("Prototype_GameObject_UI_Image"),
		CUI_Image::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;

	Set_LoadingText(TEXT("로딩이 완료되었습니다."));

	return S_OK;
}

HRESULT CLoader::Loading_For_Level_MarinHouse()
{
	Set_LoadingText(TEXT("Map를(을) 로딩중입니다."));
	m_pGameInstance->Load_ResourceAndInstances(ENUM_TO_UINT(LEVEL::MARINHOUSE), ENUM_TO_UINT(LEVEL::MARINHOUSE));

	Set_LoadingText(TEXT("텍스쳐를(을) 로딩중입니다."));
	/* For.Prototype_Component_Texture_Loading_Background */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::MARINHOUSE),
		TEXT("Prototype_Component_Texture_Logo_Background_White"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/ScreenFade_01.dds"), 1))))
		return E_FAIL;

	Set_LoadingText(TEXT("모델를(을) 로딩중입니다."));

	Set_LoadingText(TEXT("객체원형를(을) 로딩중입니다."));

	Set_LoadingText(TEXT("로딩이 완료되었습니다."));

	return S_OK;
}

HRESULT CLoader::Loading_For_Level_Creadit()
{
	Set_LoadingText(TEXT("텍스쳐를(을) 로딩중입니다."));
	/* For.Prototype_Component_Texture_Credit_Background */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::CREDIT),
		TEXT("Prototype_Component_Texture_Credit_Background"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/Credit_BG_1.png"), 1))))
		return E_FAIL;

	Set_LoadingText(TEXT("모델를(을) 로딩중입니다."));

	Set_LoadingText(TEXT("객체원형를(을) 로딩중입니다."));

	Set_LoadingText(TEXT("로딩이 완료되었습니다."));

	return S_OK;
}

HRESULT CLoader::Loading_For_Level_SandBox()
{
	Set_LoadingText(TEXT("Map를(을) 로딩중입니다."));
	//m_pGameInstance->Load_ResourceAndInstances(ENUM_TO_UINT(LEVEL::SANDBOX), ENUM_TO_UINT(LEVEL::SANDBOX));

	Set_LoadingText(TEXT("텍스쳐를(을) 로딩중입니다."));
	
	Set_LoadingText(TEXT("모델를(을) 로딩중입니다."));
	//{
	//	CContainerObject::CONTAINERR_INIT_DESC tContainerInitDesc = {};
	//	tContainerInitDesc.tContainerDesc.vPosition = { 40.8f, 7.5f, 21.f };
	//	m_pGameInstance->Load_ContainerXML(
	//		ENUM_TO_UINT(LEVEL::MARTHAsBAY),
	//		TEXT("MoriblinSpear"),
	//		tContainerInitDesc, TEXT(""));
	//	m_pGameInstance->Clone_Instance_Container(ENUM_TO_UINT(LEVEL::MARTHAsBAY), tContainerInitDesc);
	//}

	Set_LoadingText(TEXT("콜라이더를(을) 로딩중입니다."));

	Set_LoadingText(TEXT("객체원형를(을) 로딩중입니다."));

	Set_LoadingText(TEXT("로딩이 완료되었습니다."));

	return S_OK;
}

HRESULT CLoader::Loading_For_Level_Field()
{
	Set_LoadingText(TEXT("Map를(을) 로딩중입니다."));
	//m_pGameInstance->Load_ResourceAndInstances(ENUM_TO_UINT(LEVEL::FIELD), ENUM_TO_UINT(LEVEL::FIELD));
	m_pGameInstance->Load_ResourceAndInstances(ENUM_TO_UINT(LEVEL::FIELD), 66);

	return S_OK;
}

HRESULT CLoader::Loading_For_Level_MarthasBay()
{
	Set_LoadingText(TEXT("Map를(을) 로딩중입니다."));
	m_pGameInstance->Load_ResourceAndInstances(ENUM_TO_UINT(LEVEL::MARTHAsBAY), ENUM_TO_UINT(LEVEL::MARTHAsBAY));

	{
		CContainerObject::CONTAINERR_INIT_DESC tContainerInitDesc = {};
		tContainerInitDesc.tContainerDesc.vPosition = { 45.6f, 0.f, 10.f };
		m_pGameInstance->Load_ContainerXML(
			ENUM_TO_UINT(LEVEL::MARTHAsBAY),
			TEXT("MoriblinSpear"),
			tContainerInitDesc, TEXT(""));
		m_pGameInstance->Clone_Instance_Container(ENUM_TO_UINT(LEVEL::MARTHAsBAY), tContainerInitDesc);
	}
	
	{
		CContainerObject::CONTAINERR_INIT_DESC tContainerInitDesc = {};
		tContainerInitDesc.tContainerDesc.vPosition = { 40.8f, 0.f, 21.f };
		m_pGameInstance->Load_ContainerXML(
			ENUM_TO_UINT(LEVEL::MARTHAsBAY),
			TEXT("MoriblinSpear"),
			tContainerInitDesc, TEXT(""));
		m_pGameInstance->Clone_Instance_Container(ENUM_TO_UINT(LEVEL::MARTHAsBAY), tContainerInitDesc);
	}

	return S_OK;
}

HRESULT CLoader::Loading_For_Level_Dungeon()
{
	Set_LoadingText(TEXT("Map를(을) 로딩중입니다."));
	m_pGameInstance->Load_ResourceAndInstances(ENUM_TO_UINT(LEVEL::DUNGEON), ENUM_TO_UINT(LEVEL::DUNGEON));

	Set_LoadingText(TEXT("텍스쳐를(을) 로딩중입니다."));
	/* For.Prototype_Component_Texture_Loading_Background */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::DUNGEON),
		TEXT("Prototype_Component_Texture_Logo_Background_White"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/ScreenFade_01.dds"), 1))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Loading_For_Level_Tower()
{
	Set_LoadingText(TEXT("Map를(을) 로딩중입니다."));
	m_pGameInstance->Load_ResourceAndInstances(ENUM_TO_UINT(LEVEL::TOWER), ENUM_TO_UINT(LEVEL::TOWER));

	return S_OK;
}

HRESULT CLoader::Loading_For_Level_TelephoneBox()
{
	Set_LoadingText(TEXT("Map를(을) 로딩중입니다."));
	m_pGameInstance->Load_ResourceAndInstances(ENUM_TO_UINT(LEVEL::TELEPHONEBOX), ENUM_TO_UINT(LEVEL::TELEPHONEBOX));

	Set_LoadingText(TEXT("텍스쳐를(을) 로딩중입니다."));
	/* For.Prototype_Component_Texture_Loading_Background */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::TELEPHONEBOX),
		TEXT("Prototype_Component_Texture_Logo_Background_White"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/ScreenFade_01.dds"), 1))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Loading_For_Level_Loading()
{
	return S_OK;
}

_bool CLoader::isFinished()
{
	/* 완료 상태와 그 이전 worker 작업을 함께 관찰 */
	return m_isFinish.load(std::memory_order_acquire);
}

void CLoader::Output()
{
	_tchar szLoadingText[MAX_PATH] = {};

	EnterCriticalSection(&m_CriticalSectionKey);

	lstrcpyn(szLoadingText, m_szLoadingText, MAX_PATH);

	LeaveCriticalSection(&m_CriticalSectionKey);

	/* 임계영역 밖에서 호출 */
	/* 공유 데이터 복사만 보호해서, 임계영역 유지 시간을 짧게 유지 */
	SetWindowText(g_hWnd, szLoadingText);
}

_matrix CLoader::PreTransformMatrix(const _float3& _scale, const _float3& _rotation, const _float3& _translation)
{
	return XMMatrixScaling(_scale.x, _scale.y, _scale.z)
		* XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(_rotation.x),
			XMConvertToRadians(_rotation.y),
			XMConvertToRadians(_rotation.z))
		* XMMatrixTranslation(_translation.x, _translation.y, _translation.z);
}

void CLoader::Debug_Timer()
{
	wchar_t szLog[128]{};
	swprintf(szLog, 128, L"Loading Time : %.3f sec\n", m_fLoadingAccTime);
	OutputDebugStringW(szLog);
}

void CLoader::Set_LoadingText(const wchar_t* _pText)
{
	if (_pText) return;

	/* 공유 문자열의 쓰기 구간만 잠근다. */
	EnterCriticalSection(&m_CriticalSectionKey);

	lstrcpyn(m_szLoadingText, _pText, MAX_PATH);

	LeaveCriticalSection(&m_CriticalSectionKey);
}

CLoader* CLoader::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, LEVEL _eNextLevel)
{
	CLoader* pInstance = new CLoader(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize(_eNextLevel)))
	{
		MSG_BOX("FAILED TO CREATED : CLoader");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CLoader::Free()
{
	__super::Free();

	/* Multi Thread */
	/* https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject */
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = nullptr;
	}
	DeleteCriticalSection(&m_CriticalSectionKey);

	Safe_Release(m_pDeviceContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
