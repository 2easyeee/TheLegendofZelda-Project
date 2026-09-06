#include "Trigger.h"
#include "GameInstance.h"

CTrigger::CTrigger(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CWorldObject{ _pDevice, _pDeviceContext }
{
}

CTrigger::CTrigger(const CTrigger& _Prototype)
    : CWorldObject(_Prototype)
{
}

HRESULT CTrigger::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTrigger::Initialize(void* _pArg)
{
    /* class */
    CGameObject::OBJECT_DESC tObjectDesc = {};
    lstrcpy(tObjectDesc.ObjectID, TEXT("Trigger"));

    if (FAILED(CGameObject::Initialize(&tObjectDesc)))
        return E_FAIL;
    if (FAILED(m_pTransformCom->Initialize(nullptr)))
        return E_FAIL;

    /* Trigger */
    TRIGGER_DESC* pDesc = static_cast<TRIGGER_DESC*>(_pArg);
    if (pDesc)
    {
        m_eType = pDesc->eType;
        m_iValue = pDesc->iValue;
        m_vTargetPosition = pDesc->vTargetPosition;
        m_pTransformCom->Set_State(Engine::STATE::POSITION, XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, pDesc->vPosition.z, 1.f));    
    }

    /* 6. Collider */
    if (FAILED(Ready_Collider(pDesc->vScale)))
        return E_FAIL;

    return S_OK;
}

void CTrigger::Priority_Update(_float _fTimeDelta)
{
}

void CTrigger::Update(_float _fTimeDelta)
{
    /* 6. Collider */
    if (m_pColliderCom)
        m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CTrigger::Late_Update(_float _fTimeDelta)
{
    CWorldObject::Late_Update(_fTimeDelta);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CTrigger::Render()
{
#ifdef _DEBUG
    /* Collider */
    //if (m_pColliderCom)
    //    m_pColliderCom->Render();
#endif // _DEBUG

    return S_OK;
}

void CTrigger::OnTriggerEnter(CCollider* _pSrc, CCollider* _Dst)
{
    if (m_bTriggered)
        return;
    
    if (_pSrc->Get_Group() == GROUP::TRIGGER && _Dst->Get_Group() == GROUP::PLAYER)
    {   
        /* Trigger (Input) */
        if (m_eType == TRIGGERTYPE::BOX_OPEN)
        {
            m_bPlayerInRange = true;
            return;
        }

        /* Trigger (Once) */
        m_bTriggered = true;

        CEventManager::GAME_EVENT tEvent = {};

        switch (m_eType)
        {
        case Client::CTrigger::TRIGGERTYPE::LEVEL_CHANGE:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::LEVEL_CHANGE;
            tEvent.iValue = m_iValue;
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::ROOM_TRANSITION:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::ROOM_TRANSITION;
            tEvent.vPos = XMLoadFloat3(&m_vTargetPosition);
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::STAIR:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::STAIR;
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::LADDER_UP:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::LADDER_UP;
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::LADDER_DOWN:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::LADDER_DOWN;
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::SPAWN_BOSS:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::SPAWN_BOSS;
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::SPAWN_TOWER:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::SPAWN_TOWER;
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::CUTSCENE_START:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::CUTSCENE_START;
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::CUTSCENE_END:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::CUTSCENE_END;
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::DIALOGUE_START:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::DIALOGUE_START;
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::BOX_APPEAR:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::BOX_APPEAR;
            tEvent.iValue = m_iValue;
        }
        break;
        case Client::CTrigger::TRIGGERTYPE::NAV_ATTACH:
        {
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::NAV_ATTACH;
        }
        }
        
        m_pGameInstance->Push_GameEVENT(tEvent);
    }
}

void CTrigger::OnTriggerStay(CCollider* _pSrc, CCollider* _Dst)
{
    if (m_bTriggered)
        return;

    if (m_eType != TRIGGERTYPE::BOX_OPEN)
        return;

    if (_pSrc->Get_Group() == GROUP::TRIGGER && _Dst->Get_Group() == GROUP::PLAYER)
    {
        m_bPlayerInRange = true;

        if (m_pGameInstance->Get_DIKeyDown(DIK_L))
        {
            m_bTriggered = true;

            CEventManager::GAME_EVENT tEvent = {};
            tEvent.eType = CEventManager::GAME_EVENT_TYPE::BOX_OPEN;
            tEvent.iValue = m_iValue;
            m_pGameInstance->Push_GameEVENT(tEvent);
        }
    }
}

void CTrigger::OnTriggerExit(CCollider* _pSrc, CCollider* _Dst)
{
    if (m_eType != TRIGGERTYPE::BOX_OPEN)
        return;

    if (_pSrc->Get_Group() == GROUP::TRIGGER && _Dst->Get_Group() == GROUP::PLAYER)
        m_bPlayerInRange = false;
}

HRESULT CTrigger::Ready_Collider(_float3 _vExtents)
{
    /* Collider */
    CBounding_AABB::AABB_DESC tAABBDesc = {};
    tAABBDesc.vExtents = _vExtents;
    tAABBDesc.vCenter = _float3(0.f, tAABBDesc.vExtents.y, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_AABB"),
        TEXT("Com_Collider"),
        reinterpret_cast<CComponent**>(&m_pColliderCom),
        &tAABBDesc)))
        return E_FAIL;

    m_pColliderCom->Set_Owner(this);
    m_pColliderCom->Set_Group(GROUP::TRIGGER);
    m_pGameInstance->Register_Collider(m_pColliderCom);
    m_pColliderCom->Set_Trigger();

    /* (0, 0, 0) ¹æÁö */
    if (m_pColliderCom)
        m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    return S_OK;
}

CTrigger* CTrigger::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CTrigger* pInstance = new CTrigger(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CTrigger");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CTrigger::Clone(void* _pArg)
{
    CTrigger* pInstance = new CTrigger(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CTrigger");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CTrigger::Free()
{
    __super::Free();

    Safe_Release(m_pColliderCom);
}
