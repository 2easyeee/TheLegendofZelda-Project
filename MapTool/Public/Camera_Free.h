#pragma once
#include "MapTool_Defines.h"
#include "Camera.h"

NS_BEGIN(MapTool)
class CCamera_Free final : public CCamera
{
private:
	CCamera_Free(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CCamera_Free(const CCamera_Free& _Prototype);
	virtual ~CCamera_Free() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_bool m_bMouseEnable = { false };

private:
	HRESULT Ready_Components();
	void Mouse_Enable();

public:
	static CCamera_Free* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END