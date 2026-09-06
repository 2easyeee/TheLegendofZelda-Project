#include "Renderer.h"

#include "GameObject.h"
#include "GameInstance.h"
#include "UIObject.h"
#include <algorithm>

CRenderer::CRenderer(ID3D11Device* _pDeivce, ID3D11DeviceContext* _pDeviceContext)
	: m_pGameInstance{ CGameInstance::GetInstance() }
	, m_pDevice { _pDeivce }
	, m_pDeviceContext { _pDeviceContext }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pDeviceContext);
}

HRESULT CRenderer::Initialize()
{
	/* DepthDisableState */
	D3D11_DEPTH_STENCIL_DESC tDisabledesc = {};
	tDisabledesc.DepthEnable = FALSE;
	tDisabledesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	tDisabledesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	m_pDevice->CreateDepthStencilState(&tDisabledesc, &m_pDepthDisableState);

	/* DepthEnableState */
	D3D11_DEPTH_STENCIL_DESC tEnabledesc = {};
	tEnabledesc.DepthEnable = TRUE;
	tEnabledesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	tEnabledesc.DepthFunc = D3D11_COMPARISON_LESS;
	m_pDevice->CreateDepthStencilState(&tEnabledesc, &m_pDepthEnableState);

	/* BlendState (Light) */
	D3D11_BLEND_DESC tLightBlendDesc = {};
	tLightBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	tLightBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	tLightBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	tLightBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	tLightBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	tLightBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	tLightBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	tLightBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_pDevice->CreateBlendState(&tLightBlendDesc, &m_pLightBlendState);

	/* Instancing */
	D3D11_BUFFER_DESC tIVBDesc = {};
	tIVBDesc.ByteWidth = sizeof(_float4x4) * INSTANCE_CAPACITY;
	tIVBDesc.Usage = D3D11_USAGE_DYNAMIC;
	tIVBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	tIVBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tIVBDesc.MiscFlags = 0;
	tIVBDesc.StructureByteStride = sizeof(_float4x4);

	if (FAILED(m_pDevice->CreateBuffer(&tIVBDesc, nullptr, &m_pVBInstance)))
		return E_FAIL;

	/* Deffered Shade */
	if (FAILED(Ready_DeferredResources()))
 		return E_FAIL;

	return S_OK;
}

void CRenderer::Draw()
{
	/* NumPad 1 : FXAA 전환 */
	if (m_pGameInstance->Get_DIKeyDown(DIK_NUMPAD1))
		m_bFXAAEnabled = !m_bFXAAEnabled;

	Render_Shadow();

	ID3D11ShaderResourceView* pNullSRV[16] = {};
	m_pDeviceContext->PSSetShaderResources(0, 16, pNullSRV);

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Scene"))))
		return;

	Render_NonBlend();
	Render_LightAcc();
	Render_Priority();
	Render_CombinedRT();
	Render_NonLight();
	Render_Blend();

	if (FAILED(m_pGameInstance->End_MRT()))
		return;

	Render_FXAA();
	Render_UI();

#ifdef _DEBUG
	Render_Debug();
#endif

	Render_Fonts(m_pDeviceContext);
}

HRESULT CRenderer::Add_RenderGroup(RENDERGROUP _eGroupID, CGameObject* _pRenderObject)
{
	if (_eGroupID >= RENDERGROUP::END)
		return E_FAIL;

	m_RenderObjectList[ENUM_TO_UINT(_eGroupID)].push_back(_pRenderObject);
	Safe_AddRef(_pRenderObject);

	return S_OK;
}

void CRenderer::Add_Instance(CModel* _pModel, const _float4x4& _matWorld)
{
	Add_Instance(_pModel, nullptr, _matWorld);
}

void CRenderer::Add_Instance(CModel* _pModel, CShader* _pShader, const _float4x4& _matWorld)
{
	if (!m_bInstancingEnabled)
		return;

	if (nullptr == _pModel || nullptr == _pShader)
		return;

	if (0 == _pModel->Get_NumMeshes())
		return;

	CMesh* pBatchKey = _pModel->Get_Mesh(0);
	if (nullptr == pBatchKey)
		return;

	INSTANCE_BATCH& Batch = m_InstanceBatches[pBatchKey];

	if (Batch.WorldMatrices.empty())
	{
		Batch.pRepresentativeModel = _pModel;
		Batch.pShader = _pShader;
	}

	Batch.WorldMatrices.push_back(_matWorld);
}

void CRenderer::Set_InstnacingEnabled(_bool _bEnabled)
{
	m_bInstancingEnabled = _bEnabled;
	Clear_InstanceBatches(); // Clear cached batches when toggling instancing.
}

_bool CRenderer::Is_InstancingEnabled() const
{
	return m_bInstancingEnabled;
}

void CRenderer::Render_Priority()
{
	for (auto& pRenderObject : m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::PRIORITY)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}
	m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::PRIORITY)].clear();
}

void CRenderer::Render_Shadow()
{
	if (!m_pShadowDSV)
		return;

	/* Unbind the shadow SRV from the previous frame. */
	ID3D11ShaderResourceView* nullSRV[16] = {};
	m_pDeviceContext->PSSetShaderResources(0, 16, nullSRV);
	m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthEnableState, 0);

	m_pDeviceContext->ClearDepthStencilView(m_pShadowDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Shadow"), m_pShadowDSV)))
		return;

	D3D11_VIEWPORT	ViewPortDesc;
	ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
	ViewPortDesc.TopLeftX = 0;
	ViewPortDesc.TopLeftY = 0;
	ViewPortDesc.Width = (_float)g_iMaxWidth;
	ViewPortDesc.Height = (_float)g_iMaxHeight;
	ViewPortDesc.MinDepth = 0.f;
	ViewPortDesc.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &ViewPortDesc);

	for (auto& pRenderObject : m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::SHADOW)])
	{
		if (pRenderObject)
			pRenderObject->Render_Shadow();

		Safe_Release(pRenderObject);
	}

	m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::SHADOW)].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return;

	m_pDeviceContext->RSSetViewports(1, &m_OriginalViewport);
}

void CRenderer::Render_NonBlend()
{
	/* SRV null (harzard) */
	ID3D11ShaderResourceView* nullSRV[16] = { nullptr };
	m_pDeviceContext->PSSetShaderResources(0, 16, nullSRV);

	/* Deferred Shade (Diffuse + Normal) */
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_GameObject"))))
		return;

	/* Blend OFF, Depth ON */
	m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthEnableState, 0);

	for (auto& pRenderObject : m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::NONBLEND)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}
	m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::NONBLEND)].clear();

	/* Instancing */
	if (m_bInstancingEnabled)
		Render_Instancing();
	else
		Clear_InstanceBatches();

	/* Deffered Shade */
	if (FAILED(m_pGameInstance->End_MRT()))
		return;
}

void CRenderer::Render_NonLight()
{
	/* Blend ON, Depth OFF */
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthEnableState, 0);
	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState, nullptr, 0xffffffff);

	for (auto& pRenderObject : m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::NONLIGHT)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}
	m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::NONLIGHT)].clear();
}

void CRenderer::Render_Blend()
{
	/* Blend ON, Depth ON */
	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState, nullptr, 0xffffffff);
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthEnableState, 0);

	for (auto& pRenderObject : m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::BLEND)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}
	m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::BLEND)].clear();
}

void CRenderer::Render_UI()
{
	auto& renderList = m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::UI)];

	renderList.sort(
		[](CGameObject* a, CGameObject* b)
		{
			return static_cast<CUIObject*>(a)->Get_UI_Layer()
				< static_cast<CUIObject*>(b)->Get_UI_Layer();
		}
	);

	/* Blend ON, Depth OFF */
	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState, nullptr, 0xffffffff);
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthDisableState, 0);

	for (auto& pRenderObject : m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::UI)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}
	m_RenderObjectList[ENUM_TO_UINT(RENDERGROUP::UI)].clear();
}

void CRenderer::Render_CombinedRT()
{
	/* Blend ON, Depth OFF */
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthDisableState, 0);
	m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	if (FAILED(m_pGameInstance->Bind_RT_SRV(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return;
	if (FAILED(m_pGameInstance->Bind_RT_SRV(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return;
	if (FAILED(m_pGameInstance->Bind_RT_SRV(TEXT("Target_Material"), m_pShader, "g_MaterialTexture")))
		return;
	if (FAILED(m_pGameInstance->Bind_RT_SRV(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return;
	if (FAILED(m_pGameInstance->Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return;
	if (FAILED(m_pGameInstance->Bind_RT_SRV(TEXT("Target_Shadow"), m_pShader, "g_ShadowTexture")))
		return;

	/* PCF */
	const _float2 vShadowTexelSize = { 1.f / static_cast<_float>(g_iMaxWidth), 1.f / static_cast<_float>(g_iMaxHeight) };
	if (FAILED(m_pShader->Bind_RawValue("g_vShadowTexelSize", &vShadowTexelSize, sizeof(_float2))))
		return;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return;

	if (FAILED(m_pGameInstance->Bind_InvTransformState(m_pShader, "g_ViewMatrixInv", D3DTS::VIEW)))
		return;
	if (FAILED(m_pGameInstance->Bind_InvTransformState(m_pShader, "g_ProjMatrixInv", D3DTS::PROJ)))
		return;

	if (FAILED(m_pGameInstance->Bind_ShadowTransformState(m_pShader, "g_LightViewMatrix", D3DTS::VIEW)))
		return;
	if (FAILED(m_pGameInstance->Bind_ShadowTransformState(m_pShader, "g_LightProjMatrix", D3DTS::PROJ)))
		return;

	if (FAILED(m_pShader->Begin(ENUM_TO_UINT(DEFERRED::COMBINED))))
		return;
	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return;
	if (FAILED(m_pVIBuffer->Render()))
		return;
}

HRESULT CRenderer::Render_ShadowDepthStencilView(_uint _iWidth, _uint _iHeight)
{
	ID3D11Texture2D* pDepthStencilTexture = { nullptr };

	D3D11_TEXTURE2D_DESC	TextureDesc{};
	TextureDesc.Width = _iWidth;
	TextureDesc.Height = _iHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
		return E_FAIL;

	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pShadowDSV)))
		return E_FAIL;

	Safe_Release(pDepthStencilTexture);
	
	return S_OK;
}

#ifdef _DEBUG
void CRenderer::Render_Debug()
{
	/* Blend ON, Depth OFF */
	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState, nullptr, 0xffffffff);
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthDisableState, 0);

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return;

	if (FAILED(m_pGameInstance->Render_MRT(TEXT("MRT_GameObject"), m_pShader, m_pVIBuffer)))
		return;
	if (FAILED(m_pGameInstance->Render_MRT(TEXT("MRT_LightAcc"), m_pShader, m_pVIBuffer)))
		return;
	if (FAILED(m_pGameInstance->Render_MRT(TEXT("MRT_Shadow"), m_pShader, m_pVIBuffer)))
		return;
	if (FAILED(m_pGameInstance->Render_MRT(TEXT("MRT_Scene"), m_pShader, m_pVIBuffer)))
		return;

	for (auto& pDebugComponent : m_DebugComponent)
	{
		if (nullptr != pDebugComponent)
			pDebugComponent->Render();
		Safe_Release(pDebugComponent);
	}
	m_DebugComponent.clear();
}

HRESULT CRenderer::Add_DebugComponent(CComponent* _pDebugComponent)
{
	Safe_AddRef(_pDebugComponent);

	m_DebugComponent.push_back(_pDebugComponent);

	return S_OK;
}
#endif

void CRenderer::Render_LightAcc()
{
	/* Blend ON, Depth OFF */
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthDisableState, 0);
	m_pDeviceContext->OMSetBlendState(m_pLightBlendState, nullptr, 0xffffffff);

	/* SRV null (harzard) */
	ID3D11ShaderResourceView* nullSRV[16] = { nullptr };
	m_pDeviceContext->PSSetShaderResources(0, 16, nullSRV);

	/* Shade */
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_LightAcc"))))
		return;

	if (FAILED(m_pGameInstance->Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")))
		return;
	if (FAILED(m_pGameInstance->Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return;
	if (FAILED(m_pGameInstance->Bind_RT_SRV(TEXT("Target_Material"), m_pShader, "g_MaterialTexture")))
		return;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return;

	if (FAILED(m_pGameInstance->Bind_InvTransformState(m_pShader, "g_ViewMatrixInv", D3DTS::VIEW)))
		return;
	if (FAILED(m_pGameInstance->Bind_InvTransformState(m_pShader, "g_ProjMatrixInv", D3DTS::PROJ)))
		return;

	if (FAILED(m_pGameInstance->Bind_CameraPosition(m_pShader, "g_vCameraPosition")))
		return;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return;

	m_pGameInstance->Render_Light(m_pShader, m_pVIBuffer);

	if (FAILED(m_pGameInstance->End_MRT()))
		return;
}

void CRenderer::Render_Fonts(ID3D11DeviceContext* _pDeviceContext)
{
	m_pGameInstance->Render_Fonts(m_pDeviceContext);
}

void CRenderer::Render_Instancing()
{
	for (auto& Pair : m_InstanceBatches)
	{
		INSTANCE_BATCH& Batch = Pair.second;

		if (Batch.WorldMatrices.empty())
			continue;

		if (nullptr == Batch.pRepresentativeModel || nullptr == Batch.pShader)
			continue;

		if (FAILED(Render_InstanceBatch(Batch)))
			continue;
	}

	Clear_InstanceBatches();
}

void CRenderer::Clear_InstanceBatches()
{
	for (auto& Pair : m_InstanceBatches)
	{
		INSTANCE_BATCH& Batch = Pair.second;

		Batch.pRepresentativeModel = nullptr;
		Batch.pShader = nullptr;
		Batch.WorldMatrices.clear();
	}
}

HRESULT CRenderer::Render_InstanceBatch(INSTANCE_BATCH& _tBatch)
{
	CModel* pModel = _tBatch.pRepresentativeModel;
	CShader* pShader = _tBatch.pShader;

	if (nullptr == pModel || nullptr == pShader)
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_TransformState(pShader, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_TransformState(pShader, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;

	const size_t iTotalInstanceCount = _tBatch.WorldMatrices.size();

	for (size_t iStart = 0; iStart < iTotalInstanceCount; iStart += INSTANCE_CAPACITY)
	{
		const size_t iRemainCount = iTotalInstanceCount - iStart;
		const _uint iDrawInstanceCount = static_cast<_uint>(
				min(iRemainCount, static_cast<size_t>(INSTANCE_CAPACITY)));
		

		D3D11_MAPPED_SUBRESOURCE MappedResource = {};

		if (FAILED(m_pDeviceContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
			return E_FAIL;

		memcpy(MappedResource.pData, _tBatch.WorldMatrices.data() + iStart, sizeof(_float4x4) * iDrawInstanceCount);

		m_pDeviceContext->Unmap(m_pVBInstance, 0);

		const _uint iNumMeshes = pModel->Get_NumMeshes();

		for (_uint iMeshIndex = 0; iMeshIndex < iNumMeshes; ++iMeshIndex)
		{
			CMesh* pMesh = pModel->Get_Mesh(iMeshIndex);

			if (nullptr == pMesh)
				continue;

			/* Material */
			if (FAILED(pModel->Bind_Material( pShader, "g_DiffuseTexture", iMeshIndex, MATERIAL::DIFFUSE)))
				return E_FAIL;

			if (FAILED(pModel->Bind_Material( pShader, "g_NormalTexture", iMeshIndex, MATERIAL::NORMALS)))
				return E_FAIL;

			if (FAILED(pModel->Bind_Material(pShader, "g_AOTexture", iMeshIndex, MATERIAL::AO)))
				return E_FAIL;

			if (FAILED(pModel->Bind_Material(pShader, "g_MTLTexture", iMeshIndex, MATERIAL::METALLIC)))
				return E_FAIL;

			if (FAILED(pModel->Bind_Material(pShader, "g_SMTTexture", iMeshIndex, MATERIAL::ROUGHNESS)))
				return E_FAIL;

			if (FAILED(pShader->Begin(0)))
				return E_FAIL;

			if (FAILED(pModel->Bind_Resources(iMeshIndex)))
				return E_FAIL;

			ID3D11Buffer* pVertexBuffers[2] = { pMesh->Get_VertexBuffer(), m_pVBInstance };

			const _uint iStrides[2] = { pMesh->Get_VertexStride(), sizeof(_float4x4) };
			const _uint iOffsets[2] = { 0, 0 };

			m_pDeviceContext->IASetVertexBuffers(0, 2, pVertexBuffers, iStrides, iOffsets);
			m_pDeviceContext->DrawIndexedInstanced(pModel->Get_IndexCount(iMeshIndex), iDrawInstanceCount, 0, 0, 0);
		}
	}

	return S_OK;
}

void CRenderer::Render_FXAA()
{
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthDisableState, 0);
	m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	if (FAILED(m_pGameInstance->Bind_RT_SRV(TEXT("Target_Scene"), m_pShader, "g_Texture")))
		return;

	const _float2 vInverseScreenSize = { 1.f / m_OriginalViewport.Width, 1.f / m_OriginalViewport.Height };

	if (FAILED(m_pShader->Bind_RawValue("g_vInverseScreenSize", &vInverseScreenSize, sizeof(_float2))))
		return;

	/* HLSL uint와 동일한 크기로 FXAA 활성 상태 전달 */
	const _uint iUseFXAA = m_bFXAAEnabled ? 1u : 0u;
	if (FAILED(m_pShader->Bind_RawValue("g_iUseFXAA", &iUseFXAA, sizeof(_uint))))
		return;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return;

	constexpr _uint iFXAAPass = 4;
	if (FAILED(m_pShader->Begin(iFXAAPass)))
		return;
	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return;
	if (FAILED(m_pVIBuffer->Render()))
		return;

	m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

HRESULT CRenderer::Ready_DeferredResources()
{
	_uint iNumViewports = { 1 };
	D3D11_VIEWPORT ViewportDesc = {};
	m_pDeviceContext->RSGetViewports(&iNumViewports, &ViewportDesc);
	m_OriginalViewport = ViewportDesc;

	/* 1. RenderTarget */
	if (FAILED(m_pGameInstance->Add_RenderTarget(
		TEXT("Target_Diffuse"),
		ViewportDesc.Width,
		ViewportDesc.Height,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		_float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(
		TEXT("Target_Normal"),
		ViewportDesc.Width,
		ViewportDesc.Height,
		DXGI_FORMAT_R16G16B16A16_UNORM,
		_float4(0.f, 0.f, 0.f, 1.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(
		TEXT("Target_Material"),
		ViewportDesc.Width,
		ViewportDesc.Height,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		_float4(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(
		TEXT("Target_Depth"),
		ViewportDesc.Width,
		ViewportDesc.Height,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		_float4(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(
		TEXT("Target_Shade"),
		ViewportDesc.Width,
		ViewportDesc.Height,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		_float4(0.f, 0.f, 0.f, 1.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(
		TEXT("Target_Specular"),
		ViewportDesc.Width,
		ViewportDesc.Height,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		_float4(0.f, 0.f, 0.f, 1.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(
		TEXT("Target_Shadow"),
		g_iMaxWidth,
		g_iMaxHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		_float4(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(
		TEXT("Target_Scene"),
		ViewportDesc.Width,
		ViewportDesc.Height,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		_float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Shadow DSV */
	if (FAILED(Render_ShadowDepthStencilView(g_iMaxWidth, g_iMaxHeight)))
		return E_FAIL;

	/* 2. MRT*/
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Normal"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Depth"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Material"))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Shade"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Specular"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Shadow"), TEXT("Target_Shadow"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Scene"), TEXT("Target_Scene"))))
		return E_FAIL;

	/* 3. Shader, VIBuffer */
	m_pShader = CShader::Create(
		m_pDevice, m_pDeviceContext, 
		TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"),
		VTXPOSTEX::Elements,
		VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pDeviceContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	/* 4. (Ortho) View, Proj */
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.f, 1.f));

#ifdef  _DEBUG
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Diffuse"), 80.f, 70.f, 130.f, 130.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Normal"), 80.f, 210.f, 130.f, 130.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Shade"), 80.f, 350.f, 130.f, 130.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Material"), 80.f, 490.f, 130.f, 130.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Specular"), 80.f, 630.f, 130.f, 130.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Depth"), 1200.f, 70.f, 130.f, 130.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Shadow"), 1200.f, 210.f, 130.f, 130.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Scene"), 1200.f, 350.f, 130.f, 130.f)))
		return E_FAIL;
#endif //  _DEBUG

	return S_OK;
}

CRenderer* CRenderer::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CRenderer* pInstance = new CRenderer(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("FAILED TO CREATED : CRenderer");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CRenderer::Free()
{
	__super::Free();

	/* list release */
	for (size_t i = 0; i < ENUM_TO_UINT(RENDERGROUP::END); i++)
	{
		for (auto& pRenderObject : m_RenderObjectList[i])
			Safe_Release(pRenderObject);
		m_RenderObjectList[i].clear();
	}

	Safe_Release(m_pShadowDSV);

	Safe_Release(m_pVBInstance);

	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);

	Safe_Release(m_pLightBlendState);
	Safe_Release(m_pDepthDisableState);
	Safe_Release(m_pDepthEnableState);
	Safe_Release(m_pAlphaBlendState);

	Safe_Release(m_pDeviceContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
