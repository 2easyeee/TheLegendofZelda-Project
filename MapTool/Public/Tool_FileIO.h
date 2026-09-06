#pragma once
#include "ImGui_Object.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(MapTool)
class CTool_FileIO final : public CImGui_Object
{
private:
	CTool_FileIO(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CTool_FileIO() = default;

public:
	virtual HRESULT Initialize();
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	_int m_iFileIndex_Save = 0;
	_int m_iFileIndex_Load = 0;

private:
	HRESULT Render_Save_Button();
	HRESULT Render_Load_Button();

public:
	static CTool_FileIO* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	void Free() override;
};
NS_END