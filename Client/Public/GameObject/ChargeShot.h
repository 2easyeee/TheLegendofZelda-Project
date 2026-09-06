#pragma once
#include "Client_Defines.h"
#include "Effect.h"

NS_BEGIN(Engine)
class CEffect;
NS_END

NS_BEGIN(Client)
class CChargeShot final : public CEffect
{
private:
	CChargeShot(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CChargeShot(const CChargeShot& _Prototype);
	virtual ~CChargeShot() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	static CChargeShot* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END