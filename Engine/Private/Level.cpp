#include "Level.h"
#include "GameInstance.h"

CLevel::CLevel(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: m_pGameInstance { CGameInstance::GetInstance() }
	, m_pDevice { _pDevice }
	, m_pDeviceContext { _pDeviceContext }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDeviceContext);
	Safe_AddRef(m_pDevice);
}

HRESULT CLevel::Initialize()
{
	return S_OK;
}

void CLevel::Update(_float fTimeDelta)
{
}

HRESULT CLevel::Render()
{
	return S_OK;
}

HRESULT CLevel::Ready_Layer_Shadow()
{
	SHADOW_LIGHT_DESC ShadowLightDesc = {};
	ShadowLightDesc.vEye = _float4(30.652f, 20.f, -17.58f, 1.f);  // _float4(0.f, 20.f, -15.f, 1.f);
	ShadowLightDesc.vAt = _float4(30.652f, 0.f, -2.58f, 1.f);  // _float4(0.f, 0.f, 0.f, 1.f);
	ShadowLightDesc.fFovy = XMConvertToRadians(90.f);
	ShadowLightDesc.fNear = 0.1f;
	ShadowLightDesc.fFar = 1000.f;
	ShadowLightDesc.fAspect = static_cast<_float>(g_iMaxWidth) / static_cast<_float>(g_iMaxHeight);

	if (FAILED(m_pGameInstance->Add_ShadowLight(ShadowLightDesc)))
		return E_FAIL;

	return S_OK;
}

void CLevel::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pDeviceContext);
	Safe_Release(m_pGameInstance);
}
