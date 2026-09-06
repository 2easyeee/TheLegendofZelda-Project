#pragma once
#include "MapTool_Defines.h"
#include "Level.h"

NS_BEGIN(MapTool)
class CEffectEditor final : public CLevel
{
private:
	CEffectEditor(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CEffectEditor() = default;

public:
	virtual HRESULT Initialize(class CImGui_Manager* _pImGuiManager);
	virtual void Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CImGui_Manager* m_pImGuiManager = { nullptr };

private:
	HRESULT Ready_Layer_Camera(const _wstring& strLayerTag);

public:
	static CEffectEditor* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CImGui_Manager* _pImGuiManager);
	virtual void Free() override;
};
NS_END