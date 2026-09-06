#pragma once
#include "ImGui_Object.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
NS_END

NS_BEGIN(MapTool)
class CTool_Placement final : public CImGui_Object
{
public:
	enum class PICKING_STATE { NONE, SELECT, PLACE, END };

private:
	CTool_Placement(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CTool_Placement() = default;

public:
	virtual HRESULT Initialize(class CMapEditor* _pMapEditor);
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	class CMapEditor* m_pMapEditor = { nullptr };

	// PICKING
	PICKING_STATE m_ePickingState = PICKING_STATE::NONE;
	_float m_YFloor = { 0.f };

	// SNAP
	_bool m_bSnapEnabled = { false };
	_float3 m_vSnapStep = { 1.f, 1.f, 1.f };

	// PREVIEW
	class CGameObject* m_pPreviewObject = { nullptr };
	_bool m_bIsPreviewEnable = { false };

	// UNIQUE ID
	map<_wstring, _uint> m_ObjectsID;

private:
	HRESULT Render_Picking_Button();
	HRESULT Render_Snap_Button();

	// PICKING
	void Picking_Enable();
	void Update_Picking();

	// SNAP
	void Snap_Enable();
	_float3 Snap_RoundVector3(_float3& v3, _float3 vStep);
	void Snap_Translate(_float3& vPosition, _float3 vStep);

	// PREVIEW
	void Update_Place_Preview();
	void Clone_Preview(_float3 _vPreviewPos);
	void Clone_GameObject();

	// UNIQUE ID
	_wstring Make_UniqueID(_wstring& _ID);

public:
	static CTool_Placement* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CMapEditor* _pMapEditor);
	void Free() override;
};
NS_END