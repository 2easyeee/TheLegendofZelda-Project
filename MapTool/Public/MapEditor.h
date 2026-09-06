#pragma once
#include "MapTool_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
class CTransform;
NS_END

NS_BEGIN(MapTool)
class CMapEditor final : public CLevel
{
private:
	CMapEditor(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CMapEditor() = default;

public:
	virtual HRESULT Initialize(class CImGui_Manager* _pImGuiManager);
	virtual void Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	class CGameObject* Get_ImGui_SelectedObject() { return m_pImGuiSelected; }
	void Set_ImGui_SelectedObject(class CGameObject* _pLastSelected) { m_pImGuiSelected = _pLastSelected; }
	
	_float3 Get_ImGui_PickedPosition() { return m_vPickedPosition; }
	void Set_ImGui_PickedPosition(_float3 _vPickedPosition) { m_vPickedPosition = _vPickedPosition; }

	EXPORT_TAGS Get_ImGui_PreviewGameObject() { return m_tPreviewGameObject; }
	void Set_ImGui_PreviewGameObject(EXPORT_TAGS _tTag) { m_tPreviewGameObject = _tTag; m_bIsPreviewUpdate = true; }
	_bool IsPreviewUpdated();

private: // Common
	class CImGui_Manager* m_pImGuiManager = { nullptr };
	class CGameObject* m_pImGuiSelected = { nullptr };

	_float3 m_vPickedPosition = { 0.f, 0.f, 0.f };

	EXPORT_TAGS m_tPreviewGameObject = {};
	_bool m_bIsPreviewUpdate = { false };

private:
	class CGameObject* Render_ImGui_Hierarchy();
	HRESULT Render_ImGui_Inspector();
	HRESULT Render_ImGui_Transform();

private:
	_matrix PreTransformMatrix(
		const _float3& _scale = { 1.f, 1.f, 1.f },
		const _float3& _rotation = { 0.f, 0.f, 0.f },
		const _float3& _translation = { 0.f, 0.f, 0.f });
	HRESULT Ready_Layer_Terrain(const _wstring& strLayerTag);
	HRESULT Ready_Layer_Camera(const _wstring& strLayerTag);

private:
	void Add_Lights();

public:
	static CMapEditor* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CImGui_Manager* _pImGuiManager);
	virtual void Free() override;
};
NS_END