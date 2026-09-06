#include "GameObject/Accessory_Player.h"
#include "GameInstance.h"

CAccessory_Player::CAccessory_Player(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CPartObject{ _pDevice, _pDeviceContext }
{
}

CAccessory_Player::CAccessory_Player(const CAccessory_Player& _Prototype)
    : CPartObject(_Prototype)
{
}

HRESULT CAccessory_Player::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAccessory_Player::Initialize(void* _pArg)
{
    /* 1. PartObject */
    PART_CREATE_DESC* tInitDesc = static_cast<PART_CREATE_DESC*>(_pArg);
    if (FAILED(CPartObject::Initialize(tInitDesc)))
        return E_FAIL;

    if (&tInitDesc->tPartDesc)
    {
        m_pParentState = tInitDesc->tPartDesc.pPartentSTATE;
        m_pSocketMatrix = tInitDesc->tPartDesc.pSocketMatrix;
    }

    if (&tInitDesc->tObjectDesc)
    {
        /* 2. Component */
        if (FAILED(Ready_Components(&tInitDesc->tObjectDesc)))
            return E_FAIL;
    }

    /* Here Transform */
    m_pTransformCom->Set_Scale(0.5f, 0.5f, 0.5f);
    m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 240.f);
    m_pTransformCom->Set_State(Engine::STATE::POSITION, XMVectorSet(0.f, 0.5f, 0.f, 1.f));

    return S_OK;
}

void CAccessory_Player::Priority_Update(_float _fTimeDelta)
{
}

void CAccessory_Player::Update(_float _fTimeDelta)
{
    return;

    /* 1. SocketMatrix */
    _matrix WorldMatrix = XMMatrixIdentity();
    _matrix SocketMatrix = XMLoadFloat4x4(m_pSocketMatrix);
    for (size_t i = 0; i < ENUM_TO_UINT(PART::END); i++)
    {
        SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);
    }
    WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * SocketMatrix;

    /* 2. Update WorldMatrix */
    CPartObject::Update_WorldMatrix(WorldMatrix);
}

void CAccessory_Player::Late_Update(_float _fTimeDelta)
{
    return;

    CPartObject::Late_Update(_fTimeDelta);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CAccessory_Player::Render()
{
    return S_OK;

    /* Transform */
    if (FAILED(CPartObject::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
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

    return S_OK;
}

HRESULT CAccessory_Player::Ready_Components(OBJECT_DESC* _Desc)
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

    return S_OK;
}

CAccessory_Player* CAccessory_Player::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CAccessory_Player* pInstance = new CAccessory_Player(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CAccessory_Player");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CAccessory_Player::Clone(void* _pArg)
{
    CAccessory_Player* pInstance = new CAccessory_Player(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CAccessory_Player");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CAccessory_Player::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
