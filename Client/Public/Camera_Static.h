#pragma once
#include "Client_Defines.h"
#include "Camera.h"

NS_BEGIN(Client)
class CCamera_Static final : public CCamera
{
private:
	CCamera_Static(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CCamera_Static(const CCamera_Static& _Prototype);
	virtual ~CCamera_Static() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Components();
	void Mouse_Enable();

public:
	static CCamera_Static* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END