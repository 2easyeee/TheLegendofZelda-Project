#include "GameObject/Monster_BuzzBlob.h"
#include "GameInstance.h"
#include "GameObject/BuzzBlob_Walk.h"

CMonster_BuzzBlob::CMonster_BuzzBlob(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CPawnObject{ _pDevice, _pDeviceContext }
{
}

CMonster_BuzzBlob::CMonster_BuzzBlob(const CMonster_BuzzBlob& _Prototype)
	: CPawnObject(_Prototype)
{
}
HRESULT CMonster_BuzzBlob::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster_BuzzBlob::Initialize(void* _pArg)
{
	/* 1. Pawn Object */
	if (FAILED(CPawnObject::Initialize(_pArg)))
		return E_FAIL;

	/* 5. Init State */
	Register_State(new CBuzzBlob_Walk());

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

void CMonster_BuzzBlob::Priority_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	CPawnObject::Priority_Update(_fTimeDelta);
}

void CMonster_BuzzBlob::Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	CPawnObject::Update(_fTimeDelta);

	/* Collider */
	if (m_pBodyCollider)
		m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	/* Navigation */
	if (m_pNavCom)
		m_pTransformCom->Set_State(STATE::POSITION, m_pNavCom->SetUp_OnNavigation(m_pTransformCom->Get_State(STATE::POSITION)));
}

void CMonster_BuzzBlob::Late_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	CPawnObject::Late_Update(_fTimeDelta);

	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
		return;
}

HRESULT CMonster_BuzzBlob::Render()
{
	if (!m_bActive)
		return S_OK;

	if (FAILED(CPawnObject::Render()))
		return E_FAIL;

#ifdef _DEBUG
	if (m_pBodyCollider) m_pBodyCollider->Render();
	if (m_pNavCom) m_pNavCom->Render();
#endif

	if (m_bDelete)
	{
		this->Set_Active(false);
		Collider_BodyEnable(false);
	}

	return S_OK;
}

void CMonster_BuzzBlob::Collider_BodyEnable(_bool _bActive)
{
	if (m_pBodyCollider)
		m_pBodyCollider->Set_Active(_bActive);
}

void CMonster_BuzzBlob::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
	if (_pSrc->Get_Group() == GROUP::MONSTER)
	{
		if (_Dst->Get_Group() == GROUP::WEAPON_PLAYER)
		{
			// KnockBack
			CTransform* pTargetTransform = static_cast<CTransform*>(_Dst->Get_Owner()->Get_Component(TEXT("Com_Transform")));
			_vector vWeaponPos = pTargetTransform->Get_State(STATE::POSITION);
			Take_Damage(1.f, vWeaponPos);

			RequestToChangeState(ENUM_TO_UINT(STATE_ID::DAMAGE));
		}
	}
}

void CMonster_BuzzBlob::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CMonster_BuzzBlob::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
}

_float CMonster_BuzzBlob::Compute_Distance_To_Target()
{
	if (!m_pTarget)
		return FLT_MAX;

	CTransform* pTransform_Target = static_cast<CTransform*>(m_pTarget->Get_Component(TEXT("Com_Transform")));

	_vector vMy = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTarget = pTransform_Target->Get_State(STATE::POSITION);

	_vector DeleteY = XMVectorSetY(vTarget - vMy, 0.f);
	_vector vDir = XMVector3Normalize(DeleteY);

	/* Distance */
	_float fDist = XMVectorGetX(XMVector3Length(DeleteY));

	return fDist;
}

HRESULT CMonster_BuzzBlob::Ready_Collider()
{
	/* Collider (Body) */
	CBounding_AABB::AABB_DESC tAABBDesc = {};
	tAABBDesc.vExtents = _float3(0.6f, 0.8f, 0.6f);
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
	m_pBodyCollider->Set_Active(true);
	m_pGameInstance->Register_Collider(m_pBodyCollider);

	return S_OK;
}

HRESULT CMonster_BuzzBlob::Ready_Navigation()
{
	// TODO : why ?
	CNavigation::NAVIGATION_DESC tNavDesc = {};
	tNavDesc.iCurrentCellIndex = 37;

	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_MARTHAsBAY_MONSTER"),
		TEXT("Com_Navigation"),
		reinterpret_cast<CComponent**>(&m_pNavCom),
		&tNavDesc)))
		return E_FAIL;

	//_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vPos = XMVectorSet(41.878f, 0, 28.194f, 1.f);
	
	_int iCell = m_pNavCom->Find_CurrentCell(m_pTransformCom->Get_State(STATE::POSITION));
	m_pNavCom->Set_CurrentCell(iCell);

	return S_OK;
}

void CMonster_BuzzBlob::Take_Damage(_float _fDamage, _vector _vHitPos)
{
	if (m_fHP < 0.f)
		return;

	m_fHP -= _fDamage;

	if (m_fHP <= 0.f)
	{
		Start_KnockBack(_vHitPos, 10.f, 0.5f);
		m_bDelete = true;
	}
}

CMonster_BuzzBlob* CMonster_BuzzBlob::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CMonster_BuzzBlob* pInstance = new CMonster_BuzzBlob(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMonster_BuzzBlob");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMonster_BuzzBlob::Clone(void* _pArg)
{
	CMonster_BuzzBlob* pInstance = new CMonster_BuzzBlob(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster_BuzzBlob");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMonster_BuzzBlob::Free()
{
	__super::Free();
}
