#include "Ladder.h"

CLadder::CLadder(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CGameObject{ _pDevice, _pDeviceContext }
{
}

CLadder::CLadder(const CLadder& _Prototype)
    : CGameObject (_Prototype)
{
}

HRESULT CLadder::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLadder::Initialize(void* _pArg)
{
    /* 0. CGameObject ÀÇ Init */
    OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("Ladder"));

    if (FAILED(CGameObject::Initialize(&tObjectDesc)))
        return E_FAIL;
    if (FAILED(m_pTransformCom->Initialize(nullptr)))
        return E_FAIL;

    /* Ladder */
    LADDER_DESC* pDesc = static_cast<LADDER_DESC*>(_pArg);
    pDesc->vTopPos = m_vTopPos;
    pDesc->vBottomPos = m_vBottomPos;

	return S_OK;
}

void CLadder::Priority_Update(_float _fTimeDelta)
{
}

void CLadder::Update(_float _fTimeDelta)
{
}

void CLadder::Late_Update(_float _fTimeDelta)
{
}

HRESULT CLadder::Render()
{
	return S_OK;
}

_float CLadder::Get_TopY() const
{
    return XMVectorGetY(m_vTopPos);
}

_float CLadder::Get_BottomY() const
{
    return XMVectorGetY(m_vBottomPos);
}

CLadder* CLadder::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CLadder* pInstance = new CLadder(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CLadder");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CLadder::Clone(void* _pArg)
{
    CLadder* pInstance = new CLadder(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CLadder");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CLadder::Free()
{
	__super::Free();
}
