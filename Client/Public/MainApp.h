#pragma once
#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)
class CMainApp final : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT Initialize();
	void Update(_float _fTimeDelta);
	HRESULT Render();

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	class CUIManager* m_pUIManager = { nullptr };

private:
	HRESULT Start_Level(LEVEL _eStartLevel);
	HRESULT Register_UIManager();

	HRESULT Loading_For_Level_Static();

#ifdef  _DEBUG
private:
	_uint m_iRenderCnt = { 0 };
	_tchar m_szFPS[MAX_PATH] = TEXT("");
	_float m_fTimeAcc = 0.f;

	void Show_FPS(_float _fTimeDelta);
#endif //  _DEBUG

public:
	static CMainApp* Create();
	virtual void Free();
};
NS_END