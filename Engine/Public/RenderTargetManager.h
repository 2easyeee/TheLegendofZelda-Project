#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CRenderTargetManager final : public CBase
{
private:
	CRenderTargetManager(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CRenderTargetManager() = default;

public:
	HRESULT Add_RenderTarget(const _wstring& _TargetTag, _uint _iWidth, _uint _iHeight, DXGI_FORMAT _eFormat, const _float4& _vClearColor);
	HRESULT Add_MRT(const _wstring& _MRTTag, const _wstring& _TargetTag);
	HRESULT Begin_MRT(const _wstring& _MRTTag);
	HRESULT Begin_MRT(const _wstring& _MRTTag, ID3D11DepthStencilView* _pDSV);
	HRESULT End_MRT();
	HRESULT Bind_RT_SRV(const _wstring& _TargetTag, class CShader* _pShader, const _char* _pConstantName);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	static constexpr _uint MRT_STACK_CAPACITY = 8;
	ID3D11RenderTargetView* m_pRenderTargetStack[MRT_STACK_CAPACITY] = {};
	ID3D11DepthStencilView* m_pDepthStencilStack[MRT_STACK_CAPACITY] = {};
	_uint m_iMRTStackDepth = { 0 };

	map<const _wstring, class CRenderTarget*> m_RenderTargets;
	map<const _wstring, list<class CRenderTarget*>> m_MRTs;		

private:
	class CRenderTarget* Find_RenderTarget(const _wstring& _TargetTag);
	list<class CRenderTarget*>* Find_MRT(const _wstring& _MRTTag);

public:
	static CRenderTargetManager* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual void Free() override;

#ifdef  _DEBUG
public:
	HRESULT Ready_RT_Debug(const _wstring& _TargetTag, _float _fX, _float _fY, _float _fSizeX, _float _fSizeY);
	HRESULT Render_MRT(const _wstring& _MRTTag, class CShader* _pShader, class CVIBuffer_Rect* _pVIBuffer);
#endif //  _DEBUG
};
NS_END
