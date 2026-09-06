#include "GameObject/Monster_MoriblinSword.h"
#include "GameInstance.h"
#include "GameObject/MoriblinSword_Attack.h"
#include "GameObject/MoriblinSword_Damage.h"
#include "GameObject/MoriblinSword_Dead.h"
#include "GameObject/MoriblinSword_Find.h"
#include "GameObject/MoriblinSword_Follow.h"
#include "GameObject/MoriblinSword_Guard.h"
#include "GameObject/MoriblinSword_IDLE.h"
#include "GameObject/MoriblinSword_KyoroKyoro.h"
#include "GameObject/MoriblinSword_Walk.h"
#include "GameObject/MoriblinSword_Piyo.h"

CMonster_MoriblinSword::CMonster_MoriblinSword(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CPawnObject{ _pDevice, _pDeviceContext }
{
}

CMonster_MoriblinSword::CMonster_MoriblinSword(const CMonster_MoriblinSword& _Prototype)
    : CPawnObject(_Prototype)
{
}

HRESULT CMonster_MoriblinSword::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMonster_MoriblinSword::Initialize(void* _pArg)
{
    /* 1. Pawn Object */
    if (FAILED(CPawnObject::Initialize(_pArg)))
        return E_FAIL;

    /* 5. Init State */
	Register_State(new CMoriblinSword_Attack());
	Register_State(new CMoriblinSword_Damage());
	Register_State(new CMoriblinSword_Dead());
	Register_State(new CMoriblinSword_Find());
	Register_State(new CMoriblinSword_Follow());
	Register_State(new CMoriblinSword_Guard());
	Register_State(new CMoriblinSword_IDLE());
	Register_State(new CMoriblinSword_KyoroKyoro());
	Register_State(new CMoriblinSword_Walk());
	Register_State(new CMoriblinSword_Piyo());

	RequestToChangeState(ENUM_TO_UINT(STATE_ID::IDLE));
	Change_State();

    /* 6. Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

	/* Navigation */
	if (FAILED(Ready_Navigation()))
		return E_FAIL;

    return S_OK;
}

void CMonster_MoriblinSword::Priority_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

    CPawnObject::Priority_Update(_fTimeDelta);
}

void CMonster_MoriblinSword::Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	// refactor
	if (m_tDeleteTime.bActive)
	{
		m_tDeleteTime.fAccTime += _fTimeDelta;
		if (m_tDeleteTime.fAccTime >= m_tDeleteTime.fAccDurationTime)
		{
			this->Set_Active(false);
			Collider_BodyEnable(false);
			Collider_SensorEnable(false);
			Collider_ShieldEnable(false);
			Collider_WeaponEnable(false);
		}
	}

	/* PawnObject b*/
    CPawnObject::Update(_fTimeDelta);

    /* 6. Collider (Socket) */
    if (FAILED(Attach_Socket()))
        return;

    /* Collider */
    if (m_pBodyCollider)
        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    if (m_pSensorCollider)
        m_pSensorCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	/* Navigation */
	if (m_pNavCom)
		m_pTransformCom->Set_State(STATE::POSITION, m_pNavCom->SetUp_OnNavigation(m_pTransformCom->Get_State(STATE::POSITION)));
}

void CMonster_MoriblinSword::Late_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

    CPawnObject::Late_Update(_fTimeDelta);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;

	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::SHADOW, this)))
		return;
}

HRESULT CMonster_MoriblinSword::Render()
{
	if (!m_bActive)
		return S_OK;

    if (FAILED(CPawnObject::Render()))
        return E_FAIL;

#ifdef _DEBUG
    //if (m_pWeaponCollider) m_pWeaponCollider->Render();
    //if (m_pShieldCollider) m_pShieldCollider->Render();
    //if (m_pBodyCollider) m_pBodyCollider->Render();
    //if (m_pSensorCollider) m_pSensorCollider->Render();
	//if (m_pNavCom) m_pNavCom->Render();
#endif

    return S_OK;
}

void CMonster_MoriblinSword::Collider_BodyEnable(_bool _bActive)
{
    if (m_pBodyCollider)
        m_pBodyCollider->Set_Active(_bActive);
}

void CMonster_MoriblinSword::Collider_WeaponEnable(_bool _bActive)
{
    if (m_pWeaponCollider)
        m_pWeaponCollider->Set_Active(_bActive);
}

void CMonster_MoriblinSword::Collider_ShieldEnable(_bool _bActive)
{
    if (m_pShieldCollider)
        m_pShieldCollider->Set_Active(_bActive);
}

void CMonster_MoriblinSword::Collider_SensorEnable(_bool _bActive)
{
    if (m_pSensorCollider)
        m_pSensorCollider->Set_Active(_bActive);
}

void CMonster_MoriblinSword::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
    if (_pSrc->Get_Group() == GROUP::MONSTER)
    {
        if (_Dst->Get_Group() == GROUP::WEAPON_PLAYER)
        {
			// KnockBack
			CTransform* pTargetTransform = static_cast<CTransform*>(_Dst->Get_Owner()->Get_Component(TEXT("Com_Transform")));
			_vector vWeaponPos = pTargetTransform->Get_State(STATE::POSITION);
			Take_Damage(1.f, vWeaponPos);
        }
    }

    if (_pSrc->Get_Group() == GROUP::SENSOR_MONSTER)
    {
        if (_Dst->Get_Group() == GROUP::PLAYER)
        {
            Set_Target(_Dst->Get_Owner());
        }
    }

	if (_pSrc->Get_Group() == GROUP::WEAPON_MONSTER)
	{
		if (_Dst->Get_Group() == GROUP::SHIELD_PLAYER)
		{
			RequestToChangeState(ENUM_TO_UINT(STATE_ID::PIYO));
		}
	}
}

void CMonster_MoriblinSword::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CMonster_MoriblinSword::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
}

_float CMonster_MoriblinSword::Compute_Distance_To_Target()
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

HRESULT CMonster_MoriblinSword::Ready_Collider()
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

	/* Collider (Weapon) */
	CBounding_OBB::OBB_DESC tOBBDesc = {};
	tOBBDesc.vAngles = _float3(0.f, 0.f, 0.f);

	tOBBDesc.vExtents = _float3(0.2f, 0.2f, 0.8f);
	tOBBDesc.vCenter = _float3(0.f, 0.f, -0.5f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_WeaponCollider"),
		reinterpret_cast<CComponent**>(&m_pWeaponCollider),
		&tOBBDesc)))
		return E_FAIL;

	m_pWeaponCollider->Set_Owner(this);
	m_pWeaponCollider->Set_Group(GROUP::WEAPON_MONSTER);
	m_pWeaponCollider->Set_Active(false);
	m_pGameInstance->Register_Collider(m_pWeaponCollider);

	/* Collider (Shield) */
	tOBBDesc.vExtents = _float3(0.2f, 0.5f, 0.5f);
	tOBBDesc.vCenter = _float3(-0.1f, 0.f, 0.f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_ShieldCollider"),
		reinterpret_cast<CComponent**>(&m_pShieldCollider),
		&tOBBDesc)))
		return E_FAIL;

	m_pShieldCollider->Set_Owner(this);
	m_pShieldCollider->Set_Group(GROUP::SHIELD_MONSTER);
	m_pShieldCollider->Set_Active(false);
	m_pGameInstance->Register_Collider(m_pShieldCollider);

	/* Collider (Sensor) */
	CBounding_Sphere::SPHERE_DESC tSphereDesc = {};
	tSphereDesc.fRadius = 3.f;
	tSphereDesc.vCenter = _float3(0.f, 0.f, 0.f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_SPHERE"),
		TEXT("Com_SensorCollider"),
		reinterpret_cast<CComponent**>(&m_pSensorCollider),
		&tSphereDesc)))
		return E_FAIL;
	m_pSensorCollider->Set_Owner(this);
	m_pSensorCollider->Set_Group(GROUP::SENSOR_MONSTER);
	m_pSensorCollider->Set_Active(true);
	m_pGameInstance->Register_Collider(m_pSensorCollider);

	return S_OK;
}

HRESULT CMonster_MoriblinSword::Attach_Socket()
{
	if (!m_pModelCom)
		return S_FALSE;

	_matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	/* Weapon : hand_R */
	if (m_pWeaponCollider)
	{
		const _float4x4* pBone = m_pModelCom->Get_BoneMatrixPtr("attach_L");

		if (pBone)
		{
			_matrix Bone = XMLoadFloat4x4(pBone);
			_matrix Final = Bone * WorldMatrix;
			m_pWeaponCollider->Update(Final);
		}
	}

	/* Shield : hand_L */
	if (m_pShieldCollider)
	{
		const _float4x4* pBone = m_pModelCom->Get_BoneMatrixPtr("attach_R");

		if (pBone)
		{
			_matrix Bone = XMLoadFloat4x4(pBone);
			_matrix Final = Bone * WorldMatrix;
			m_pShieldCollider->Update(Final);
		}
	}

	return S_OK;
}

HRESULT CMonster_MoriblinSword::Ready_Navigation()
{
	CNavigation::NAVIGATION_DESC tNavDesc = {};
	tNavDesc.iCurrentCellIndex = 35;

	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_MARTHAsBAY_MONSTER"),
		TEXT("Com_Navigation"),
		reinterpret_cast<CComponent**>(&m_pNavCom),
		&tNavDesc)))
		return E_FAIL;

	//_int iCell = m_pNavCom->Find_CurrentCell(m_pTransformCom->Get_State(STATE::POSITION));
	//m_pNavCom->Set_CurrentCell(iCell);

	return S_OK;
}

void CMonster_MoriblinSword::Take_Damage(_float _fDamage, _vector _vHitPos)
{
	if (m_fHP < 0.f)
		return;

	m_fHP -= _fDamage;

	if (m_fHP <= 0.f)
	{
		Start_Dissolve();
		Start_KnockBack(_vHitPos, 10.f, 0.5f);
		m_tDeleteTime.bActive = true;
		RequestToChangeState(ENUM_TO_UINT(STATE_ID::DEAD));
	}
	else
	{
		Start_HitFlash();
		Start_KnockBack(_vHitPos, 8.f, 0.8f);
		RequestToChangeState(ENUM_TO_UINT(STATE_ID::DAMAGE));
	}
}

CMonster_MoriblinSword* CMonster_MoriblinSword::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CMonster_MoriblinSword* pInstance = new CMonster_MoriblinSword(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMonster_MoriblinSword");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMonster_MoriblinSword::Clone(void* _pArg)
{
	CMonster_MoriblinSword* pInstance = new CMonster_MoriblinSword(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster_MoriblinSword");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMonster_MoriblinSword::Free()
{
	__super::Free();

	Safe_Release(m_pWeaponCollider);
	Safe_Release(m_pShieldCollider);
	Safe_Release(m_pSensorCollider);
}
