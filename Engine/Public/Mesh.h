#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)
class CMesh final : public CVIBuffer
{
private:
	CMesh(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMesh(const CMesh& _Prototype);
	virtual ~CMesh() = default;

public:
	virtual HRESULT Initialize_Prototype(const MESH_DESC& _tMeshDesc, _fmatrix _PreTransformMatrix);
	virtual HRESULT Initialize(void* _pArg) override;

	vector<VERTEXTYPE> Get_Vertices() const { return m_PickingVertices; } // TODO : 이름 모호함
	vector<VTXMESH> Get_NonAnimVertices() const { return m_NonAnimVertices; } // For. Map Batch
	vector<_uint> Get_Indices() const { return m_Indices; }
	
	const vector<VTXMESH>& Get_NonAnimVerticesRef() const { return m_NonAnimVertices; }
	const vector<_uint>& Get_IndicesRef() const { return m_Indices; }
	const vector<VTXANIMMESH>& Get_SkinnedVerticesRef() const { return m_SkinnedVertices; }

	ID3D11Buffer* Get_VertexBuffer() const;
	_uint Get_VertexStride() const;
	_uint Get_IndexCount() const;

	_string Get_MeshName() const { return m_strMeshName; }
	
	_string Get_MaterialName() const { return m_strMaterialName; }
	_uint Get_MaterialIndex() const { return m_iMaterialIndex; }
	void Set_MaterialIndex(_uint iMaterialIndex) { m_iMaterialIndex = iMaterialIndex; }
	
	MODEL Get_ModelType() const { return m_eModelType; }

	HRESULT Bind_BoneMatrices(class CShader* _pShader, const _char* _pConstantName, const vector<class CBone*>& _Bones);
	HRESULT Bind_Resources();

	HRESULT Ready_VertexBuffer_NonAnim(const MESH_DESC& _tMeshDesc, _fmatrix _PreTransformMatrix);
	HRESULT Ready_VertexBuffer_Skinned(const MESH_DESC& _tMeshDesc, _fmatrix _PreTransformMatrix);

private:
	_uint m_iMaterialIndex = {};
	_string m_strMeshName = {};
	_string m_strMaterialName = {};

	vector<VERTEXTYPE> m_PickingVertices;
	vector<VTXMESH> m_NonAnimVertices;
	vector<VTXANIMMESH> m_SkinnedVertices;
	vector<_uint> m_Indices;

	_float4x4 m_BoneMatrices[512] = {};

	_int m_iEffectTotalBoneCnt = {};
	vector<_int> m_vecEffectBoneIndices;
	vector<_float4x4> m_OffsetMatrices;

	MODEL m_eModelType = {};

public:
	static CMesh* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, 
		const MESH_DESC& _tMeshDesc, _fmatrix _PreTransformMatrix);
	virtual CComponent* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END