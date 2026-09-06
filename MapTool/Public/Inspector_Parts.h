#pragma once
#include "ImGui_Object.h"
#include "PartsBuilder.h"

NS_BEGIN(MapTool)
class CInspector_Parts final : public CImGui_Object
{
public:
	typedef struct tagComboDesc
	{
		_uint iSelectedIndex = 0;
		_wstring wstrSelectedTag;
		bool bIsSelected = { false };
	}COMBO_DESC;

private:
	CInspector_Parts(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CInspector_Parts() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	_wstring m_ContainerID = {};
	_wstring m_ContainerObjectTag = {};
	vector<EXPORT_PART_TAGS> m_vecParts;
	COMBO_DESC m_tComboDesc_Container;
	COMBO_DESC m_tComboDesc_GameObject;
	COMBO_DESC m_tComboDesc_Shader;

private:
	HRESULT Render_Parts_List();
	HRESULT Render_Input_Tags(_uint _iIndex);
	HRESULT Render_Add_Part_Button();
	HRESULT Render_Export_Button();

	HRESULT Export_TAGS_ForXML();

	void Render_ImGui_Part_Type_Combo(EXPORT_PART_TAGS& _tPart);
	void Render_ImGui_Combo(const _char* _strLabel, vector<_wstring> _vecTags, COMBO_DESC& _tComboDesc, _wstring& _outTag);
	_bool Render_ImGui_InputText(const _char* _strLabel, _wstring& _output);
	void Filter_Tags(const vector<_wstring>& _vecTags, const _wstring& _prefix, vector<_wstring>& _output);

public:
	static CInspector_Parts* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	void Free() override;
};
NS_END