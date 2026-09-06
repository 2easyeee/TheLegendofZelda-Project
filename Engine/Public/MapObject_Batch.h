#pragma once
#include "Engine_Defines.h"
#include "GameObject.h"
#include "WorldObject.h"

NS_BEGIN(Engine)
class CMapObject_Batch final : public CGameObject
{
public:
	typedef struct tagBatchVertex
	{
		_float3 vPosition;
		_float3 vNormal;
		_float2 vTexcoord;
	}BATCH_VERTEX;

private:
	CMapObject_Batch(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMapObject_Batch(const CMapObject_Batch& _Prototype);
	virtual ~CMapObject_Batch() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Add_StaticInstance(class CModel* pModel, _matrix worldMatrix);
	HRESULT Build_Buffers();

private:
	ID3D11Buffer* m_pVB = { nullptr };
	ID3D11Buffer* m_pIB = { nullptr };

	CShader* m_pShaderCom = { nullptr };

	vector<BATCH_VERTEX> m_MergedVertices;
	vector<_uint> m_MergedIndices;

	_uint m_iIndexCount = { 0 };

public:
	static CMapObject_Batch* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END