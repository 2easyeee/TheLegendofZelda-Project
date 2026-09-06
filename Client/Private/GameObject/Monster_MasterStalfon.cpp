#include "GameObject/Monster_MasterStalfon.h"
#include "GameInstance.h"
#include "GameObject/MasterStalfon_IDLE.h"
#include "GameObject/MasterStalfon_Attack.h"
#include "GameObject/MasterStalfon_Down.h"
#include "GameObject/MasterStalfon_Walk.h"
#include "GameObject/MasterStalfon_Damage.h"
#include "GameObject/MasterStalfon_Dead.h"
#include "GameObject/MasterStalfon_Pop.h"
#include "GameObject/MasterStalfon_Escape.h"
#include "GameObject/MasterStalfon_Guard.h"
#include "GameObject/MasterStalfon_Jump.h"
#include "GameObject/MasterStalfon_KyoroKyoro.h"
#include "GameObject/MasterStalfon_Follow.h"
#include "GameObject/MasterStalfon_Teleport.h"
#include "GameObject/MasterStalfon_Pop_Wait.h"
#include "GameObject/Rupee.h"
#include "MapObject.h"

CMonster_MasterStalfon::CMonster_MasterStalfon(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CPawnObject { _pDevice, _pDeviceContext }
{
}

CMonster_MasterStalfon::CMonster_MasterStalfon(const CMonster_MasterStalfon& _Prototype)
	: CPawnObject(_Prototype)
{
}

HRESULT CMonster_MasterStalfon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster_MasterStalfon::Initialize(void* _pArg)
{
	/* 1. Pawn Object */
	if (FAILED(CPawnObject::Initialize(_pArg)))
		return E_FAIL;

	/* 5. Init State */
	Register_State(new CMasterStalfon_IDLE());
	Register_State(new CMasterStalfon_Walk());
	Register_State(new CMasterStalfon_Attack());
	Register_State(new CMasterStalfon_Damage());
	Register_State(new CMasterStalfon_Down());
	Register_State(new CMasterStalfon_Dead());
	Register_State(new CMasterStalfon_Pop());
	Register_State(new CMasterStalfon_Escape());
	Register_State(new CMasterStalfon_Guard());
	Register_State(new CMasterStalfon_Jump());
	Register_State(new CMasterStalfon_KyoroKyoro());
	Register_State(new CMasterStalfon_Follow());
	Register_State(new CMasterStalfon_Teleport());
	Register_State(new CMasterStalfon_Pop_Wait());

	RequestToChangeState(ENUM_TO_UINT(STATE_ID::POP));
	Change_State();

	/* 6. Collider */
	if (FAILED(Ready_Collider()))
		return E_FAIL;

	/* Navigation */
	if (FAILED(Ready_Navigation()))
		return E_FAIL;

	return S_OK;
}

void CMonster_MasterStalfon::Priority_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	CPawnObject::Priority_Update(_fTimeDelta);
}

void CMonster_MasterStalfon::Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	// refactor
	if (m_tDeleteTime.bActive)
	{
		m_tDeleteTime.fAccTime += _fTimeDelta;

		_float fRatio = m_tDeleteTime.fAccTime / m_tDeleteTime.fAccDurationTime;
		if (fRatio >= 0.5f && !m_bDissolveStarted)
		{
			Start_Dissolve();
			m_bDissolveStarted = true;
		}

		if (m_tDeleteTime.fAccTime >= m_tDeleteTime.fAccDurationTime)
		{

			/* SFX */
			if (!m_tDeleteSFX.bActive)
			{
				m_pGameInstance->Stop_Sound(SOUND::BGM);
				m_pGameInstance->Play_BGM(L"0_LastBossWin.ry.48.dspadpcm.bfstm.wav");
				m_tDeleteSFX.bActive = true;
			}

			Drop_Rupee();
			this->Set_Active(false);
			Collider_BodyEnable(false);
			Collider_SensorEnable(false);
			Collider_ShieldEnable(false);
			Collider_WeaponEnable(false);
			Collider_AttackWindowEnable(false);
		}
	}

	/* PawnObject */
	CPawnObject::Update(_fTimeDelta);

	/* Navigation */
	if (m_pNavCom)
		m_pTransformCom->Set_State(STATE::POSITION, m_pNavCom->SetUp_OnNavigation(m_pTransformCom->Get_State(STATE::POSITION)));

	/* 6. Collider (Socket) */
	if (FAILED(Attach_Socket()))
		return;

	/* Collider */
	if (m_pBodyCollider)
		m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	if (m_pSensorCollider)
		m_pSensorCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	if (m_pAttackWindowCollider)
		m_pAttackWindowCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CMonster_MasterStalfon::Late_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	CPawnObject::Late_Update(_fTimeDelta);

	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
		return;

	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::SHADOW, this)))
		return;
}

HRESULT CMonster_MasterStalfon::Render()
{
	if (!m_bActive)
		return S_OK;

	if (!m_bRenderActive)
		return S_OK;

	if (FAILED(CPawnObject::Render()))
		return E_FAIL;

#ifdef _DEBUG
	//if (m_pWeaponCollider) m_pWeaponCollider->Render();
	//if (m_pShieldCollider) m_pShieldCollider->Render();
	//if (m_pBodyCollider) m_pBodyCollider->Render();
	//if (m_pSensorCollider) m_pSensorCollider->Render();
	//if (m_pAttackWindowCollider) m_pAttackWindowCollider->Render();
	//if (m_pNavCom) m_pNavCom->Render();
#endif

	return S_OK;
}

void CMonster_MasterStalfon::Collider_BodyEnable(_bool _bActive)
{
	if (m_pBodyCollider)
		m_pBodyCollider->Set_Active(_bActive);
}

void CMonster_MasterStalfon::Collider_WeaponEnable(_bool _bActive)
{
	if (m_pWeaponCollider)
		m_pWeaponCollider->Set_Active(_bActive);
}

void CMonster_MasterStalfon::Collider_ShieldEnable(_bool _bActive)
{
	if (m_pShieldCollider)
		m_pShieldCollider->Set_Active(_bActive);
}

void CMonster_MasterStalfon::Collider_SensorEnable(_bool _bActive)
{
	if (m_pSensorCollider)
		m_pSensorCollider->Set_Active(_bActive);
}

void CMonster_MasterStalfon::Collider_AttackWindowEnable(_bool _bActive)
{
	if (m_pAttackWindowCollider)
		m_pAttackWindowCollider->Set_Active(_bActive);
}

void CMonster_MasterStalfon::Render_Enable(_bool _bActive)
{
	m_bRenderActive = _bActive;
}

void CMonster_MasterStalfon::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
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
}

void CMonster_MasterStalfon::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
	if (_pSrc->Get_Group() == GROUP::SENSOR_MONSTER)
	{
		if (_Dst->Get_Group() == GROUP::PLAYER)
		{
			Set_Target(_Dst->Get_Owner());
		}
	}
}

void CMonster_MasterStalfon::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
}

_float CMonster_MasterStalfon::Compute_Distance_To_Target()
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

CMonster_MasterStalfon::PATTERN CMonster_MasterStalfon::Get_CurrentPattern() const
{
	return m_PatternQueue[m_iPatternIndex];
}

void CMonster_MasterStalfon::NextPattern()
{
	m_iPatternIndex++;

	if (m_iPatternIndex >= m_PatternQueue.size())
		m_iPatternIndex = 0;
}

void CMonster_MasterStalfon::Run_CurrentPattern()
{
	switch (Get_CurrentPattern())
	{
	case PATTERN::GUARD:
		RequestToChangeState(ENUM_TO_UINT(STATE_ID::GUARD));
		break;

	case PATTERN::ATTACK:
		RequestToChangeState(ENUM_TO_UINT(STATE_ID::FOLLOW));
		break;

	case PATTERN::JUMP:
		RequestToChangeState(ENUM_TO_UINT(STATE_ID::JUMP));
		break;

	case PATTERN::ESCAPE:
		RequestToChangeState(ENUM_TO_UINT(STATE_ID::ESCAPE));
		break;

	case PATTERN::TELEPORT:
		RequestToChangeState(ENUM_TO_UINT(STATE_ID::TELEPORT));
		break;
	}
}

_uint CMonster_MasterStalfon::Get_PreState() const
{
	return m_iPreState;
}

void CMonster_MasterStalfon::Notify_CameraShake()
{
	m_bCameraShake = true;
}

_bool CMonster_MasterStalfon::Check_CameraShake()
{
	if (!m_bCameraShake)
		return false;

	m_bCameraShake = false;
	return true;
}

HRESULT CMonster_MasterStalfon::Ready_Collider()
{
	/* Collider (Body) */
	CBounding_AABB::AABB_DESC tAABBDesc = {};
	tAABBDesc.vExtents = _float3(0.7f, 1.f, 0.7f);
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
	m_pBodyCollider->Set_Active(false);
	m_pGameInstance->Register_Collider(m_pBodyCollider);

	/* Collider (Weapon / Shield) */
	CBounding_OBB::OBB_DESC tOBBDesc = {};
	tOBBDesc.vAngles = _float3(0.f, 0.f, 0.f);

	tOBBDesc.vExtents = _float3(0.2f, 1.2f, 0.2f);
	tOBBDesc.vCenter = _float3(0.f, -1.f, 0.f);
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

	// Shield
	tOBBDesc.vExtents = _float3(0.8f, 0.8f, 0.15f);
	tOBBDesc.vCenter = _float3(0.2f, 0.f, 0.25f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_ShieldCollider"),
		reinterpret_cast<CComponent**>(&m_pShieldCollider),
		&tOBBDesc)))
		return E_FAIL;

	m_pShieldCollider->Set_Owner(this);
	m_pShieldCollider->Set_Group(GROUP::SHIELD_MONSTER);
	m_pShieldCollider->Set_Active(true);
	m_pGameInstance->Register_Collider(m_pShieldCollider);

	/* Collider (Sensor) */
	CBounding_Sphere::SPHERE_DESC tSphereDesc = {};
	tSphereDesc.fRadius = 5.f;
	tSphereDesc.vCenter = _float3(0.f, tSphereDesc.fRadius, 0.f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_SPHERE"),
		TEXT("Com_SensorCollider"),
		reinterpret_cast<CComponent**>(&m_pSensorCollider),
		&tSphereDesc)))
		return E_FAIL;
	m_pSensorCollider->Set_Owner(this);
	m_pSensorCollider->Set_Group(GROUP::SENSOR_MONSTER);
	m_pSensorCollider->Set_Active(false);
	m_pGameInstance->Register_Collider(m_pSensorCollider);

	/* Collider (AttackWindow) */
	CBounding_Sphere::SPHERE_DESC tSphereDesc_AttackWindow = {};
	tSphereDesc_AttackWindow.fRadius = 7.f;
	tSphereDesc_AttackWindow.vCenter = _float3(0.f, tSphereDesc_AttackWindow.fRadius, 0.f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_SPHERE"),
		TEXT("Com_AttackWindowCollider"),
		reinterpret_cast<CComponent**>(&m_pAttackWindowCollider),
		&tSphereDesc_AttackWindow)))
		return E_FAIL;
	m_pAttackWindowCollider->Set_Owner(this);
	m_pAttackWindowCollider->Set_Group(GROUP::ATTACK_WINDOW_MONSTER);
	m_pAttackWindowCollider->Set_Active(false);
	m_pGameInstance->Register_Collider(m_pAttackWindowCollider);

	return S_OK;
}

HRESULT CMonster_MasterStalfon::Attach_Socket()
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

HRESULT CMonster_MasterStalfon::Ready_Navigation()
{
	CNavigation::NAVIGATION_DESC tNavDesc = {};
	tNavDesc.iCurrentCellIndex = 35;

	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_Dungeon_MONSTER_BOSS"),
		TEXT("Com_Navigation"),
		reinterpret_cast<CComponent**>(&m_pNavCom),
		&tNavDesc)))
		return E_FAIL;

	_int iCell = m_pNavCom->Find_CurrentCell(m_pTransformCom->Get_State(STATE::POSITION));
	m_pNavCom->Set_CurrentCell(iCell);

	return S_OK;
}

void CMonster_MasterStalfon::Take_Damage(_float _fDamage, _vector _vHitPos)
{
	if (m_fHP < 0.f)
		return;

	m_fHP -= _fDamage;

	if (m_fHP <= 5.f)
	{
		/* Patter2 */
		m_PatternQueue = {
			PATTERN::JUMP,
			PATTERN::ATTACK,
			PATTERN::ESCAPE,
			PATTERN::ATTACK,
			PATTERN::JUMP,
			PATTERN::ESCAPE,
			PATTERN::ATTACK
		};
	}

	if (m_fHP <= 0.f)
	{
		Start_KnockBack(_vHitPos, 5.f, 0.5f);
		RequestToChangeState(ENUM_TO_UINT(STATE_ID::DEAD));
		m_tDeleteTime.bActive = true;
		m_tDeleteTime.fAccTime = 0.f;
	}
	else
	{		
		auto StateID = m_pState->Get_StateID();

		Start_HitFlash();
		if (StateID != ENUM_TO_UINT(STATE_ID::DOWN))
			RequestToChangeState(ENUM_TO_UINT(STATE_ID::DOWN));
	}
}

void CMonster_MasterStalfon::Drop_Rupee()
{
	_int iCount = 70;
	_float fRadiusX = 10.f;
	_float fRadiusZ = 10.f;
	_vector vOrigin = m_pTransformCom->Get_State(STATE::POSITION);

	for (_int i = 0; i < iCount; ++i)
	{
		_float angle = XM_2PI * ((rand() % 1000) * 0.001f);
		_float radius = fRadiusX * sqrtf((rand() % 1000) * 0.001f);

		_float x = cosf(angle);
		_float z = sinf(angle);

		/* Init Position */
		_vector vSpawnPos = vOrigin + XMVectorSet(0.f, 1.f, 0.f, 0.f);

		/* Burst */
		_vector vDir = XMVectorSet(x, 1.5f + (rand() % 100) * 0.01f, z, 0.f);
		vDir = XMVector3Normalize(vDir);

		Spawn_Rupee(vSpawnPos, vDir);
	}
}

void CMonster_MasterStalfon::Spawn_Rupee(_vector vPos, _vector vDir)
{
	CRupee::RUPEE_TYPE eType;
	_int r = rand() % 100;
	if (r < 60)        eType = CRupee::RUPEE_TYPE::GREEN;
	else if (r < 90)   eType = CRupee::RUPEE_TYPE::PURPLE;
	else               eType = CRupee::RUPEE_TYPE::RED;

	CGameObject::OBJECT_DESC tObjectDesc = {};
	SET_DESC(tObjectDesc.ObjectID, TEXT("Rupee"));
	tObjectDesc.iLevel = RESOURCE_LEVEL_STATIC;
	SET_DESC(tObjectDesc.LayerTag, TEXT("Layer_Rupee"));
	SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Map_Rupee"));
	SET_DESC(tObjectDesc.ModelTag, TEXT("Prototype_Component_Model_Map_RupeeGreen"));

	_float3 vPos3;
	XMStoreFloat3(&vPos3, vPos);

	CWorldObject::WORLD_DESC tWorldDesc = {};
	tWorldDesc.vPosition = vPos3;

	CMapObject::MAP_INIT_DESC tInitDesc = {};
	tInitDesc.tObjectDesc = tObjectDesc;
	tInitDesc.tWorldDesc = tWorldDesc;

	CGameObject* pGameObject = nullptr;

	m_pGameInstance->Add_GameObject_ToLayer(
		RESOURCE_LEVEL_STATIC,
		tObjectDesc.ObjectTag,
		ENUM_TO_UINT(LEVEL::DUNGEON),
		TEXT("Layer_Rupee"),
		&tInitDesc,
		&pGameObject);

	auto pRupee = static_cast<CRupee*>(pGameObject);
	pRupee->Set_Type(eType);
	pRupee->Set_Burst(vDir * (3.f + rand() % 3));
}

void CMonster_MasterStalfon::Start_DeleteVFX()
{
	/* Effect Pos */
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	vPos += XMVectorSet(0.f, 2.f, 0.f, 1.f);

	/* Star */
	_vector basePos = vPos;
	_int iCount = 16;

	for (_int i = 0; i < iCount; ++i)
	{
		/* Effect */
		CEffectManager::EFFECT_EVENT tEvent{};
		tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::SPAWN_EFFECT;
		tEvent.EffectName = TEXT("Star");

		_float fAngle = XM_2PI * ((rand() % 1000) / 1000.f);
		_float fX = cosf(fAngle);
		_float fZ = sinf(fAngle);
		_float fY = ((rand() % 100) / 100.f) * 0.5f;

		_vector vDir = XMVector3Normalize(XMVectorSet(fX, fY, fZ, 0.f));

		_float foffsetScale = 0.1f + ((rand() % 100) / 100.f) * 0.3f;
		_vector vOffset = vDir * foffsetScale;

		tEvent.vPosition = basePos + vOffset;
		tEvent.vDirection = vDir;
		tEvent.fScale = 1.f;

		m_pGameInstance->Push_EffectEVENT(tEvent);
	}

	/* Purple_Smoke */
	CEffectManager::EFFECT_EVENT tEvent{};
	tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::SPAWN_EFFECT;
	tEvent.EffectName = TEXT("Plane");
	tEvent.vPosition = vPos;
	tEvent.fScale = 7.5f;
	m_pGameInstance->Push_EffectEVENT(tEvent);
}

CMonster_MasterStalfon* CMonster_MasterStalfon::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CMonster_MasterStalfon* pInstance = new CMonster_MasterStalfon(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMonster_MasterStalfon");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMonster_MasterStalfon::Clone(void* _pArg)
{
	CMonster_MasterStalfon* pInstance = new CMonster_MasterStalfon(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster_MasterStalfon");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMonster_MasterStalfon::Free()
{
	__super::Free();

	Safe_Release(m_pAttackWindowCollider);
	Safe_Release(m_pBodyCollider);
	Safe_Release(m_pWeaponCollider);
	Safe_Release(m_pShieldCollider);
	Safe_Release(m_pSensorCollider);
}
