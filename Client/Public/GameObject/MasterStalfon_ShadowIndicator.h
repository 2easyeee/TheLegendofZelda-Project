#pragma once
#include "Client_Defines.h"
#include "MapObject.h"

NS_BEGIN(Engine)
class CVIBuffer_Plane;
class CShader;
class CTexture;
NS_END

NS_BEGIN(Client)
class CMasterStalfon_ShadowIndicator final : public CMapObject
{
private:
	CMasterStalfon_ShadowIndicator(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CMasterStalfon_ShadowIndicator(const CMasterStalfon_ShadowIndicator& _Prototype);
	virtual ~CMasterStalfon_ShadowIndicator() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Set_Position(_vector _vPos);
	void Set_Scale(_float _fScale);
	void Destroy();

private:
	/* Component */
	CVIBuffer_Plane* m_pVIBufferCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };

	/* Shadow */
	_float m_fScale = { 0.f };
	_float m_fMaxScale = { 7.f };

private:
	HRESULT Ready_Components(OBJECT_DESC* _Desc);

public:
	static CMasterStalfon_ShadowIndicator* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END