#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class ENGINE_DLL CModel final : public CComponent
{
	friend class CFileIO;

private:
	CModel(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CModel(const CModel& _Prototype);
	virtual ~CModel() = default;

public:
	virtual HRESULT Initialize_Prototype(const _char* _pModelFilePath, _fmatrix _PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;
	HRESULT Render(_uint iMeshIndex);
	
public:
	/* Mesh */
	_uint Get_NumMeshes() const { return m_iNumMeshes; }
	class CMesh* Get_Mesh(_uint _iMeshIndex) const;
	vector<class CMesh*> Get_Meshes() const { return m_vecMeshes; }
	_int Get_BoneIndex(const _char* _pBoneName);
	_uint Get_IndexCount(_uint _iMeshIndex) const;
	void Set_MeshActive(const _string& _meshName, _bool _bActive);
	void Set_AllMeshActive(_bool _bActive);
	HRESULT Build_StaticBatches();

	/* Bone */
	const _float4x4* Get_BoneMatrixPtr(const _char* _pBoneName);
	void Set_BoneOffset(const _char* _BoneName, _fmatrix _offset);
	void Clear_BoneOffset();
	void Remove_BoneOffset(const _char* _BoneName);
	void Set_BoneSnap(const _char* _TargetBoneName, const _char* _SourceBoneName);
	void Remove_BoneSnap(const _char* _TargetBoneName);
	void Clear_BoneSnap();
	
	/* Animation */
	_bool Play_Animation(_float _fTimeDelta);
	HRESULT Set_Animation_Name(_string _AnimName, _bool _isLoop = true, _bool _isForce = false);
	void Set_Animation(_uint _iAnimIndex, _bool _isLoop = true, _bool _isForce = false);
	void Reset_Animation_TrackPosition();
	void Set_AnimationSpeedMulti(_float _fAnimSpeedMulti);
	_bool IsAnimFinished() const;
	_float Get_PlayRatio() const;

public:
	HRESULT Bind_BoneMatrices(class CShader* _pShader, const _char* _pConstantName, _uint _iMeshIndex);
	HRESULT Bind_Material(class CShader* pShader, const _char* pConstantName,
		_uint iMeshIndex, MATERIAL _eMaterialType, _uint iTextureIndex = 0);
	HRESULT Bind_Resources(_uint _iMeshIndex);

private:
	/* Path */
	_string						m_strMeshPath = "";
	_string						m_strMaterialPath = "";
	/* Transform */
	_float4x4					m_PreTransformMatrix = {};
	/* Bone */
	_uint						m_iNumBones = {};
	vector<BONE_DESC>			m_vecBoneData;
	vector<class CBone*>		m_vecBones;
	MODEL						m_eModelType;

	unordered_map<_string, _int> m_BoneNameCache;
	unordered_map<_string, _matrix> m_BoneOverrides;		// For. Bone Offset
	unordered_map<_int, _int> m_BoneSnaps;
	/* Mesh */
	_uint						m_iNumMeshes = {};
	vector<MESH_DESC>			m_vecMeshDesc;
	vector<class CMesh*>		m_vecMeshes;
	map<_string, _bool>			m_MeshRenderMask;
	/* Material*/
	_uint						m_iNumMaterials = {};
	vector<class CMaterial*>	m_vecMaterials;
	map<_string, _uint>			m_MaterialNameToIndex;
	/* Animation */
	_bool						m_isAnimLoop = { false };
	_uint						m_iCurrentAnimIndex = {};
	_uint						m_iNextAnimIndex = {};
	STATE_TIME					m_tAnimLerp = { false, 0.f, 0.25f };
	_uint						m_iNumAnimations = {};
	vector<class CAnimation*>	m_vecAnimations;
	map<_string, _uint>			m_AnimNameToIndex;

private:
	HRESULT Initialize_CPU(const _char* _pModelFilePath, _fmatrix _PreTransformMatrix);
	HRESULT Initialize_GPU();
	
	HRESULT Ready_Bones(HANDLE hFile);
	HRESULT Ready_Meshes(HANDLE hFile);
	HRESULT Ready_MeshData(HANDLE hFile);
	HRESULT Ready_Materials();
	HRESULT Ready_Animations(HANDLE hFile);

	HRESULT Create_MeshResources();

private:
	void Update_BonesOverrides();
	void Update_BoneSnap();
	void Update_Bones();
	_bool Lerp_Animation(_float _fTimeDelta);
	void Register_Animation(_string _name, _uint _iIndex);

public:
	static CModel* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, 
		const _char* _pModelFilePath, _fmatrix _PreTransformMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END