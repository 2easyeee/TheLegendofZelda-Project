#include "GameObject/Monster_Bomber.h"
#include "GameInstance.h"
#include "GameObject/Bomber_IDLE.h"
#include "GameObject/Bomber_Walk.h"
#include "GameObject/Bomber_Browoff.h"

CMonster_Bomber::CMonster_Bomber(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CPawnObject{ _pDevice, _pDeviceContext }
{
}

CMonster_Bomber::CMonster_Bomber(const CMonster_Bomber& _Prototype)
	: CPawnObject(_Prototype)
{
}

HRESULT CMonster_Bomber::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster_Bomber::Initialize(void* _pArg)
{
	/* 1. Pawn Object */
	if (FAILED(CPawnObject::Initialize(_pArg)))
		return E_FAIL;

	/* State */
	Register_State(new CBomber_IDLE());
	Register_State(new CBomber_Walk());
	Register_State(new CBomber_Browoff());

	RequestToChangeState(ENUM_TO_UINT(STATE_ID::WALK));
	Change_State();

	/* 6. Collider */
	if (FAILED(Ready_Collider()))
		return E_FAIL;

	/* Navigation */
	if (FAILED(Ready_Navigation()))
		return E_FAIL;

	return S_OK;
}

void CMonster_Bomber::Priority_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	CPawnObject::Priority_Update(_fTimeDelta);
}

void CMonster_Bomber::Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	/* Effect */
	if (m_tPreExplodeFlash.bActive)
	{
		m_tExplodeTime.fAccTime += _fTimeDelta;
		_float fRatio = m_tExplodeTime.fAccTime / m_tExplodeTime.fAccDurationTime;
		fRatio = min(fRatio, 1.f);

		_float curve = fRatio * fRatio * fRatio;
		_float startInterval = 1.f;
		_float endInterval = 0.1f; 
		m_tPreExplodeFlash.fAccDurationTime = startInterval + (endInterval - startInterval) * curve;

		m_tPreExplodeFlash.fAccTime += _fTimeDelta;
		if (m_tPreExplodeFlash.fAccTime >= m_tPreExplodeFlash.fAccDurationTime)
		{
			Start_HitFlash();
			m_tPreExplodeFlash.fAccTime = 0.f;
		}
	}

	/* PawnObject */
	CPawnObject::Update(_fTimeDelta);

	/* Navigation */
	if (m_pNavCom)
		m_pTransformCom->Set_State(STATE::POSITION, m_pNavCom->SetUp_OnNavigation(m_pTransformCom->Get_State(STATE::POSITION)));
	
	/* Collider */
	if (m_pBodyCollider)
		m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CMonster_Bomber::Late_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	CPawnObject::Late_Update(_fTimeDelta);

	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
		return;

	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::SHADOW, this)))
		return;
}

HRESULT CMonster_Bomber::Render()
{
	if (!m_bActive)
		return S_OK;

	if (FAILED(CPawnObject::Render()))
		return E_FAIL;

#ifdef _DEBUG
	//if (m_pBodyCollider) m_pBodyCollider->Render();
	//if (m_pNavCom) m_pNavCom->Render();
#endif

	return S_OK;
}

void CMonster_Bomber::Collider_BodyEnable(_bool _bActive)
{
	if (m_pBodyCollider)
		m_pBodyCollider->Set_Active(_bActive);
}

void CMonster_Bomber::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
	if (_pSrc->Get_Group() == GROUP::MONSTER)
	{
		if (_Dst->Get_Group() == GROUP::WEAPON_PLAYER)
		{
			Set_Target(_Dst->Get_Owner());
			Start_HitFlash();
			m_tPreExplodeFlash.bActive = true;
			m_tPreExplodeFlash.fAccTime = 0.f;
			RequestToChangeState(ENUM_TO_UINT(STATE_ID::BROWOFF));
		}
	}
}

void CMonster_Bomber::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CMonster_Bomber::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
}

void CMonster_Bomber::Notify_Delete()
{
	this->Set_Active(false);
	Collider_BodyEnable(false);

	/* EFFECT */

	/* SFX */
	CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Bomber_Delete.wav", SOUND::EFFECT, 3.f);
}

HRESULT CMonster_Bomber::Ready_Collider()
{
	/* Collider (Body) */
	CBounding_AABB::AABB_DESC tAABBDesc = {};
	tAABBDesc.vExtents = _float3(0.5f, 0.5f, 0.5f);
	tAABBDesc.vCenter = _float3(0.f, tAABBDesc.vExtents.y, 0.f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_BodyCollider"),
		reinterpret_cast<CComponent**>(&m_pBodyCollider),
		&tAABBDesc)))
		return E_FAIL;

	m_pBodyCollider->Set_Owner(this);
	m_pBodyCollider->Set_Group(GROUP::MONSTER);
	m_pGameInstance->Register_Collider(m_pBodyCollider);

	return S_OK;
}

HRESULT CMonster_Bomber::Ready_Navigation()
{
	CNavigation::NAVIGATION_DESC tNavDesc = {};
	tNavDesc.iCurrentCellIndex = 35;

	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_Dungeon_MONSTER"),
		TEXT("Com_Navigation"),
		reinterpret_cast<CComponent**>(&m_pNavCom),
		&tNavDesc)))
		return E_FAIL;

	_int iCell = m_pNavCom->Find_CurrentCell(m_pTransformCom->Get_State(STATE::POSITION));
	m_pNavCom->Set_CurrentCell(iCell);

	return S_OK;
}

CMonster_Bomber* CMonster_Bomber::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CMonster_Bomber* pInstance = new CMonster_Bomber(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMonster_Bomber");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMonster_Bomber::Clone(void* _pArg)
{
	CMonster_Bomber* pInstance = new CMonster_Bomber(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster_Bomber");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMonster_Bomber::Free()
{
	__super::Free();
}