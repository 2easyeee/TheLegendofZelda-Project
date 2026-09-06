#include "Grid.h"
#include "GameInstance.h"

CGrid::CGrid(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CGameObject{ _pDevice, _pDeviceContext }
{
}

CGrid::CGrid(const CGrid& _Prototype)
    : CGameObject(_Prototype)
{
}

HRESULT CGrid::Initialize_Prototype()
{
    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

HRESULT CGrid::Initialize(void* _pArg)
{
    return S_OK;
}

void CGrid::Priority_Update(_float _fTimeDelta)
{
}

void CGrid::Update(_float _fTimeDelta)
{
    const _float4* pCamPos4 = m_pGameInstance->Get_CameraPosition();
    _float3 vCamPos = { pCamPos4->x, pCamPos4->y, pCamPos4->z };

    // Grid 중심 스냅
    vCamPos.x = floorf(vCamPos.x / m_fGridSize) * m_fGridSize;
    vCamPos.z = floorf(vCamPos.z / m_fGridSize) * m_fGridSize;
    vCamPos.y = m_fGridY;

    if (memcmp(&vCamPos, &m_vLastCenter, sizeof(_float3)) != 0)
    {
        m_vLastCenter = vCamPos;

        //// Line VIBuffer에 Grid 정점 재생성
        Engine::CVIBuffer_Line* pVIBufferLine = static_cast<CVIBuffer_Line*>(m_pVIBufferCom);
        pVIBufferLine->Build_Grid(vCamPos, m_fGridSize, m_iHalfCount);
    }
}

void CGrid::Late_Update(_float _fTimeDelta)
{
    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CGrid::Render()
{
    /* Transform */
    if (FAILED(m_pTransformCom->Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    /* Perspective */
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;

    /* Shader */
    if (FAILED(m_pShaderCom->Begin(0)))
        return E_FAIL;

    /* VIBuffer */
    if (FAILED(m_pVIBufferCom->Bind_Resources()))
        return E_FAIL;
    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;
}

HRESULT CGrid::Ready_Components()
{
    if (FAILED(Add_Component(
        ENUM_TO_UINT(LEVEL::STATIC),
        TEXT("Prototype_Component_Shader_VtxLine"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    if (FAILED(Add_Component(
        ENUM_TO_UINT(LEVEL::STATIC),
        TEXT("Prototype_Component_VIBuffer_Line"),
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
        return E_FAIL;

    return S_OK;
}

CGrid* CGrid::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CGrid* pInstance = new CGrid(_pDevice, _pDeviceContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CGrid");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CGrid::Clone(void* _pArg)
{
    CGrid* pInstance = new CGrid(*this);

    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("Failed to Cloned : CGrid");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CGrid::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pShaderCom);
}
