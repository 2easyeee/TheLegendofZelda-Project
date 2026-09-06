#pragma once
#include "ImGui_Object.h"
#include "GameObject/MapObject_Light.h"

NS_BEGIN(MapTool)
class CTool_Light final : public CImGui_Object
{
private:
	CTool_Light(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CMapEditor* _pMapEditor);
	virtual ~CTool_Light() = default;

public:
	virtual HRESULT Initialize(CMapEditor* _pMapEditor);
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	class CMapEditor* m_pMapEditor = { nullptr };

	LIGHT_DESC m_EditDesc = {};
	class CMapObject_Light* m_pTarget = { nullptr };

	_int m_iFileIndex_Save = 0;
	_int m_iFileIndex_Load = 0;

private:
	void Render_Light();
	HRESULT Render_Save_Button();
	HRESULT Render_Load_Button();

private:
	HRESULT Save_LightBinary(_uint _iLevelTotalNum, _uint _iFileIndex);

public:
	static CTool_Light* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CMapEditor* _pMapEditor);
	void Free() override;
};
NS_END