#pragma once
#include "Client_Defines.h"
#include "UI_Rect.h"

NS_BEGIN(Client)
class CUI_Spinner final : public CUI_Rect
{
private:
	CUI_Spinner(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CUI_Spinner(const CUI_Spinner& _Prototype);
	virtual ~CUI_Spinner() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float m_fRotSpeed = {};
	_float m_fAngle = {};

public:
	static CUI_Spinner* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END