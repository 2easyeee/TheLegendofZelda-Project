#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CRenderTarget final : public CBase
{
private:
	CRenderTarget(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CRenderTarget() = default;

public:
	HRESULT Initialize(_uint _iWidth, _uint _iHeight, DXGI_FORMAT _eFormat, const _float4& _vClearColor);
	HRESULT Bind_SRV(class CShader* _pShader, const _char* _pConstantName);
	void Clear();

	ID3D11RenderTargetView* Get_RTV() const;

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	ID3D11Texture2D* m_pTexture2D = { nullptr };
	ID3D11RenderTargetView* m_pRTV = { nullptr };
	ID3D11ShaderResourceView* m_pSRV = { nullptr };

	_float4 m_vClearColor = {};

public:
	static CRenderTarget* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		_uint _iWidth, _uint _iHeight, DXGI_FORMAT _eFormat, const _float4& _vClearColor);
	virtual void Free() override;

#ifdef _DEBUG
private:
	_float4x4 m_WorldMatrix = {};

public:	
	HRESULT Ready_RT_Debug(_float _fX, _float _fY, _float _fSizeX, _float _fSizeY);
	HRESULT Render(class CShader* _pShader, class CVIBuffer_Rect* _pVIBuffer);
#endif // _DEBUG

};
NS_END