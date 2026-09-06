#pragma once
#include "Client_Defines.h"
#include "MapObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CMapObject_Collision final : public CMapObject
{
private:
	CMapObject_Collision(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMapObject_Collision(const CMapObject_Collision& _Prototype);
	virtual ~CMapObject_Collision() = default;

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
	static CMapObject_Collision* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END