#pragma once
#include "MapTool_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
class CTransform;
NS_END

NS_BEGIN(MapTool)
class CPartsBuilder final : public CLevel
{
private:
	CPartsBuilder(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CPartsBuilder() = default;

public:
	virtual HRESULT Initialize(class CImGui_Manager* _pImGuiManager);
	virtual void Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CImGui_Manager* m_pImGuiManager = { nullptr };

private:
	HRESULT Ready_Layer_Camera(const _wstring& strLayerTag);

public:
	static CPartsBuilder* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CImGui_Manager* _pImGuiManager);
	virtual void Free() override;
};
NS_END