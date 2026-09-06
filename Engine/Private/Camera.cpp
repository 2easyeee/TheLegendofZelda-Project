#include "Camera.h"
#include "GameInstance.h"

CCamera::CCamera(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CWorldObject { _pDevice, _pDeviceContext }
{
}

CCamera::CCamera(const CCamera& _Prototype)
    : CWorldObject (_Prototype)
{
}

HRESULT CCamera::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCamera::Initialize(void* _pArg)
{
    if (nullptr == _pArg)
        return E_FAIL;

    CAMERA_INIT_DESC* pInitDesc = static_cast<CAMERA_INIT_DESC*>(_pArg);

    /* 0. Init a Transform */
    if (FAILED(CGameObject::Initialize(&pInitDesc->tObjectDesc)))
        return E_FAIL;

    /* 1. Init a WorldObject */
    if (FAILED(CWorldObject::Initialize(&pInitDesc->tWorldDesc)))
        return E_FAIL;

    /* 1-0. Set a Transform */
    if (&pInitDesc->tCameraDesc)
    {
       CAMERA_DESC* pCameraDesc = static_cast<CAMERA_DESC*>(&pInitDesc->tCameraDesc);
       if (FAILED(m_pTransformCom->Initialize(pCameraDesc)))
           return E_FAIL;

       /* 1-1. Set 'w' = 1.f */
       m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pCameraDesc->vEye), 1.f));
       m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&pCameraDesc->vAt), 1.f));

       /* 1-2. 원근 투영을 위한 Setting */
       _uint iNumViewports = { 1 };
       D3D11_VIEWPORT ViewportDesc = {};
       m_pDeviceContext->RSGetViewports(&iNumViewports, &ViewportDesc);

       /* 1-3. set 멤버변수 (ViewportDesc 뒤에) */
       m_fFovy = pCameraDesc->fFovy;
       m_fNear = pCameraDesc->fNear;
       m_fFar = pCameraDesc->fFar;
       m_fAspect = static_cast<_float>(ViewportDesc.Width) / ViewportDesc.Height;
       m_fMouseSensor = pCameraDesc->fSensor;
    }

    /* PipeLine */
    Update_PipeLines();

    return S_OK;
}

void CCamera::Priority_Update(_float _fTimeDelta)
{
}

void CCamera::Update(_float _fTimeDelta)
{
}

void CCamera::Late_Update(_float _fTimeDelta)
{
}

HRESULT CCamera::Render()
{
    return S_OK;
}

void CCamera::Update_PipeLines()
{
    m_pGameInstance->Set_Transform(D3DTS::VIEW, XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));
    m_pGameInstance->Set_Transform(D3DTS::PROJ, XMMatrixPerspectiveFovLH(m_fFovy, m_fAspect, m_fNear, m_fFar));
}

void CCamera::Free()
{
    __super::Free();
}
