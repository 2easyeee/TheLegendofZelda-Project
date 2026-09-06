#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class ENGINE_DLL CUIObject abstract : public CGameObject
{
public:
	enum class UI_LAYER { BACKGROUND, HUD, POPUP, OVERLAY, END };
	enum class UI_INTERACTION { NON_INTERACTION, INTERACTION, END};
	enum class UI_BLEND { NONBLEND, ALPHABLEND, END};
	typedef struct tagUIObjectDesc : public CTransform::TRANSFORM_DESC
	{
		_float fX, fY;
		_float fSizeX, fSizeY;
		UI_LAYER eUILayer;
		UI_INTERACTION eUIInteraction;
		UI_BLEND eUIBlend;
	}UI_DESC;

	typedef struct tagUIInitDesc
	{
		OBJECT_DESC tObjectDesc;
		UI_DESC tuiDesc;
	}UI_INIT_DESC;

protected:
	CUIObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CUIObject(const CUIObject& _Prototype);
	virtual ~CUIObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Bind_ViewMatrix(CShader* _pShader, const _char* _pConstantName);
	HRESULT Bind_ProjMatrix(CShader* _pShader, const _char* _pConstantName);

	UI_LAYER Get_UI_Layer() { return m_eUILayer; }
	void Set_UIPosition(_float _fX, _float _fY) { m_fX = _fX; m_fY = _fY; }
	void Apply_UITransform();
	void Set_UIScale(_float _fSizeX, _float _fSizeY);

protected:
	UI_LAYER m_eUILayer = {};
	UI_INTERACTION m_eUIInteraction = {};
	UI_BLEND m_eUIBlend = {};

private:
	_float4x4 m_ViewMatrix = {};
	_float4x4 m_ProjMatrix = {};

	_float m_fX;
	_float m_fY;
	_float m_fSizeX;
	_float m_fSizeY;

private:
	_bool IsPicked(HWND hWnd);

public:
	virtual CGameObject* Clone(void* _pArg) = 0;
	virtual void Free() override;
};
NS_END