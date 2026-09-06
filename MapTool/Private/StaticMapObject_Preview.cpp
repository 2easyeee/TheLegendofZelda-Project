#include "StaticMapObject_Preview.h"
#include "GameInstance.h"

CStaticMapObject_Preview::CStaticMapObject_Preview(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CMapObject{ _pDevice, _pDeviceContext }
{
}

CStaticMapObject_Preview::CStaticMapObject_Preview(const CStaticMapObject_Preview& _Prototype)
    : CMapObject(_Prototype)
{
}

HRESULT CStaticMapObject_Preview::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CStaticMapObject_Preview::Initialize(void* _pArg)
{
    if (FAILED(CMapObject::Initialize(_pArg)))
        return E_FAIL;

    MAP_INIT_DESC* pInitDesc = static_cast<MAP_INIT_DESC*>(_pArg);
    if (&pInitDesc->tObjectDesc)
    {
        if (FAILED(Ready_Components(&pInitDesc->tObjectDesc)))
            return E_FAIL;
    }

    return S_OK;
}

void CStaticMapObject_Preview::Priority_Update(_float _fTimeDelta)
{
}

void CStaticMapObject_Preview::Update(_float _fTimeDelta)
{
}

void CStaticMapObject_Preview::Late_Update(_float _fTimeDelta)
{
    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLEND, this)))
        return;
}

HRESULT CStaticMapObject_Preview::Render()
{
    /* Transform */
    if (FAILED(m_pTransformCom->Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    /* Perspective */
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;

    /* Model */
    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();
    for (size_t i = 0; i < iNumMeshes; i++)
    {
        /* Material */
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MATERIAL::DIFFUSE)))
            return E_FAIL;
        /* Shader */
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        /* Mesh */
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CStaticMapObject_Preview::Ready_Components(OBJECT_DESC* _Desc)
{
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        _Desc->ShaderTag,
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    if (FAILED(Add_Component(
        _Desc->iLevel,
        _Desc->ModelTag,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
        return E_FAIL;
}

CStaticMapObject_Preview* CStaticMapObject_Preview::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CStaticMapObject_Preview* pInstance = new CStaticMapObject_Preview(_pDevice, _pDeviceContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CStaticMapObject_Preview");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CStaticMapObject_Preview::Clone(void* _pArg)
{
    CStaticMapObject_Preview* pInstance = new CStaticMapObject_Preview(*this);

    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("Failed to Cloned : CStaticMapObject_Preview");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStaticMapObject_Preview::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}