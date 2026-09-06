#include "TrailEffect.h"
#include "GameInstance.h"

CTrailEffect::CTrailEffect(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CMapObject{ _pDevice, _pDeviceContext }
{
}

CTrailEffect::CTrailEffect(const CTrailEffect& _Prototype)
	: CMapObject(_Prototype)
{
}

HRESULT CTrailEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrailEffect::Initialize(void* _pArg)
{
	/* 1. Map Object */
	if (FAILED(CMapObject::Initialize(_pArg)))
		return E_FAIL;

	/* 2. MAP_INIT_DESC */
	TRAIL_INIT_DESC* pInitDesc = static_cast<TRAIL_INIT_DESC*>(_pArg);
	if (FAILED(Ready_Components(&pInitDesc->tObjectDesc)))
		return E_FAIL;

	Set_TextureSlots(2, 0);
	//Set_TextureSlots(3, 0);
	Set_EffectPassIndex(TRAIL_EFFECT_TYPE::SLASH);
	
	/* Trail */
	m_fWidth = pInitDesc->fWidth;
	if (pInitDesc->iMaxPoint == 0)
		m_iMaxPoint = 32;
	else
		m_iMaxPoint = pInitDesc->iMaxPoint;

	/* ³Ñ°ÜÁà¾ßÇÔ */
	if (m_pVIBufferCom)
	{
		m_pVIBufferCom->Set_MaxPoint(m_iMaxPoint);
	}

	m_pParentMatrix = pInitDesc->pParentMatrix;
	m_pBaseSocketMatrix = pInitDesc->pBaseSocketMatrix;
	m_pTipSocketMatrix = pInitDesc->pTipSocketMatrix;
	m_tTime.fAccDurationTime = pInitDesc->fTrailDuration;

	return S_OK;
}

void CTrailEffect::Priority_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;
}

void CTrailEffect::Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	if (!m_pParentMatrix)
		return;

	m_tShaderDesc.fTime += _fTimeDelta;

	_matrix parent = XMLoadFloat4x4(m_pParentMatrix);
	_vector vtip = XMVector3TransformCoord(XMVectorZero(), XMLoadFloat4x4(m_pTipSocketMatrix) * parent);
	_vector vbase = XMVector3TransformCoord(XMVectorZero(), XMLoadFloat4x4(m_pBaseSocketMatrix) * parent);
	Add_TipBasePoint(vtip, vbase);

	if (m_pVIBufferCom)
		m_pVIBufferCom->Update_Trail(m_TrailTips, m_TrailBases);
	
	/* Timeline */
	m_tTime.fAccTime += _fTimeDelta;
	if (m_tTime.fAccTime >= m_tTime.fAccDurationTime)
	{
		if (!m_bDelete)
		{
			m_pGameInstance->Reserve_DeleteObject(this);
			m_bDelete = true;
		}
		m_bActive = false;
	}
}

void CTrailEffect::Late_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	if (m_TrailTips.size() < 2)
		return;

	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONLIGHT, this)))
		return;
}

HRESULT CTrailEffect::Render()
{
	if (!m_bActive)
		return S_OK;

	/* Transform */
	if (FAILED(CGameObject::m_pTransformCom->Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	/* Perspective */
	if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;

	/* Texture */
	if (m_pMainTexture)
		m_pMainTexture->Bind_ShaderResourceView(m_pShaderCom, "g_MainTexture", m_iMainTexSlot);
	if (m_pDistortionTexture)
		m_pDistortionTexture->Bind_ShaderResourceView(m_pShaderCom, "g_DistortionTexture", m_iDistortionSlot);

	/* Time */
	m_pShaderCom->Bind_RawValue("g_Time", &m_tShaderDesc.fTime, sizeof(_float));

	/* Shader */
	if (FAILED(m_pShaderCom->Begin(m_iEffectPassIndex)))
		return E_FAIL;

	/* VIBuffer */
	if (FAILED(m_pVIBufferCom->Bind_Resources()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CTrailEffect::Add_Point(_vector _vPos)
{
	if (!m_TrailPoints.empty())
	{
		_vector last = m_TrailPoints.back();
		_float dist = XMVectorGetX(XMVector3Length(_vPos - last));
		if (dist < 0.02f)
			return;
	}
	m_TrailPoints.push_back(_vPos);
}

void CTrailEffect::Add_TipBasePoint(_vector _vTip, _vector _vBase)
{
	if (!m_TrailTips.empty())
	{
		_float dist = XMVectorGetX(XMVector3Length(_vTip - m_TrailTips.back()));
		if (dist < 0.02f)
			return;
	}

	m_TrailTips.push_back(_vTip);
	m_TrailBases.push_back(_vBase);

	if (m_TrailTips.size() > m_iMaxPoint)
	{
		m_TrailTips.erase(m_TrailTips.begin());
		m_TrailBases.erase(m_TrailBases.begin());
	}
}

void CTrailEffect::Set_TextureSlots(_uint main, _uint distortion)
{
	m_iMainTexSlot = main;
	m_iDistortionSlot = distortion;
}

void CTrailEffect::Set_EffectPassIndex(TRAIL_EFFECT_TYPE _eEffectType)
{
	m_iEffectPassIndex = ENUM_TO_UINT(_eEffectType);
}

HRESULT CTrailEffect::Ready_Components(OBJECT_DESC* _pTags)
{
	/* Com_Shader */
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		_pTags->ShaderTag,
		TEXT("Com_Shader"),
		reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_VIBuffer */
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		_pTags->VIBufferTag,
		TEXT("Com_VIBufferCom"),
		reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	/* Com_MainTexture */
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Texture_Slash"),
		TEXT("Com_MainTexture"),
		reinterpret_cast<CComponent**>(&m_pMainTexture))))
		return E_FAIL;

	/* Com_DistortionTexture */
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Texture_Noise"),
		TEXT("Com_DistortionTexture"),
		reinterpret_cast<CComponent**>(&m_pDistortionTexture))))
		return E_FAIL;

	return S_OK;
}

CTrailEffect* CTrailEffect::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CTrailEffect* pInstance = new CTrailEffect(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("FAILED TO CREATED : CTrailEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTrailEffect::Clone(void* _pArg)
{
	CTrailEffect* pInstance = new CTrailEffect(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("FAILED TO CLONED : CTrailEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTrailEffect::Free()
{
	__super::Free();

	Safe_Release(m_pDistortionTexture);
	Safe_Release(m_pMainTexture);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
}
