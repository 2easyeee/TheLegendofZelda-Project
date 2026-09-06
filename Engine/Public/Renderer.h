#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CRenderer final : public CBase
{
private:
	CRenderer(ID3D11Device* _pDeivce, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CRenderer() = default;

public:
	HRESULT Initialize();
	void Draw();
	HRESULT Add_RenderGroup(RENDERGROUP _eGroupID, class CGameObject* _pRenderObject);
	void Add_Instance(class CModel* _pModel, const _float4x4& _matWorld);
	void Add_Instance(class CModel* _pModel, class CShader* _pShader, const _float4x4& _matWorld);

	void Set_InstnacingEnabled(_bool _bEnabled);
	_bool Is_InstancingEnabled() const;

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	list<class CGameObject*> m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::END)];
	
	/* Render */
	ID3D11BlendState* m_pAlphaBlendState = { nullptr };
	ID3D11BlendState* m_pLightBlendState = { nullptr };
	ID3D11DepthStencilState* m_pDepthDisableState = { nullptr }; // UI
	ID3D11DepthStencilState* m_pDepthEnableState = { nullptr };
	
	ID3D11DepthStencilView* m_pShadowDSV = { nullptr };
	D3D11_VIEWPORT m_OriginalViewport = { };
	_bool m_bFXAAEnabled = { true };

	ID3D11BlendState* m_pFXAABlendState = { nullptr };

	/* RenderTaget */
	class CShader* m_pShader = { nullptr };
	class CVIBuffer_Rect* m_pVIBuffer = { nullptr };
	_float4x4 m_WorldMatrix = {};
	_float4x4 m_ViewMatrix  = {};
	_float4x4 m_ProjMatrix = {};

	/* Instancing */
	typedef struct tagInstanceBatch
	{
		CModel* pRepresentativeModel = { nullptr };
		CShader* pShader = { nullptr };
		vector<_float4x4> WorldMatrices;
	} INSTANCE_BATCH;

	static constexpr _uint INSTANCE_CAPACITY = 1000;
	_bool m_bInstancingEnabled = { false };
	map<class CMesh*, INSTANCE_BATCH> m_InstanceBatches;
	ID3D11Buffer* m_pVBInstance = { nullptr };

private:
	void Render_Priority();
	void Render_Shadow();
	void Render_NonBlend();
	void Render_NonLight();
	void Render_Blend();
	void Render_UI();

	/* RenderTaget */
	void Render_CombinedRT();

	/* Shadow */
	HRESULT Render_ShadowDepthStencilView(_uint _iWidth, _uint _iHeight);

	/* Light */
	void Render_LightAcc();

	/* Font */
	void Render_Fonts(ID3D11DeviceContext* _pDeviceContext);

	/* Instancing */
	void Render_Instancing();
	void Clear_InstanceBatches();
	HRESULT Render_InstanceBatch(INSTANCE_BATCH& _tBatch);

	/* FXAA */
	void Render_FXAA();

private:
	HRESULT Ready_DeferredResources();

public:
	static CRenderer* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual void Free() override;

#ifdef _DEBUG
private:
	list<class CComponent*> m_DebugComponent;

private:
	void Render_Debug();

public:
	HRESULT Add_DebugComponent(class CComponent* _pDebugComponent);
#endif // _DEBUG
};
NS_END
