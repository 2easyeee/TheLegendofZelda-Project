#pragma once
#include "Client_Defines.h"
#include "UI_Rect.h"

NS_BEGIN(Client)
class CUI_Number final : public CUI_Rect
{
private:
	CUI_Number(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CUI_Number(const CUI_Number& _Prototype);
	virtual ~CUI_Number() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

	void Set_Number(_uint _iNumber) { m_iNumber = _iNumber; }

private:
	_uint m_iNumber = {};

public:
	static CUI_Number* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END