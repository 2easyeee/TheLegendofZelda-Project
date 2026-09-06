#include "EffectManager.h"
#include "GameInstance.h"

CEffectManager::CEffectManager()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CEffectManager::Initialize()
{
    return S_OK;
}

void CEffectManager::Update(_float _fTimeDelta)
{
    EFFECT_EVENT tEvent;
    
    while (Pop_EffectEVENT(tEvent))
    {
        switch (tEvent.eType)
        {
        case EFFECT_EVENT_TYPE::SPAWN_EFFECT:
        {
            /* 1. Find */
            auto iter = m_EffectRegistry.find(tEvent.EffectName);
            if (iter == m_EffectRegistry.end())
                break;

            /* 2. Info */
            CEffect::EFFECT_INIT_DESC tDesc = {};
            tDesc = iter->second;
            tDesc.fScale = tEvent.fScale;

            /* 3. Clone */
            CGameObject* pGameObject = { nullptr };
            m_pGameInstance->Add_GameObject_ToLayer(
                RESOURCE_LEVEL_STATIC,
                tDesc.tObjectDesc.ObjectTag,
                RESOURCE_LEVEL_STATIC,
                TEXT("Layer_Effect"),
                &tDesc, &pGameObject);

            /* 4. Transform */
            CTransform* pTransform = static_cast<CTransform*>(pGameObject->Get_Component(TEXT("Com_Transform")));
            pTransform->Set_State(STATE::POSITION, tEvent.vPosition);

            CEffect* pEffect = static_cast<CEffect*>(pGameObject);
            if (pEffect)
            {
                pEffect->Set_Dir(tEvent.vDirection);
            }
        }
        break;
        case EFFECT_EVENT_TYPE::TRAIL:
        {
            /* 1. Find */
            auto iter = m_TrailEffectRegistry.find(tEvent.EffectName);
            if (iter == m_TrailEffectRegistry.end())
                break;

            /* 2. Info */
            CTrailEffect::TRAIL_INIT_DESC tDesc = {};
            tDesc = iter->second;
            
            tDesc.pParentMatrix = tEvent.pParentMatrix;
            tDesc.pBaseSocketMatrix = tEvent.pBaseSocketMatrix;
            tDesc.pTipSocketMatrix = tEvent.pTipSocketMatrix;
            tDesc.fTrailDuration = tEvent.fDuration;

            /* 3. Clone */
            CGameObject* pGameObject = { nullptr };
            m_pGameInstance->Add_GameObject_ToLayer(
                RESOURCE_LEVEL_STATIC,
                tDesc.tObjectDesc.ObjectTag,
                RESOURCE_LEVEL_STATIC,
                TEXT("Layer_Effect_Trail"),
                &tDesc, &pGameObject);
        }
        break;
        }
    }
}

void CEffectManager::Push_EffectEVENT(EFFECT_EVENT& _tEffectEvent)
{
    m_EffectEvents.push(_tEffectEvent);
}

_bool CEffectManager::Pop_EffectEVENT(EFFECT_EVENT& _tEffectEvent)
{
    if (m_EffectEvents.empty())
        return false;

    _tEffectEvent = m_EffectEvents.front();
    m_EffectEvents.pop();

    return true;
}

HRESULT CEffectManager::Register_EffectEvent(_wstring _EventName, CEffect::EFFECT_INIT_DESC _tDesc)
{
    if (m_EffectRegistry.find(_EventName) != m_EffectRegistry.end())
        return E_FAIL;

    m_EffectRegistry.emplace(_EventName, _tDesc);

    return S_OK;
}

HRESULT CEffectManager::Register_EffectEvent(_wstring _EventName, CTrailEffect::TRAIL_INIT_DESC _tDesc)
{
    if (m_TrailEffectRegistry.find(_EventName) != m_TrailEffectRegistry.end())
        return E_FAIL;

    m_TrailEffectRegistry.emplace(_EventName, _tDesc);

    return S_OK;
}

CEffectManager* CEffectManager::Create()
{
    CEffectManager* pInstance = new CEffectManager();
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CEffectManager");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CEffectManager::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
