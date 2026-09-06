#pragma once
#include "Base.h"
#include "MapObject.h"
#include "ContainerObject.h"

NS_BEGIN(Engine)

constexpr _bool ENABLE_FIELD_STATIC_BATCHING = true;

class ENGINE_DLL CFileIO : public CBase
{
private:
	typedef struct tagCloneDesc
	{
		CGameObject::OBJECT_DESC tObjectDesc;
		CWorldObject::WORLD_DESC tWorldDesc;
	}CLONE_DESC;

	typedef struct tagLightInitDesc
	{
		CGameObject::OBJECT_DESC tObjectDesc;
		CWorldObject::WORLD_DESC tWorldDesc;
		LIGHT_DESC tLightDesc;
	} LIGHT_INIT_DESC;

	typedef struct tagModelLoadContext
	{
		class CModel* pModel = { nullptr };
		_string strMeshPath = {};
		_wstring strModelTag = {};
		HRESULT hrResult = { E_FAIL };

#ifdef _DEBUG
	DWORD dwWorkerThreadID = { 0 };
#endif // _DEBUG

	} MODEL_LOAD_CONTEXT;

private:
	CFileIO(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CFileIO() = default;

public:
	HRESULT Initialize();

public:
	HRESULT Save_MapBinary(_uint _iLevelTotalNum, _uint _iFileIndex);
	HRESULT Load_ResourceOnly(_uint _iInstanceLevel);
	HRESULT Load_InstancesOnly(_uint _iInstanceLevel, _uint _iFileIndex = 0);
	HRESULT Load_ResourceAndInstances(_uint _iInstanceLevel, _uint _iFileIndex = 0);
	HRESULT Load_ContainerXML(_uint _iInstanceLevel, const _wstring& _ContainerID, CContainerObject::CONTAINERR_INIT_DESC& _OutDesc, _wstring _OutMeshPath);
	HRESULT Clone_Instance_Container(_uint _iInstanceLevel, CContainerObject::CONTAINERR_INIT_DESC _ContainerTag, class CGameObject** _ppOut = nullptr);
	HRESULT Parse_XML_TAGS(_wstring _XMLPath, EXPORT_TAGS& _outTags);
	HRESULT Save_AssetCache(_uint _iInstanceLevel);

	/* Effect */
	HRESULT Register_EffectMeshes(_uint _iInstanceLevel);
	HRESULT Register_EffectTextures(_uint _iInstanceLevel);
	HRESULT Save_EffectXML(SHADER_PARAM_DESC _Param, SHADER_TIMELINE _Timeline, SHADER_TEXTURE_DESC _Textures); 
	HRESULT Load_EffectXML(_wstring _EffectName, SHADER_PARAM_DESC& _OutParam, SHADER_TIMELINE& _OutTimeline, SHADER_TEXTURE_DESC& _OutTextures); 
	
	/* Light */
	HRESULT Load_LightBinary(_uint _iInstanceLevel, _uint _iFileIndex = 0);
	HRESULT Clone_LightBinary(_uint _iInstanceLevel, _uint _iFileIndex = 0);

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	vector<CGameObject::OBJECT_DESC> m_vecInstanceObjectDescs;
	vector<CWorldObject::WORLD_DESC> m_vecInstanceWorldDescs;

	map<_wstring, _wstring> m_AssetCache;

	/* Thread */
	class CTasksSystem* m_pTasksSystem = { nullptr };

private:
	/* Prototype */
	HRESULT Load_Full_Prototypes(_uint _iInstanceLevel);
	HRESULT Load_Prototypes(_uint _iFileIndex, _uint _iInstanceLevel);
	HRESULT Register_Prototype(_uint _iInstanceLevel, _wstring _ModelTag, _wstring _MeshPath);
	HRESULT Register_PreparedPrototype(_uint _iInstanceLevel, const _wstring& _ModelTag, class CModel* _pModel);

	/* Binary */
	HRESULT Load_MapBinary(_uint _iFileIndex);
	
	/* GameObject */
	HRESULT Clone_Instance(CGameObject::OBJECT_DESC _tObjectDesc, CWorldObject::WORLD_DESC _tWorldDesc);

	/* Cache */
	HRESULT Bake_AssetCache_FromXML(_wstring _RootBinPath, _wstring _OutBinPath);
	HRESULT Load_AssetCache(_wstring _BinPath);
	HRESULT Write_AssetCache(_wstring _OutBinPath);
	HRESULT Read_AssetCache(_wstring _BinPath);
	_bool IsExistContainer(_wstring _ObjectID);

	/* Thread */
	static HRESULT Execute_ModelCPU(void* _pContext);

public:
	static CFileIO* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual void Free() override;
};
NS_END