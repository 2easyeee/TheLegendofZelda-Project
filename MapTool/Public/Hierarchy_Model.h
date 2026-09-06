#pragma once
#include "ImGui_Object.h"

NS_BEGIN(MapTool)
class CHierarchy_Model final : public CImGui_Object
{
private:
	CHierarchy_Model(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		class CAssimpEditor* _pEditor);
	virtual ~CHierarchy_Model() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CAssimpEditor* m_pAssimpEditor = { nullptr };
	const aiScene* m_paiScene = {nullptr};

private:
	void Render_Node(const aiNode* _pNode);

public:
	static CHierarchy_Model* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		class CAssimpEditor* _pEditor);
	void Free() override;
};
NS_END