#pragma once
#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)
class CWeapon_Player final : public CPartObject
{
public:
	enum class WEAPON_STATE { ATTACHED, DETACHED, END };

private:
	CWeapon_Player(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CWeapon_Player(const CWeapon_Player& _Prototype);
	virtual ~CWeapon_Player() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Throw(_float3 _vDir);

public:
	/* Boomerang */
	class CBoomerang* m_pBoomerang = { nullptr };

private:
	const _uint* m_pParentState = { nullptr };
	const _float4x4* m_pSocketMatrix = { nullptr };

	/* State */
	WEAPON_STATE m_eWeaponState = WEAPON_STATE::ATTACHED;
	_matrix m_ThrowMatrix;

private:
	void Create_Boomerang();

public:
	static CWeapon_Player* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END