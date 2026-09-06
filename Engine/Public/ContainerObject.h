#pragma once
#include "GameObject.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class ENGINE_DLL CContainerObject abstract : public CGameObject
{
public:
	typedef struct tagContainerObjectDesc final : public CTransform::TRANSFORM_DESC
	{
		_uint iTotalPartsCnt;
		_float3 vPosition = { 0.f, 0.f, 0.f };
		_float3 vRotation = { 0.f, 0.f, 0.f };
		_float3 vScale = { 1.f, 1.f, 1.f };
	}CONTAINER_DESC;

	typedef struct tagContainerInitDesc
	{
		OBJECT_DESC tObjectDesc;
		CONTAINER_DESC tContainerDesc;
		vector<CPartObject::PART_CREATE_DESC> vecPartsDescs;
	}CONTAINERR_INIT_DESC;

protected:
	CContainerObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CContainerObject(const CContainerObject& _Prototype);
	virtual ~CContainerObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	vector<class CPartObject*> m_vecPartObjects;
	_uint m_iTotalPartsCnt = {};

protected:
	HRESULT Add_PartObject(_uint _iPartObjIndex, _uint _iPrototypeLevelIndex,
		const _wstring& _strPrototypeTag, void* _pArg);

public:
	virtual CGameObject* Clone(void* _pArg) = 0;
	virtual void Free() override;
};
NS_END