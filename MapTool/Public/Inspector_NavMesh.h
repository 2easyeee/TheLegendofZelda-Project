#pragma once
#include "ImGui_Object.h"
#include "NavigationEditor.h"

NS_BEGIN(Engine)
class CNavigation;
class CCell;
NS_END

NS_BEGIN(MapTool)
class CInspector_NavMesh final : public CImGui_Object
{
private:
	CInspector_NavMesh(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CInspector_NavMesh() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	vector<_float3> m_vecTempPoints;
	_int m_iSelectedCell = { -1 };

	/* GameObject */
	class CNavigation* m_pNavCom = { nullptr };
	class CNavHolder* m_pCurrentNavHolder = { nullptr };

	/* FileIO */
	_int m_iFileIndex_Save = { -1 };
	_int m_iFileIndex_Load = { -1 };
	_int m_iFileIndex_Map = { -1 };
	_bool m_bDeleteNavHolder = { false };

	/* Height */
	_float m_fHeightOffset = { 0.f };
	_float m_fSelectedHeightOffset = { 0.f };
	_float m_fPrevSelectedHeightOffset = { 0.f };

	/* Slope */
	_float m_fSlopeX = { 0.f };
	_float m_fPrevSlopeX = { 0.f };
	_float m_fSlopeZ = { 0.f };
	_float m_fPrevSlopeZ = { 0.f };

	/* Multi Select */
	vector<_int> m_vecSelectedCells;

private:
	void Update_Delete_Nav();

	HRESULT Render_FileIO();
	HRESULT Render_Cell_Info();
	HRESULT Render_Create_Cell();
	HRESULT Render_Delete_Cell();
	HRESULT Render_Height();
	HRESULT Render_Position();
	HRESULT Render_Position_Multi();

	void Handle_MouseInput();
	void Picking_Cell();
	void Save_Binary();
	void Undo();
	HRESULT Init_NavFile();
	void Apply_SelectedCell_Height(_float _fOffset);
	void Apply_SelectedCell_Slope(_float _fOffset, _bool _bActive);
	void Move_SelectedCells(_float3 _fOffset);

	HRESULT Load_Resources();

	HRESULT MakeNavigationHolder();
	_float3 Snap_RoundVector3(_float3& v3, _float3 vStep);
	void Snap_Translate(_float3& vPosition, _float3 vStep);
	
public:
	static CInspector_NavMesh* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	void Free() override;
};
NS_END