#include "Camera_Static.h"
#include "GameInstance.h"

CCamera_Static::CCamera_Static(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CCamera{ _pDevice, _pDeviceContext }
{
}

CCamera_Static::CCamera_Static(const CCamera_Static& _Prototype)
    : CCamera(_Prototype)
{
}

HRESULT CCamera_Static::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCamera_Static::Initialize(void* _pArg)
{
    if (FAILED(CCamera::Initialize(_pArg)))
        return E_FAIL;

    return S_OK;
}

void CCamera_Static::Priority_Update(_float _fTimeDelta)
{
    /* PipeLine */
    __super::Update_PipeLines();
}

void CCamera_Static::Update(_float _fTimeDelta)
{
}

void CCamera_Static::Late_Update(_float _fTimeDelta)
{
}

HRESULT CCamera_Static::Render()
{
    return S_OK;
}

HRESULT CCamera_Static::Ready_Components()
{
    return S_OK;
}

void CCamera_Static::Mouse_Enable()
{
}

CCamera_Static* CCamera_Static::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CCamera_Static* pInstance = new CCamera_Static(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CCamera_Static");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CCamera_Static::Clone(void* _pArg)
{
    CCamera_Static* pInstance = new CCamera_Static(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CCamera_Static");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCamera_Static::Free()
{
    __super::Free();
}
