#pragma once
#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)
class CWeapon_Monster final : public CPartObject
{
private:
	CWeapon_Monster(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CWeapon_Monster(const CWeapon_Monster& _Prototype);
	virtual ~CWeapon_Monster() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

	const _float4x4* Get_SocketMatrixPtr(const _char* _pBoneName);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	const _uint* m_pParentState = { nullptr };
	const _float4x4* m_pSocketMatrix = { nullptr };

private:
	HRESULT Ready_Components(OBJECT_DESC* _Desc);

public:
	static CWeapon_Monster* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END