#include "GameObject/CrystalSwitch.h"
#include "GameInstance.h"
#include "GameObject/ColorToggleBlock.h"

CCrystalSwitch::CCrystalSwitch(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CMapObject{ _pDevice, _pDeviceContext }
{
}

CCrystalSwitch::CCrystalSwitch(const CCrystalSwitch& _Prototype)
    : CMapObject(_Prototype)
{
}

HRESULT CCrystalSwitch::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCrystalSwitch::Initialize(void* _pArg)
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

    return S_OK;
}

void CCrystalSwitch::Priority_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;
}

void CCrystalSwitch::Update(_float _fTimeDelta)
{
    if (m_pCollider)
        m_pCollider->Set_Active(m_bActive);

    if (!m_bActive)
        return;

    /* Hit */
    if (m_tHitTime.bActive)
    {
        m_tHitTime.fAccTime += _fTimeDelta;
        if (m_tHitTime.fAccTime >= m_tHitTime.fAccDurationTime)
        {
            m_tHitTime.bActive = false;
            m_tHitTime.fAccTime = 0.f;
        }
    }
    /* Collider */
    if (m_pCollider)
        m_pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CCrystalSwitch::Late_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CCrystalSwitch::Render()
{
    if (!m_bActive)
        return S_OK;

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
        /* Material (Diffuse) */
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MATERIAL::DIFFUSE)))
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

void CCrystalSwitch::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
    if (_pSrc->Get_Group() == GROUP::MAP_OBJECT)
    {
        if (_Dst->Get_Group() == GROUP::WEAPON_PLAYER)
        {
            if (m_tHitTime.bActive)
                return;

            m_tHitTime.bActive = true;
            m_pGameInstance->Start_HitStop(0.1f, 0.f);
            m_bRequestShake = true;

            if (CColorToggleBlock::s_eState == CColorToggleBlock::ACTIVE_BLOCK::ORANGE)
                CColorToggleBlock::s_eState = CColorToggleBlock::ACTIVE_BLOCK::BLUE;
            else
                CColorToggleBlock::s_eState = CColorToggleBlock::ACTIVE_BLOCK::ORANGE;

            if (!m_bClearPuzzleOnce)
            {
                /* SFX */
                m_pGameInstance->Play_Sound(L"OBJ_Puzzle_Solved.mp3", SOUND::EFFECT);
                m_bClearPuzzleOnce = true;
            }
            else
            {
                /* SFX */
                m_pGameInstance->Play_Sound(L"OBJ_CrystalSwitch.wav", SOUND::EFFECT, 2.f);
            }
        }
    }
}

void CCrystalSwitch::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CCrystalSwitch::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
}

_bool CCrystalSwitch::Check_CameraShake()
{
    if (m_bRequestShake)
    {
        m_bRequestShake = false;
        return true;
    }
    return false;
}

HRESULT CCrystalSwitch::Ready_Components(OBJECT_DESC* _pTags)
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

HRESULT CCrystalSwitch::Ready_Collider()
{
    /* Collider */
    CBounding_AABB::AABB_DESC tAABBDesc = {};
    tAABBDesc.vExtents = _float3(0.5f, 0.5f, 0.5f);
    tAABBDesc.vCenter = _float3(0.f, tAABBDesc.vExtents.y, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_AABB"),
        TEXT("Com_Collider"),
        reinterpret_cast<CComponent**>(&m_pCollider),
        &tAABBDesc)))
        return E_FAIL;

    m_pCollider->Set_Group(GROUP::MAP_OBJECT);
    m_pCollider->Set_Owner(this);
    m_pGameInstance->Register_Collider(m_pCollider);

    return S_OK;
}

CCrystalSwitch* CCrystalSwitch::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CCrystalSwitch* pInstance = new CCrystalSwitch(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CCrystalSwitch");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CCrystalSwitch::Clone(void* _pArg)
{
    CCrystalSwitch* pInstance = new CCrystalSwitch(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("Failed to Cloned : CCrystalSwitch");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCrystalSwitch::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
    Safe_Release(m_pCollider);
}
