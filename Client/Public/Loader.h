#pragma once
#include <atomic>
#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)
class CLoader final : public CBase
{
private:
	CLoader(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CLoader() = default;

public:
	HRESULT Initialize(LEVEL _eNextLevel);
	HRESULT Loading();
	HRESULT Loading_For_Level_Loading();

public:
	_bool isFinished();
	void Output();

private:
	CGameInstance* m_pGameInstance = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	LEVEL m_eNextLevel = {};
	std::atomic_bool m_isFinish = { false };

	_tchar m_szLoadingText[MAX_PATH] = {};
	
	/* Multi Thread */
	HANDLE m_hThread = {};
	CRITICAL_SECTION m_CriticalSectionKey = {};

	/* Debug Timer */
	_float m_fLoadingAccTime = 0.f;

private:
	HRESULT Loading_For_Level_Logo();
	HRESULT Loading_For_Level_MarinHouse();
	HRESULT Loading_For_Level_Creadit();
	HRESULT Loading_For_Level_SandBox();
	HRESULT Loading_For_Level_Field();
	HRESULT Loading_For_Level_MarthasBay();
	HRESULT Loading_For_Level_Dungeon();
	HRESULT Loading_For_Level_Tower();
	HRESULT Loading_For_Level_TelephoneBox();

private:
	_matrix PreTransformMatrix(
		const _float3& _scale = {1.f, 1.f, 1.f},
		const _float3& _rotation = {0.f, 0.f, 0.f},
		const _float3& _translation = { 0.f, 0.f, 0.f });
	void Debug_Timer();
	void Set_LoadingText(const wchar_t* _pText);

public:
	static CLoader* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, LEVEL _eNextLevel);
	virtual void Free() override;
};
NS_END