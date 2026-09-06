#pragma once
#include "ImGui_Object.h"
#include "ContainerObject.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
class CContainerObject;
NS_END

NS_BEGIN(MapTool)
class CTool_Edit final : public CImGui_Object
{
public:
	typedef struct tagComboDesc
	{
		_uint iSelectedIndex = 0;
		_wstring wstrSelectedTag;
		bool bIsSelected = { false };
	}COMBO_DESC;

public:
	typedef struct tagPreviewDesc
	{
		EXPORT_TAGS Tags;
		_wstring DiffuseTexturePath;
		_float3 PreviewPosition;
		class CGameObject* pPreviewSphere;
	}PREVIEW_DESC;

private:
	CTool_Edit(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CTool_Edit() = default;

public:
	virtual HRESULT Initialize(class CMapEditor* _pMapEditor);
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	class CMapEditor* m_pMapEditor = { nullptr };

	_wstring m_wstrObjectID = {};
	COMBO_DESC m_tComboDesc_Object;
	COMBO_DESC m_tComboDesc_Layer;
	COMBO_DESC m_tComboDesc_Shader;
	COMBO_DESC m_tComboDesc_Model;

	vector<EXPORT_TAGS> m_vecTags;
	_int m_iSelectedIndex = { -1 };

	vector<_wstring> m_vecContainerIDs;
	vector<CContainerObject::CONTAINERR_INIT_DESC> m_vecContainerTags;
	_int m_iSelectedContainer = { -1 };

private:
	HRESULT Render_Clone_Button();
	HRESULT Render_Remove_Button();
	HRESULT Render_Preview_GameObjects();

	HRESULT Load_All_XML_TAGS();
	HRESULT Clone_GameObject();
	HRESULT Clone_Container();
	HRESULT Remove_GameObject();

public:
	static CTool_Edit* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CMapEditor* _pMapEditor);
	void Free() override;
};
NS_END