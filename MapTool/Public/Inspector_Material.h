#pragma once
#include "ImGui_Object.h"

NS_BEGIN(MapTool)
class CInspector_Material final : public CImGui_Object
{
private:
	CInspector_Material(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		class CAssimpEditor* _pEditor);
	virtual ~CInspector_Material() = default;

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
	const aiMaterial* m_pMaterial = { nullptr };
	_int m_iMeshIndex = { -1 };
	_int m_iMaterialIndex = { -1 };

private:
	void Render_Node();
	void Render_Texture(aiTextureType _aiTextureType, const char* _label);

public:
	static CInspector_Material* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		class CAssimpEditor* _pEditor);
	void Free() override;
};
NS_END