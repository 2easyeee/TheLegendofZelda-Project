#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)
class CUI_Rect : public CUIObject
{
protected:
	CUI_Rect(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CUI_Rect(const CUI_Rect& _Prototype);
	virtual ~CUI_Rect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

	void Set_Alpha(_float _fAlpha) { m_fAlpha = max(0.f, min(_fAlpha, 1.f)); }
	_float Get_Alpha() { return m_fAlpha; }

protected:
	CShader* m_pShaderCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };

	_uint m_iTextureIndex = { 0 };

	_float m_fAlpha = { 1.f };

private:
	HRESULT Ready_Components(OBJECT_DESC* _pDesc);

public:
	virtual CGameObject* Clone(void* _pArg) override { return nullptr; }
	virtual void Free() override;

};
NS_END