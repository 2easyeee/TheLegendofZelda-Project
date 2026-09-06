#pragma once
#include "ImGui_Object.h"

NS_BEGIN(MapTool)
class CTool_Animation final : public CImGui_Object
{
private:
	CTool_Animation(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CMapEditor* _pMapEditor);
	virtual ~CTool_Animation() = default;

public:
	virtual HRESULT Initialize();
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	class CMapEditor* m_pMapEditor = { nullptr };

	/* Anim Info */
	_int iAnimComboIndex = { 0 };

private:
	void Render_Animation();

private:
	const char* Get_AnimationNameList(_uint _iAnimIndex);

public:
	static CTool_Animation* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CMapEditor* _pMapEditor);
	void Free() override;
};
NS_END