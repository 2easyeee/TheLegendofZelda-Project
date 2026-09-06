#include "Light.h"
#include "GameInstance.h"

CLight::CLight()
{
}

HRESULT CLight::Initialize(const LIGHT_DESC& _tLightDesc)
{
	m_tLightDesc = _tLightDesc;

	return S_OK;
}

void CLight::Update(_float _fTimeDelta)
{
	if (m_tLightDesc.eUsage == LIGHT_USAGE::DYNAMIC)
	{
		if (m_pTarget)
		{
			_vector vPos = m_pTarget->Get_State(STATE::POSITION);
			_vector vFinal = XMVectorAdd(vPos, m_vOffset);
			XMStoreFloat4(&m_tLightDesc.vPosition, vFinal);
			m_tLightDesc.vPosition.w = 1.f;
		}
	}
}

void CLight::Render(CShader* _pShader, CVIBuffer_Rect* _pVIBuffer)
{
	DEFERRED ePassIndex = { DEFERRED::END };
	switch (m_tLightDesc.eType)
	{
	case LIGHT::DIRECTIONAL:
	{
		if (FAILED(_pShader->Bind_RawValue("g_vLightDirectional", &m_tLightDesc.vDirection, sizeof(_float4))))
			return;
		ePassIndex = DEFERRED::DIRECTIONAL;
	}
	break;
	case LIGHT::POINT:
	{
		if (FAILED(_pShader->Bind_RawValue("g_vLightPos", &m_tLightDesc.vPosition, sizeof(_float4))))
			return;
		if (FAILED(_pShader->Bind_RawValue("g_fLightRange", &m_tLightDesc.fRange, sizeof(_float))))
			return;
		if (FAILED(_pShader->Bind_RawValue("g_fLightIntensity", &m_tLightDesc.fIntensity, sizeof(_float))))
			return;
		ePassIndex = DEFERRED::POINT;
	}
	break;	
	}

	if (FAILED(_pShader->Bind_RawValue("g_vLightDiffuse", &m_tLightDesc.vDiffuse, sizeof(_float4))))
		return;
	if (FAILED(_pShader->Bind_RawValue("g_vLightAmbient", &m_tLightDesc.vAmbient, sizeof(_float4))))
		return;
	if (FAILED(_pShader->Bind_RawValue("g_vLightSpecular", &m_tLightDesc.vSpecular, sizeof(_float4))))
		return;

	if (FAILED(_pShader->Begin(ENUM_TO_UINT(ePassIndex))))
		return;
	if (FAILED(_pVIBuffer->Render()))
		return;
}

const LIGHT_DESC* CLight::Get_Light()
{
	return &m_tLightDesc;
}

void CLight::Set_Target(CTransform* _pTarget)
{
	m_pTarget = _pTarget;
}

void CLight::Set_Offset(_vector _vOffset)
{
	m_vOffset = _vOffset;
}

void CLight::Set_Position(_vector _vPos)
{
	XMStoreFloat4(&m_tLightDesc.vPosition , _vPos);
}

CLight* CLight::Create(const LIGHT_DESC& _tLightDesc)
{
	CLight* pInstance = new CLight();
	if (FAILED(pInstance->Initialize(_tLightDesc)))
	{
		MSG_BOX("Failed to Created : CLight");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CLight::Free()
{
	__super::Free();
}
