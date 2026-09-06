#pragma once
#include "Client_Defines.h"
#include "UI_Rect.h"

NS_BEGIN(Client)
class CUI_Item final : public CUI_Rect
{
private:
	CUI_Item(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CUI_Item(const CUI_Item& _Prototype);
	virtual ~CUI_Item() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

	void Add_Item(_uint _iCount);
	void Spend_Item(_uint _iCount);
	
	_uint Get_ItemCount() { return m_iCount; }

private:
	_uint m_iCount = { 0 };

public:
	static CUI_Item* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END