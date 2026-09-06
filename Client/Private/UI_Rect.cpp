#include "UI_Rect.h"
#include "GameInstance.h"

CUI_Rect::CUI_Rect(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CUIObject { _pDevice, _pDeviceContext }
{
}

CUI_Rect::CUI_Rect(const CUI_Rect& _Prototype)
	: CUIObject (_Prototype)
{
}

HRESULT CUI_Rect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Rect::Initialize(void* _pArg)
{
    if (FAILED(__super::Initialize(_pArg)))
        return E_FAIL;

    UI_INIT_DESC* pInitDesc = static_cast<UI_INIT_DESC*>(_pArg);
    if (&pInitDesc->tObjectDesc)
    {
        if (FAILED(Ready_Components(&pInitDesc->tObjectDesc)))
            return E_FAIL;
    }

	return S_OK;
}

void CUI_Rect::Priority_Update(_float _fTimeDelta)
{
}

void CUI_Rect::Update(_float _fTimeDelta)
{
}

void CUI_Rect::Late_Update(_float _fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::UI, this)))
		return;
}

HRESULT CUI_Rect::Render()
{
    if (!m_bActive)
        return S_OK;

    /* Transform */
    if (FAILED(CGameObject::m_pTransformCom->Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    /* Ortho */
    if (FAILED(CUIObject::Bind_ViewMatrix(m_pShaderCom, "g_ViewMatrix")))
        return E_FAIL;
    if (FAILED(CUIObject::Bind_ProjMatrix(m_pShaderCom, "g_ProjMatrix")))
        return E_FAIL;

    /* Texture */
    if (FAILED(m_pTextureCom->Bind_ShaderResourceView(m_pShaderCom, "g_Texture", m_iTextureIndex)))
        return E_FAIL;

    /* Shader */
    _float4 vColor = { 1.f, 1.f, 1.f, m_fAlpha };
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &vColor, sizeof(_float4))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Begin(ENUM_TO_UINT(m_eUIBlend))))
        return E_FAIL;

    /* VIBuffer */
    if (FAILED(m_pVIBufferCom->Bind_Resources()))
        return E_FAIL;
    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;

	return S_OK;
}

HRESULT CUI_Rect::Ready_Components(OBJECT_DESC* _pDesc)
{
    /* Com_Shader */
    if (FAILED(Add_Component(
        ENUM_TO_UINT(LEVEL::STATIC),
        _pDesc->ShaderTag,
        TEXT("Com_Shader"),
        reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    /* Com_VIBuffer */
    if (FAILED(Add_Component(
        ENUM_TO_UINT(LEVEL::STATIC),
        _pDesc->VIBufferTag,
        TEXT("Com_VIBuffer"),
        reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
        return E_FAIL;

    /* Com_Texture */
    if (FAILED(Add_Component(
        _pDesc->iLevel,
        _pDesc->TextureTag,
        TEXT("Com_Texture"),
        reinterpret_cast<CComponent**>(&m_pTextureCom))))
        return E_FAIL;

	return S_OK;
}

void CUI_Rect::Free()
{
	__super::Free();

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
}
