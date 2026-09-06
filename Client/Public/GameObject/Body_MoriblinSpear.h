#pragma once
#include "Client_Defines.h"
#include "Body.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CBody_MoriblinSpear : public CBody
{
private:
	CBody_MoriblinSpear(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CBody_MoriblinSpear(const CBody_MoriblinSpear& _Prototype);
	virtual ~CBody_MoriblinSpear() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	/* Collider */
	void Collider_SensorEnable(_bool _bActive);

	/* Transform */
	void Set_ParentWorld(const _float4x4* _pParent) { m_pParentMatrix = _pParent; }
	void Set_Owner_Parent(class CGameObject* _pOwner);

private:
	/* Collider */
	CCollider* m_pSensorCollider = { nullptr };

	/* Transform */
	const _float4x4* m_pParentMatrix = { nullptr };
	class CGameObject* m_pOwner = { nullptr };

private:
	HRESULT Ready_Collider();

public:
	static CBody_MoriblinSpear* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg);
	virtual void Free() override;
};
NS_END