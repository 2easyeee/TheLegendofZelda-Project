#include "GameObject/ChargeShot.h"
#include "GameInstance.h"

CChargeShot::CChargeShot(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CEffect{ _pDevice, _pDeviceContext }
{
}

CChargeShot::CChargeShot(const CChargeShot& _Prototype)
    : CEffect(_Prototype)
{
}

HRESULT CChargeShot::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CChargeShot::Initialize(void* _pArg)
{
    /* Unique ID */
    static _uint iUniqueID = 0;
    _wstring newID = TEXT("ChargeShot") + to_wstring(iUniqueID++);
    EFFECT_INIT_DESC* pInitDesc = static_cast<EFFECT_INIT_DESC*>(_pArg);
    wcscpy_s(pInitDesc->tObjectDesc.ObjectID, MAX_PATH, newID.c_str());

    /* CEffect */
    if (FAILED(CEffect::Initialize(pInitDesc)))
        return E_FAIL;

    /* Detail */
    Set_TextureSlots(0, 0, 0, 0, 0, 0);
    Set_EffectPassIndex(EFFECT_TYPE::DEFAULT);

    m_tTime.fAccDurationTime = 0.4f + ((rand() % 100) / 100.f) * 0.4f;
    m_fRandValue = (rand() % 100) / 100.f;

    /* ChargeShot */
    m_fVSScale = 2.5f;

    return S_OK;
}

void CChargeShot::Priority_Update(_float _fTimeDelta)
{
    CEffect::Priority_Update(_fTimeDelta);
}

void CChargeShot::Update(_float _fTimeDelta)
{
    CEffect::Update(_fTimeDelta);
}

void CChargeShot::Late_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONLIGHT, this)))
        return;
}

HRESULT CChargeShot::Render()
{
    if (FAILED(CEffect::Render()))
        return S_OK;

    return S_OK;
}

CChargeShot* CChargeShot::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CChargeShot* pInstance = new CChargeShot(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CChargeShot");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CChargeShot::Clone(void* _pArg)
{
    CChargeShot* pInstance = new CChargeShot(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CChargeShot");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CChargeShot::Free()
{
    __super::Free();
}
