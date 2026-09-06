#pragma once
#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Client)
class CWeapon_MoriblinSpear final : public CPartObject
{
public:
	enum class WEAPON_STATE { ATTACHED, FLYING, END };

private:
	CWeapon_MoriblinSpear(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CWeapon_MoriblinSpear(const CWeapon_MoriblinSpear& _Prototype);
	virtual ~CWeapon_MoriblinSpear() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Throw();
	void Throw_Target(_vector _vTargetPos);
	void Respawn();

private:
	const _uint* m_pParentState = { nullptr };
	const _float4x4* m_pSocketMatrix = { nullptr };

	/* State */
	WEAPON_STATE m_eWeaponState = WEAPON_STATE::ATTACHED;
	_matrix m_ThrowMatrix;

	/* Spear */
	class CSpear* m_pSpear = { nullptr };

public:
	static CWeapon_MoriblinSpear* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END