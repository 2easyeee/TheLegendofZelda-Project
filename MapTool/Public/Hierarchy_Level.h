#pragma once
#include "ImGui_Object.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(MapTool)
class CHierarchy_Level final : public CImGui_Object
{
private:
	CHierarchy_Level(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		class CMapEditor* _pEditor);
	virtual ~CHierarchy_Level() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CMapEditor* m_pMapEditor = { nullptr };
	class CGameObject* m_pImGuiSelected = { nullptr };

public:
	static CHierarchy_Level* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		class CMapEditor* _pEditor);
	void Free() override;
};
NS_END