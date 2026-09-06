#pragma once
#include "ImGui_Object.h"

NS_BEGIN(MapTool)
class CInspector_Mesh final : public CImGui_Object
{
private:
	CInspector_Mesh(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		class CAssimpEditor* _pEditor);
	virtual ~CInspector_Mesh() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CAssimpEditor* m_pAssimpEditor = { nullptr };
	const aiNode* m_paiNode = { nullptr };
	const aiScene* m_paiScene = { nullptr };
	const aiMesh* m_pMesh = { nullptr };
	_int m_iMeshIndex = {-1};

	int m_iDebugVertexIndex = 0;
	vector<uint32_t> m_VertexInfluenceCount;
	_uint m_MaxInfluence = 0;


private:
	void Render_Node();

public:
	static CInspector_Mesh* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		class CAssimpEditor* _pEditor);
	void Free() override;
};
NS_END