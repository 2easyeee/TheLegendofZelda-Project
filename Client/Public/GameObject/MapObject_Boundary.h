#pragma once
#include "Client_Defines.h"
#include "MapObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CMapObject_Boundary final : public CMapObject
{
private:
	CMapObject_Boundary(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMapObject_Boundary(const CMapObject_Boundary& _Prototype);
	virtual ~CMapObject_Boundary() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CCollider* m_pColliderCom = { nullptr };

	_float3 m_vBaseExtents;

private:
	HRESULT Ready_Collider(WORLD_DESC* _Desc);

public:
	static CMapObject_Boundary* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END