#pragma once
#include "Client_Defines.h"
#include "Body.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CBody_Player : public CBody
{
private:
	CBody_Player(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CBody_Player(const CBody_Player& _Prototype);
	virtual ~CBody_Player() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	/* Collider */
	void Collider_SwordEnable(_bool _bActive);
	void Collider_ShiledEnable(_bool _bActive);

	/* Transform */
	void Set_ParentWorld(const _float4x4* _pParent) { m_pParentMatrix = _pParent; }
	void Set_Owner_Parent(class CGameObject* _pOwner);

	/* Debug UI Mesh */
	class CModel* Get_BodyModel() const { return m_pModelCom; }

private:
	/* Collider */
	CCollider* m_pSwordCollider = { nullptr };
	CCollider* m_pShieldCollider = { nullptr };

	/* Transform */
	const _float4x4* m_pParentMatrix = { nullptr };

private:
	HRESULT Ready_Collider();
	HRESULT Attach_Socket();

public:
	static CBody_Player* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg);
	virtual void Free() override;
};
NS_END