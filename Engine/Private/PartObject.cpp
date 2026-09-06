#include "PartObject.h"
#include "Shader.h"

CPartObject::CPartObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CGameObject{ _pDevice, _pDeviceContext }
{
}

CPartObject::CPartObject(const CPartObject& _Prototype)
    : CGameObject(_Prototype)
{
}

HRESULT CPartObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CPartObject::Initialize(void* _pArg)
{
    PART_CREATE_DESC* pInitDesc = static_cast<PART_CREATE_DESC*>(_pArg);
    /* 1. PARTObject  */
    if (&pInitDesc->tPartDesc)
    {
        m_pParentMatrix = pInitDesc->tPartDesc.pParentMatrix;
        XMStoreFloat4x4(&m_CombinedWorldMatrix, XMMatrixIdentity());
    }
    /* 2. CGameObject*/
    if (&pInitDesc->tObjectDesc)
    {
        if (FAILED(CGameObject::Initialize(&pInitDesc->tObjectDesc)))
            return E_FAIL;
        if (FAILED(m_pTransformCom->Initialize(nullptr)))
            return E_FAIL;
    }

    return S_OK;
}

void CPartObject::Priority_Update(_float _fTimeDelta)
{
    CGameObject::Priority_Update(_fTimeDelta);
}

void CPartObject::Update(_float _fTimeDelta)
{
    CGameObject::Update(_fTimeDelta);
}

void CPartObject::Late_Update(_float _fTimeDelta)
{
    CGameObject::Late_Update(_fTimeDelta);
}

HRESULT CPartObject::Render()
{
    if (FAILED(CGameObject::Render()))
        return E_FAIL;

    return S_OK;
}

HRESULT CPartObject::Update_WorldMatrix(_fmatrix _childWorldMatrix)
{
    if (m_pParentMatrix)
    {
        /* 상향식 */
        XMStoreFloat4x4(&m_CombinedWorldMatrix,
            _childWorldMatrix * XMLoadFloat4x4(m_pParentMatrix));
    }
    else
    {
        /* 상향식 */
        XMStoreFloat4x4(&m_CombinedWorldMatrix, _childWorldMatrix);
    }

    return S_OK;
}

HRESULT CPartObject::Bind_WorldMatrix(CShader* _pShader, const _char* _pConstantName)
{
    return _pShader->Bind_Matrix(_pConstantName, &m_CombinedWorldMatrix);
}

void CPartObject::Free()
{
    __super::Free();
}
