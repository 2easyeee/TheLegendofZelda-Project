#pragma once
#include "ImGui_Object.h"
#include "AssimpEditor.h"

NS_BEGIN(MapTool)
class CInspector_GameObject final : public CImGui_Object
{
public:
	typedef struct tagComboDesc
	{
		_uint iSelectedIndex = 0;
		_wstring wstrSelectedTag;
		bool bIsSelected = { false };
	}COMBO_DESC;

private:
	CInspector_GameObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CInspector_GameObject() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	const EXPORT_TAGS& Get_ExportTags() { return m_tExportTags; }

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	EXPORT_TAGS m_tExportTags = {};

	_wstring m_wstrObjectID = {};
	COMBO_DESC m_tComboDesc_Object;
	COMBO_DESC m_tComboDesc_Shader;

private:
	HRESULT Render_Input_Tags();

	void Render_ImGui_Combo(const _char* _strLabel, vector<_wstring> _vecTags, COMBO_DESC& _tComboDesc, _wstring& _outTag);
	void Render_ImGui_InputText(const _char* _strLabel, _wstring& _output);
	void Filter_Tags(const vector<_wstring>& _vecTags, const _wstring& _prefix, vector<_wstring>& _output);

public:
	static CInspector_GameObject* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	void Free() override;
};
NS_END