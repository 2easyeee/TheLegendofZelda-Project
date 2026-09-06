#pragma once
#include "MapTool_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
class CTransform;
NS_END

NS_BEGIN(MapTool)
class CAssimpEditor final : public CLevel
{
private:
	CAssimpEditor(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CAssimpEditor() = default;

public:
	virtual HRESULT Initialize(class CImGui_Manager* _pImGuiManager);
	virtual void Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	const aiScene* Get_aiScene() { return m_paiScene; }
	const aiNode* Get_SelectedNode() { return m_pSelectedNode; }
	void Set_SelectedNode(const aiNode* _pNode);
	_int Get_SelectedMeshIndex() { return m_SelectedMeshIndex; }
	_int Get_SelectedMaterialIndex() { return m_SelectedMaterialIndex; }
	class CTexture_Preview* Get_TexturePreview() { return m_pTexturePreview; }
	_wstring Get_TextureFullPath() { return m_wstrFullFilePath; }
	_uint Get_AnimationTotalCount() const { if (!m_paiScene) return 0; return m_paiScene->mNumAnimations; }
	aiAnimation* Get_Animation(_uint _iIndex);
	class CGameObject* Get_PreviewObject() { return m_pPreviewObject; }

private:
	class CaiConverter* m_pAIConverter = { nullptr };
	const aiScene* m_paiScene = { nullptr };
	class CImGui_Manager* m_pImGuiManager = { nullptr };

private:
	class CGameObject* m_pPreviewObject = { nullptr };

private:
	const aiNode* m_pSelectedNode = { nullptr };
	_int m_SelectedMeshIndex = {-1};
	_int m_SelectedMaterialIndex = {};

private:
	class CTexture_Preview* m_pTexturePreview = { nullptr };
	_wstring m_wstrFullFilePath = {};

private:
	HRESULT Ready_Layer_Camera(const _wstring& strLayerTag);
	HRESULT Create_GameObject();

private:
	void Render_Import_FBX();
	void Render_Export_FBX();
	void Render_Model();

private:
	HRESULT Export_FBX();
	HRESULT Import_FBX();
	_wstring Extract_AssetRelativePath(const _wstring& _FullPath);

private:
	void Export_GameObjectXML(_wstring _XMLPath);
	HRESULT Load_GameObjectXML(_wstring _XMLPath, EXPORT_TAGS& _outTags);
	_wstring Extract_ObjectID(_wstring _XMLPath);

public:
	static CAssimpEditor* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CImGui_Manager* _pImGuiManager);
	virtual void Free() override;
};
NS_END