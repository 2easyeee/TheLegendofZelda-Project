#pragma once
#include "Client_Defines.h"
#include "UI_Rect.h"

NS_BEGIN(Client)
class CUI_Heart final : public CUI_Rect
{
public:
	enum class HP { EMPTY, HALF, FULL };

private:
	CUI_Heart(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CUI_Heart(const CUI_Heart& _Prototype);
	virtual ~CUI_Heart() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

	void Set_Index(_uint iIndex) { m_iIndex = iIndex; }
	void Set_HP_State(_float _fCurrentHP) { m_fHPState = _fCurrentHP - m_iIndex;  }

private:
	HP m_eHP = { HP::FULL };
	_uint m_iIndex = { }; // Index in Vector
	_float m_fHPState = {};

private:
	void Update_HP_State();

public:
	static CUI_Heart* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END