#pragma once
#include "MapTool_Defines.h"

NS_BEGIN(MapTool)
class CaiConverter final
{
public:
	CaiConverter();
	~CaiConverter() = default;

public:
	HRESULT Read_AssetFile(_wstring _middlePath);
	HRESULT Export_ModelData();
	HRESULT Export_MaterialData();

	const aiScene* Get_aiScene() { return m_pAIScene; }
	const _wstring  Get_XMLPath() { return m_XMLPath; }

private:
	HRESULT Read_ModelData(aiNode* _node, _int _iIndex, _int _iParent);
	HRESULT Read_NonAnimMeshData(aiNode* _node, _uint _iMeshIndex, _uint _iBone);
	HRESULT Read_SkinnedMeshData(aiNode* _node, _uint _iMeshIndex, _uint _iBone);
	HRESULT Read_AnimationData();
	HRESULT Write_ModelFile(_wstring _MeshFullPath);

private:
	HRESULT Read_MaterialData();
	HRESULT Write_MaterialFile(_wstring _TextureFullPath);
	_string Write_TextureFile(_string _middlePath, _string _file);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	const aiScene* m_pAIScene = {};
	Assimp::Importer m_Importer = {};

	vector<ASBONE> m_vecBones;
	vector<ASMESH> m_vecMeshes;
	vector<ASMATERIAL> m_vecMaterials;
	vector<ASANIMATION> m_vecAnimations;

	MODEL m_eModelType = {};
	_char m_szBoneName[MAX_PATH] = {};

private:
	_wstring m_AssetBasePath = TEXT("../../Resources/Assets/");
	_wstring m_ModelBasePath = TEXT("../../Resources/Models/");
	_wstring m_MiddlePath = TEXT("");
	_wstring m_XMLPath = TEXT("");

private:
	_bool Contains(const _string& src, const _string& key);
	
public:
	void Replace(OUT _string& str, _string comp, _string rep);
	void Replace(OUT _wstring& wstr, _wstring comp, _wstring rep);
	_int Get_BoneIndex_ByNodeName(_string _nodeName);
	_int Find_AttachBone_ByNodeName(aiNode* _node);

public:
	static CaiConverter* Create();
	void Free();
};
NS_END