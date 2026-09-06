#include "LightManager.h"
#include "Light.h"
#include "GameObject.h"

CLightManager::CLightManager()
{
}

HRESULT CLightManager::Add_Light(const LIGHT_DESC& _tLightDesc, CTransform* _pTarget, _vector _vOffset)
{
    CLight* pLight = CLight::Create(_tLightDesc);
    if (nullptr == pLight)
        return E_FAIL;

    if (_tLightDesc.eUsage == LIGHT_USAGE::DYNAMIC && _pTarget)
    {
        pLight->Set_Target(_pTarget);
        pLight->Set_Offset(_vOffset);

        /* Init Snap */
        _vector vPos = _pTarget->Get_State(STATE::POSITION);
        _vector vFinal = vPos + _vOffset;
        pLight->Set_Position(vFinal);
    }

    m_Lights.push_back(pLight);

    return S_OK;
}

void CLightManager::Update(_float _fTimeDelta)
{
    for (auto& pLight : m_Lights)
    {
        if (pLight)
            pLight->Update(_fTimeDelta);
    }
}

void CLightManager::Render(CShader* _pShader, CVIBuffer_Rect* _pVIBuffer)
{
    for (auto& pLight : m_Lights)
    {
        if (nullptr != pLight)
            pLight->Render(_pShader, _pVIBuffer);
    }
}

void CLightManager::Clear_Light()
{
    for (auto& pLight : m_Lights)
        Safe_Release(pLight);

    m_Lights.clear();
}

const LIGHT_DESC* CLightManager::Get_LightDesc(_uint _iIndex)
{
    auto iter = m_Lights.begin();

    for (size_t i = 0; i < _iIndex; i++)
    {
        ++iter;
    }

    if (iter == m_Lights.end())
        return nullptr;

    return (*iter)->Get_Light();
}

void CLightManager::Scale_AllPointLightIntensity(_float _fIntensity)
{
    for (auto& pLight : m_Lights)
    {
        if (!pLight)
            continue;

        const LIGHT_DESC* pDesc = pLight->Get_Light();

        if (pDesc->eType == LIGHT::POINT && pDesc->eUsage == LIGHT_USAGE::STATIC)
        {
            LIGHT_DESC* pMutable = const_cast<LIGHT_DESC*>(pDesc);

            pMutable->fIntensity = _fIntensity;
        }
    }
}

void CLightManager::Pop_Back_Lights(_uint _iCount)
{
    for (_uint i = 0; i < _iCount; ++i)
    {
        if (m_Lights.empty())
            break;

        auto pLight = m_Lights.back();
        Safe_Release(pLight);

        m_Lights.pop_back();
    }
}

CLightManager* CLightManager::Create()
{
    return new CLightManager();
}

void CLightManager::Free()
{
    __super::Free();

    /* list release */
    for (auto& pLight : m_Lights)
        Safe_Release(pLight);
}
