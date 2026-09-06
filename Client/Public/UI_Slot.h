#pragma once
#include "Client_Defines.h"
#include "UI_Rect.h"

NS_BEGIN(Client)
class CUI_Slot final : public CUI_Rect
{
private:
	CUI_Slot(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CUI_Slot(const CUI_Slot& _Prototype);
	virtual ~CUI_Slot() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

	void Equip(_uint _iIndex);

private:
	_uint m_iEquipIndex = { 0 };

public:
	static CUI_Slot* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END