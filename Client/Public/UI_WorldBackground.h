#pragma once
#include "Client_Defines.h"
#include "UI_Rect.h"

NS_BEGIN(Client)
class CUI_WorldBackground final : public CUI_Rect
{
private:
	CUI_WorldBackground(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CUI_WorldBackground(const CUI_WorldBackground& _Prototype);
	virtual ~CUI_WorldBackground() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	static CUI_WorldBackground* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END