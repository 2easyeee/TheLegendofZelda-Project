#include "GameObject/MasterStalfon_ShadowIndicator.h"
#include "GameInstance.h"

CMasterStalfon_ShadowIndicator::CMasterStalfon_ShadowIndicator(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CMapObject { _pDevice, _pDeviceContext }
{
}

CMasterStalfon_ShadowIndicator::CMasterStalfon_ShadowIndicator(const CMasterStalfon_ShadowIndicator& _Prototype)
    : CMapObject (_Prototype)
{
}

HRESULT CMasterStalfon_ShadowIndicator::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMasterStalfon_ShadowIndicator::Initialize(void* _pArg)
{
    /* 1. Map Object */
    if (FAILED(CMapObject::Initialize(_pArg)))
        return E_FAIL;

    /* 2. MAP_INIT_DESC */
    MAP_INIT_DESC* pInitDesc = static_cast<MAP_INIT_DESC*>(_pArg);
    if (&pInitDesc->tObjectDesc)
    {
        if (FAILED(Ready_Components(&pInitDesc->tObjectDesc)))
            return E_FAIL;
    }

    /* Shadow */
    m_fScale = 0.2f;
    Set_Scale(m_fScale);

    return S_OK;
}

void CMasterStalfon_ShadowIndicator::Priority_Update(_float _fTimeDelta)
{
}

void CMasterStalfon_ShadowIndicator::Update(_float _fTimeDelta)
{
    m_fScale += _fTimeDelta * 3.f;
    m_fScale = min(m_fScale, m_fMaxScale);

    Set_Scale(m_fScale);
}

void CMasterStalfon_ShadowIndicator::Late_Update(_float _fTimeDelta)
{
    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLEND, this)))
        return;
}

HRESULT CMasterStalfon_ShadowIndicator::Render()
{
    /* Transform */
    if (FAILED(CGameObject::m_pTransformCom->Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    /* Perspective */
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;

    /* Texture */
    if (FAILED(m_pTextureCom->Bind_ShaderResourceView(m_pShaderCom, "g_Texture", 0)))
        return E_FAIL;

    /* Color */
    _float4 vColor = { 0.f, 0.f, 0.f, 0.6f };
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &vColor, sizeof(_float4))))
        return E_FAIL;

    /* Shader */
    if (FAILED(m_pShaderCom->Begin(1)))
        return E_FAIL;

    /* VIBuffer */
    if (FAILED(m_pVIBufferCom->Bind_Resources()))
        return E_FAIL;
    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;

    return S_OK;
}

HRESULT CMasterStalfon_ShadowIndicator::Ready_Components(OBJECT_DESC* _Desc)
{
    /* Com_Shader */
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Shader_VtxPosTex"),
        TEXT("Com_Shader"),
        reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    /* Com_VIBuffer */
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_VIBuffer_Plane"),
        TEXT("Com_VIBuffer"),
        reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
        return E_FAIL;

    /* Com_Texture */
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Texture_Map_Shadow"),
        TEXT("Com_Texture"),
        reinterpret_cast<CComponent**>(&m_pTextureCom))))
        return E_FAIL;

    return S_OK;
}

void CMasterStalfon_ShadowIndicator::Set_Position(_vector _vPos)
{
    _vPos = XMVectorSetY(_vPos, 0.01f);
    m_pTransformCom->Set_State(STATE::POSITION, _vPos);
}

void CMasterStalfon_ShadowIndicator::Set_Scale(_float _fScale)
{
    m_pTransformCom->Set_Scale(_fScale, 1.f, _fScale);
}

void CMasterStalfon_ShadowIndicator::Destroy()
{
    m_pGameInstance->Reserve_DeleteObject(this);
}

CMasterStalfon_ShadowIndicator* CMasterStalfon_ShadowIndicator::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CMasterStalfon_ShadowIndicator* pInstance = new CMasterStalfon_ShadowIndicator(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CMasterStalfon_ShadowIndicator");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CMasterStalfon_ShadowIndicator::Clone(void* _pArg)
{
    CMasterStalfon_ShadowIndicator* pInstance = new CMasterStalfon_ShadowIndicator(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CMasterStalfon_ShadowIndicator");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMasterStalfon_ShadowIndicator::Free()
{
    __super::Free();

    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);
}
