#pragma once
#include "MapTool_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(MapTool)
class CEditorApplication final : public CBase
{
private:
	CEditorApplication();
	virtual ~CEditorApplication() = default;

public:
	HRESULT Initialize();
	void Update(_float _fTimeDelta);
	HRESULT Render();

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	class CImGui_Manager* m_pImGuiManager = { nullptr };

	LEVEL m_eLevel = { LEVEL::END };
	_bool m_bLevelChangeEnable = { false };

private:
	HRESULT Start_Level(LEVEL _eStartLevel);
	HRESULT Loading_Resources();
	HRESULT Render_Change_Level();
	void Change_Level();
	HRESULT Ready_Layer_Light();

public:
	static CEditorApplication* Create();
	virtual void Free();
};
NS_END