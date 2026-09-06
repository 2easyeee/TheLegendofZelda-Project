#include "GameObject/SwordSlash.h"
#include "GameInstance.h"

CSwordSlash::CSwordSlash(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CEffect { _pDevice, _pDeviceContext }
{
}

CSwordSlash::CSwordSlash(const CSwordSlash& _Prototype)
    : CEffect (_Prototype)
{
}

HRESULT CSwordSlash::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSwordSlash::Initialize(void* _pArg)
{
    /* CEffect */
    if (FAILED(CEffect::Initialize(_pArg)))
        return E_FAIL;

    /* Set LifTime */
    m_tTime.fAccDurationTime = 1.f;

    return S_OK;
}

void CSwordSlash::Priority_Update(_float _fTimeDelta)
{
    CEffect::Priority_Update(_fTimeDelta);
}

void CSwordSlash::Update(_float _fTimeDelta)
{
    CEffect::Update(_fTimeDelta);
}

void CSwordSlash::Late_Update(_float _fTimeDelta)
{
    CEffect::Late_Update(_fTimeDelta);
}

HRESULT CSwordSlash::Render()
{
    if (FAILED(CEffect::Render()))
        return S_OK;

    return S_OK;
}

CSwordSlash* CSwordSlash::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CSwordSlash* pInstance = new CSwordSlash(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CSwordSlash");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CSwordSlash::Clone(void* _pArg)
{
    CSwordSlash* pInstance = new CSwordSlash(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CSwordSlash");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CSwordSlash::Free()
{
    __super::Free();
}
