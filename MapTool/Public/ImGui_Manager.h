#pragma once
#include "MapTool_Defines.h"
#include "Base.h"
#include "ImGui_Object.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(MapTool)
class CImGui_Manager : public CBase
{
private:
	CImGui_Manager();
	virtual ~CImGui_Manager() = default;

public:
	HRESULT Initialize(HWND _hWnd, ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	void Update(_float _fTimeDelta);
	void Late_Update(_float _fTimeDelta);
	void Clear();

public:
	HRESULT Render_Begin();
	HRESULT Render();
	HRESULT Render_End();

	template<typename T>
	void Register_ImGui_Objcet(const _wstring& wstrKey, T* pObject)
	{
		m_umImGuiObjects[wstrKey] = pObject;
	}

	CImGui_Object* Find_ImGui_Object(const _wstring& wstrKey)
	{
		auto iter = m_umImGuiObjects.find(wstrKey);

		if (iter == m_umImGuiObjects.end())
			return nullptr;
		else
			return iter->second;
	}

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	unordered_map<_wstring, CImGui_Object*> m_umImGuiObjects;

public:
	static CImGui_Manager* Create(HWND _hWnd, ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual void Free() override;
};
NS_END