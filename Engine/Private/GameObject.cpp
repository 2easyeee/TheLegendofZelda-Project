#include "GameObject.h"

#include "GameInstance.h"
#include "Component.h"

CGameObject::CGameObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: m_pGameInstance { CGameInstance::GetInstance() }
	, m_pDevice { _pDevice }
	, m_pDeviceContext { _pDeviceContext }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pDeviceContext);
}

CGameObject::CGameObject(const CGameObject& _Prototype)
	: m_pGameInstance{ CGameInstance::GetInstance() }
	, m_pDevice{ _Prototype.m_pDevice }
	, m_pDeviceContext{ _Prototype.m_pDeviceContext }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pDeviceContext);
}

HRESULT CGameObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGameObject::Initialize(void* _pArg)
{
	OBJECT_DESC* pObjectDesc = static_cast<OBJECT_DESC*>(_pArg);

	/* OBJECT_DESC */
	memcpy(&m_ObjectDesc, pObjectDesc, sizeof(OBJECT_DESC));
	m_wstrObjectID = m_ObjectDesc.ObjectID;

	/* Transform */
	m_pTransformCom = CTransform::Create(m_pDevice, m_pDeviceContext);
	if (nullptr == m_pTransformCom)
		return E_FAIL;
	m_Components.emplace(TEXT("Com_Transform"), m_pTransformCom);
	Safe_AddRef(m_pTransformCom);

	return S_OK;
}

void CGameObject::Priority_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;
}

void CGameObject::Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;
}

void CGameObject::Late_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;
}

HRESULT CGameObject::Render()
{
	if (!m_bActive || !m_OnlyRenderActive)
		return S_OK;

	return S_OK;
}

HRESULT CGameObject::Render_Shadow()
{
	return S_OK;
}

CComponent* CGameObject::Get_Component(const _wstring& _strComponentTag)
{
	auto iter = m_Components.find(_strComponentTag);

	if (iter == m_Components.end())
		return nullptr;

	return iter->second;
}

_bool CGameObject::Compare_ObjectID(const _wstring& _strObjectID)
{
	return m_wstrObjectID == _strObjectID;
}

HRESULT CGameObject::Add_Component(_uint _iPrototypeLevelID, const _wstring& _strPrototypeTag, const _wstring& _strComponentTag, CComponent** _ppOut, void* _pArg)
{
	if (m_Components.end() != m_Components.find(_strComponentTag))
		return E_FAIL;

	/* Clone Componet in PROTOTYPE_MANAGER */
	CComponent* pComponent = static_cast<CComponent*>(
		m_pGameInstance->Clone_Prototype(
			PROTOTYPE::COMPONENT,
			_iPrototypeLevelID,
			_strPrototypeTag,
			_pArg));
	if (nullptr == pComponent)
		return E_FAIL;

	m_Components.emplace(_strComponentTag, pComponent);

	*_ppOut = pComponent;

	Safe_AddRef(pComponent);

	return S_OK;
}

HRESULT CGameObject::Remove_Component(const _wstring& _strComponentTag)
{
	auto iter = m_Components.find(_strComponentTag);
	if (iter == m_Components.end())
		return E_FAIL;

	CComponent* pComponent = iter->second;

	if (pComponent)
	{
		Safe_Release(pComponent);
		pComponent = nullptr;
	}

	m_Components.erase(iter);

	return S_OK;
}

void CGameObject::Free()
{
	__super::Free();

    /* map release */
	for (auto& Pair : m_Components)
		Safe_Release(Pair.second);
	m_Components.clear();

	Safe_Release(m_pTransformCom);
	Safe_Release(m_pDeviceContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
