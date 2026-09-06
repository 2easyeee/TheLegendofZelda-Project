#include "GameObject/ColorToggleBlock.h"
#include "GameInstance.h"

/* STATIC */
CColorToggleBlock::ACTIVE_BLOCK CColorToggleBlock::s_eState = ACTIVE_BLOCK::BLUE;

CColorToggleBlock::CColorToggleBlock(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CMapObject{ _pDevice, _pDeviceContext }
{
}

CColorToggleBlock::CColorToggleBlock(const CColorToggleBlock& _Prototype)
    : CMapObject(_Prototype)
{
}

HRESULT CColorToggleBlock::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CColorToggleBlock::Initialize(void* _pArg)
{
    /* MapObject */
    if (FAILED(CMapObject::Initialize(_pArg)))
        return E_FAIL;

    MAP_INIT_DESC* pInitDesc = static_cast<MAP_INIT_DESC*>(_pArg);
    if (FAILED(Ready_Components(&pInitDesc->tObjectDesc)))
        return E_FAIL;

    /* 6. Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

    /* Set Init Info */
    if (!lstrcmp(pInitDesc->tObjectDesc.ModelTag, TEXT("Prototype_Component_Model_Map_OrangeUnevenSwitch")))
    {
        m_eColor = BLOCK_COLOR::ORANGE;
    }
    else if (!lstrcmp(pInitDesc->tObjectDesc.ModelTag, TEXT("Prototype_Component_Model_Map_PurpleUnevenSwitch")))
    {
        m_eColor = BLOCK_COLOR::BLUE;
    }

    _bool bActive = false;
    if (m_eColor == BLOCK_COLOR::ORANGE)
        bActive = (s_eState == ACTIVE_BLOCK::ORANGE);
    else
        bActive = (s_eState == ACTIVE_BLOCK::BLUE);

    m_bPrevBlockActive = bActive;

    if (bActive)
    {
        Set_Animation("on_wait", false);
        m_pCollider->Set_Active(true);
    }
    else
    {
        Set_Animation("off_wait", false);
        m_pCollider->Set_Active(false);
    }

    return S_OK;
}

void CColorToggleBlock::Priority_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;
}

void CColorToggleBlock::Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    /* Block */
    _bool bBlockActive = false;
    if (m_eColor == BLOCK_COLOR::ORANGE)
        bBlockActive = (s_eState == ACTIVE_BLOCK::ORANGE);
    else
        bBlockActive = (s_eState == ACTIVE_BLOCK::BLUE);

    if (bBlockActive != m_bPrevBlockActive)
    {
        m_bPrevBlockActive = bBlockActive;

        if (bBlockActive)
        {
            Set_Animation("on", false);
            m_pCollider->Set_Active(true);
        }
        else
        {
            Set_Animation("off", false);
            m_pCollider->Set_Active(false);
        }

        /* SFX */
        //m_pGameInstance->Play_Sound(L"OBJ_ColorBlock.wav", SOUND::EFFECT, 1.f);
    }

    /* Animation */
    m_pModelCom->Play_Animation(_fTimeDelta);

    /* Collider */
    if (m_pCollider)
        m_pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CColorToggleBlock::Late_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CColorToggleBlock::Render()
{
    if (!m_bActive)
        return S_OK;

    /* Effect */
    _float fHitStrength = -1.f;
    _float fDissolve = -1.f;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fHitStrength", &fHitStrength, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float))))
        return E_FAIL;

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
        /* Bone */
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        /* Shader */
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        /* Mesh */
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

#ifdef _DEBUG
    //if (m_pCollider) m_pCollider->Render();
#endif

    return S_OK;
}

void CColorToggleBlock::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
}

void CColorToggleBlock::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CColorToggleBlock::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
}

void CColorToggleBlock::Set_Animation(_string _AnimName, _bool _bLoop, _bool _isForce)
{
    if (!m_pModelCom)
        return;

    if (FAILED(m_pModelCom->Set_Animation_Name(_AnimName, _bLoop, _isForce)))
        return;
}

HRESULT CColorToggleBlock::Ready_Components(OBJECT_DESC* _pTags)
{
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        _pTags->ShaderTag,
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    if (FAILED(Add_Component(
        _pTags->iLevel,
        _pTags->ModelTag,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
        return E_FAIL;

    return S_OK;
}

HRESULT CColorToggleBlock::Ready_Collider()
{
    /* Collider */
    CBounding_AABB::AABB_DESC tAABBDesc = {};
    tAABBDesc.vExtents = _float3(0.6f, 0.5f, 0.6f);
    tAABBDesc.vCenter = _float3(0.f, tAABBDesc.vExtents.y, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_AABB"),
        TEXT("Com_Collider"),
        reinterpret_cast<CComponent**>(&m_pCollider),
        &tAABBDesc)))
        return E_FAIL;

    m_pCollider->Set_Group(GROUP::BOUNDARY);
    m_pCollider->Set_Owner(this);
    m_pGameInstance->Register_Collider(m_pCollider);
}

CColorToggleBlock* CColorToggleBlock::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CColorToggleBlock* pInstance = new CColorToggleBlock(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CColorToggleBlock");
        Safe_Release(pInstance);
    }
    return pInstance;
}


CGameObject* CColorToggleBlock::Clone(void* _pArg)
{
    CColorToggleBlock* pInstance = new CColorToggleBlock(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("Failed to Cloned : CColorToggleBlock");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CColorToggleBlock::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
    Safe_Release(m_pCollider);
}
