#pragma once
#include "Client_Defines.h"
#include "UI_Rect.h"

NS_BEGIN(Client)
class CUI_Transition final : public CUI_Rect
{
public:
	enum class UI_TRANSITION { NONE, APPEAR, EXPAND, HOLD, FADEIN, FADEOUT, END };

private:
	CUI_Transition(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CUI_Transition(const CUI_Transition& _Prototype);
	virtual ~CUI_Transition() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;
	
public:
	void Start_Transition(UI_TRANSITION _eTransition) { m_eUITransition = _eTransition; }
	_bool IsFinished() { return m_eUITransition == UI_TRANSITION::END; }

private:
	UI_TRANSITION m_eUITransition = UI_TRANSITION::NONE;
	_float m_fElaspedTime = { 0.f };
	_float m_fExpandDuration = { 0.3f };
	_float m_fStartScale = { 1.f };
	_float m_fTargetScale = { 10000.f };
	_float m_fHoldTime = { 0.f };
	_float m_fTargetTime = { 0.15f };

	_float m_fFadeElaspedTime = { 0.f };
	_float m_fFadeDuration = { 2.f };

public:
	static CUI_Transition* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END