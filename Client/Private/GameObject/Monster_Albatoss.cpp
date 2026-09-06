#include "GameObject/Monster_Albatoss.h"
#include "GameInstance.h"
#include "GameObject/Albatoss_Pop.h"
#include "GameObject/Albatoss_ATK_Claw.h"
#include "GameObject/Albatoss_ATK_Flapping.h"
#include "GameObject/Albatoss_Damage.h"
#include "GameObject/Albatoss_Dead.h"
#include "GameObject/Albatoss_Hovering.h"
#include "GameObject/Albatoss_IDLE.h"
#include "GameObject/Albatoss_Rush.h"
#include "GameObject/Albatoss_PiccoloWait.h"
#include "GameObject/Albatoss_Feather.h"
#include "GameObject/Rupee.h"

CMonster_Albatoss::CMonster_Albatoss(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CPawnObject{ _pDevice, _pDeviceContext }
{
}

CMonster_Albatoss::CMonster_Albatoss(const CMonster_Albatoss& _Prototype)
	: CPawnObject(_Prototype)
{
}

HRESULT CMonster_Albatoss::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster_Albatoss::Initialize(void* _pArg)
{
	/* 1. Pawn Object */
	if (FAILED(CPawnObject::Initialize(_pArg)))
		return E_FAIL;

	/* 5. Init State */
	Register_State(new CAlbatoss_Pop());
	Register_State(new CAlbatoss_ATK_Claw());
	Register_State(new CAlbatoss_ATK_Flapping());
	Register_State(new CAlbatoss_Damage());
	Register_State(new CAlbatoss_Dead());
	Register_State(new CAlbatoss_Hovering());
	Register_State(new CAlbatoss_IDLE());
	Register_State(new CAlbatoss_Rush());
	Register_State(new CAlbatoss_PiccoloWait());

	RequestToChangeState(ENUM_TO_UINT(STATE_ID::PICCOLO_WAIT));
	Change_State();

	/* 6. Collider */
	if (FAILED(Ready_Collider()))
		return E_FAIL;
	
	/* Add Prototype (Feather) */
	/* Client (Need), MapTool (Nedd X) */
	CModel* pModel = CModel::Create(m_pDevice, m_pDeviceContext,
	"../../Resources/Models/NonAnim/Weapon_AlbatossFeather/AlbatossFeather.mesh");
	if (!pModel)
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(
		m_ObjectDesc.iLevel,
		TEXT("Prototype_Component_Model_Map_AlbatossFeather"),
		pModel)))
	{
		Safe_Release(pModel);
	}
	
	return S_OK;
}

void CMonster_Albatoss::Priority_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	CPawnObject::Priority_Update(_fTimeDelta);
}

void CMonster_Albatoss::Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;
	
	if (m_tDeleteTime.bActive)
	{
		m_tDeleteTime.fAccTime += _fTimeDelta;

		_float fRatio = m_tDeleteTime.fAccTime / m_tDeleteTime.fAccDurationTime;
		if (fRatio >= 0.8f)
		{
			if (!m_tDissovleTime.bActive)
			{
				Start_DeleteVFX();
				
				CGameInstance::GetInstance()->Play_Sound(L"ENEMY_Boss_Delete.wav", SOUND::EFFECT, 2.5f);
				m_tDissovleTime.bActive = true;
			}
		}

		if (m_tDeleteTime.fAccTime >= m_tDeleteTime.fAccDurationTime)
		{
			this->Set_RenderActive(false);
			m_tRupee.bActive = true;
			m_tDeleteTime.bActive = false;
		}
	}

	if (m_tRupee.bActive)
	{
		m_tRupee.fAccTime += _fTimeDelta;
		if (m_tRupee.fAccTime >= m_tRupee.fAccDurationTime)
		{
			Drop_Rupee();
			this->Set_Active(false);
		}
	}

	/* 3. PawnObject */
	CPawnObject::Update(_fTimeDelta);

	/* 6. Collider (Socket) */
	if (FAILED(Attach_Socket()))
		return;
	if (m_pBodyCollider)
		m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	if (m_pWindCollider)
		m_pWindCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CMonster_Albatoss::Late_Update(_float _fTimeDelta)
{
	if (!m_bActive)
		return;

	CPawnObject::Late_Update(_fTimeDelta);

	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
		return;

	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::SHADOW, this)))
		return;
}

HRESULT CMonster_Albatoss::Render()
{
	if (!m_bActive)
		return S_OK;

	if (FAILED(CPawnObject::Render()))
		return E_FAIL;

#ifdef _DEBUG
	//if (m_pBodyCollider) m_pBodyCollider->Render();
	//if (m_pWindCollider) m_pWindCollider->Render();
	//if (m_pClawCollider_L) m_pClawCollider_L->Render();
	//if (m_pClawCollider_R) m_pClawCollider_R->Render();
#endif

	return S_OK;
}

void CMonster_Albatoss::Collider_BodyEnable(_bool _bActive)
{
	if (m_pBodyCollider)
		m_pBodyCollider->Set_Active(_bActive);
}

void CMonster_Albatoss::Collider_WindEnable(_bool _bActive)
{
	if (m_pWindCollider)
		m_pWindCollider->Set_Active(_bActive);
}

void CMonster_Albatoss::Collider_ClawEnable(_bool _bActive)
{
	if (m_pClawCollider_L)
		m_pClawCollider_L->Set_Active(_bActive);
	if (m_pClawCollider_R)
		m_pClawCollider_R->Set_Active(_bActive);
}

void CMonster_Albatoss::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
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

void CMonster_Albatoss::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CMonster_Albatoss::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
}

void CMonster_Albatoss::Shoot_Feather(_vector _vTargetPos)
{
	if (!m_pFeather)
		return;

	m_pFeather->Shoot_Target(_vTargetPos);
	
	m_pFeather = nullptr;
}

void CMonster_Albatoss::Respawn()
{
	/* 1. Feather */
	CAlbatoss_Feather::FEATHER_INIT_DESC tDesc = {};

	CGameObject::OBJECT_DESC tObjectDesc = {};
	SET_DESC(tObjectDesc.ObjectID, TEXT("AlbatossFeather"));
	tObjectDesc.iLevel = m_ObjectDesc.iLevel;
	SET_DESC(tObjectDesc.LayerTag, m_ObjectDesc.LayerTag);
	SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Actor_Weapon_AlbatossFeather"));
	SET_DESC(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	SET_DESC(tObjectDesc.ModelTag, TEXT("Prototype_Component_Model_Map_AlbatossFeather"));
	tDesc.tObjectDesc = tObjectDesc;

	_matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	for (_int i = 0; i < 3; ++i)
	{
		WorldMatrix.r[i] = XMVector3Normalize(WorldMatrix.r[i]);
	}
	XMStoreFloat4x4(&tDesc.tFeatherDesc.WorldMatrix, WorldMatrix);

	CGameObject* pGameObject = { nullptr };
	m_pGameInstance->Add_GameObject_ToLayer(
		RESOURCE_LEVEL_STATIC,
		tDesc.tObjectDesc.ObjectTag,
		tDesc.tObjectDesc.iLevel,
		tDesc.tObjectDesc.LayerTag,
		&tDesc, &pGameObject);
	m_pFeather = static_cast<CAlbatoss_Feather*>(pGameObject);
}

CMonster_Albatoss::PATTERN CMonster_Albatoss::Get_CurrentPattern() const
{
	return m_PatternQueue[m_iPatternIndex];
}

void CMonster_Albatoss::NextPattern()
{
	m_iPatternIndex++;

	if (m_iPatternIndex >= m_PatternQueue.size())
		m_iPatternIndex = 0;
}

_uint CMonster_Albatoss::Get_PreState() const
{
	return m_iPreState;
}

_bool CMonster_Albatoss::IsDead() const
{
	return m_pState->Get_StateID() == ENUM_TO_UINT(STATE_ID::DEAD);
}

void CMonster_Albatoss::Notify_Delete()
{
	m_tDeleteTime.bActive = true;
}

_float CMonster_Albatoss::Get_LastRushDir() const
{
	return m_fLastRushDir;
}

void CMonster_Albatoss::Set_LastRushDir(_float _fDir)
{
	m_fLastRushDir =_fDir;
}

HRESULT CMonster_Albatoss::Ready_Collider()
{
	/* Collider (Body) */
	CBounding_Sphere::SPHERE_DESC tSphereDesc = {};
	tSphereDesc.fRadius = 1.5f;
	tSphereDesc.vCenter = _float3(0.f, tSphereDesc.fRadius, 0.f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_SPHERE"),
		TEXT("Com_BodyCollider"),
		reinterpret_cast<CComponent**>(&m_pBodyCollider),
		&tSphereDesc)))
		return E_FAIL;

	m_pBodyCollider->Set_Owner(this);
	m_pBodyCollider->Set_Group(GROUP::MONSTER);
	m_pBodyCollider->Set_Active(true);
	m_pGameInstance->Register_Collider(m_pBodyCollider);

	/* Collider (Wind) */
	CBounding_Sphere::SPHERE_DESC tSphereWindDesc = {};
	tSphereWindDesc.fRadius = 6.f;
	tSphereWindDesc.vCenter = _float3(0.f, tSphereWindDesc.fRadius, 0.f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_SPHERE"),
		TEXT("Com_WindCollider"),
		reinterpret_cast<CComponent**>(&m_pWindCollider),
		&tSphereWindDesc)))
		return E_FAIL;

	m_pWindCollider->Set_Owner(this);
	m_pWindCollider->Set_Group(GROUP::WIND);
	m_pWindCollider->Set_Active(false);
	m_pGameInstance->Register_Collider(m_pWindCollider);

	/* Collider (Claw_L) */
	CBounding_OBB::OBB_DESC tOBBDesc_ClawL = {};
	tOBBDesc_ClawL.vAngles = _float3(0.f, 0.f, 0.f);
	tOBBDesc_ClawL.vExtents = _float3(0.6f, 0.3f, 0.3f);
	tOBBDesc_ClawL.vCenter = _float3(0.3f, 0.f, 0.f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_ClawCollider_L"),
		reinterpret_cast<CComponent**>(&m_pClawCollider_L),
		&tOBBDesc_ClawL)))
		return E_FAIL;
	m_pClawCollider_L->Set_Owner(this);
	m_pClawCollider_L->Set_Group(GROUP::WEAPON_MONSTER);
	m_pClawCollider_L->Set_Active(false);
	m_pGameInstance->Register_Collider(m_pClawCollider_L);

	/* Collider (Claw_R) */
	CBounding_OBB::OBB_DESC tOBBDesc_ClawR = {};
	tOBBDesc_ClawR.vAngles = _float3(0.f, 0.f, 0.f);
	tOBBDesc_ClawR.vExtents = _float3(0.6f, 0.3f, 0.3f);
	tOBBDesc_ClawR.vCenter = _float3(0.3f, 0.f, 0.f);
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_ClawCollider_R"),
		reinterpret_cast<CComponent**>(&m_pClawCollider_R),
		&tOBBDesc_ClawR)))
		return E_FAIL;
	m_pClawCollider_R->Set_Owner(this);
	m_pClawCollider_R->Set_Group(GROUP::WEAPON_MONSTER);
	m_pClawCollider_R->Set_Active(false);
	m_pGameInstance->Register_Collider(m_pClawCollider_R);

	return S_OK;
}

HRESULT CMonster_Albatoss::Attach_Socket()
{
	if (!m_pModelCom)
		return S_FALSE;

	_matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	/* Claw_L */
	if (m_pClawCollider_L)
	{
		const _float4x4* pBone = m_pModelCom->Get_BoneMatrixPtr("leg_L_d");

		if (pBone)
		{
			_matrix Bone = XMLoadFloat4x4(pBone);
			_matrix Final = Bone * WorldMatrix;
			m_pClawCollider_L->Update(Final);
		}
	}

	/* Claw_R */
	if (m_pClawCollider_R)
	{
		const _float4x4* pBone = m_pModelCom->Get_BoneMatrixPtr("leg_R_d");

		if (pBone)
		{
			_matrix Bone = XMLoadFloat4x4(pBone);
			_matrix Final = Bone * WorldMatrix;
			m_pClawCollider_R->Update(Final);
		}
	}

	/* Feather */
	if (m_pFeather)
	{
		const _float4x4* pBone = m_pModelCom->Get_BoneMatrixPtr("wing_L_b");

		if (pBone)
		{
			_matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
			_matrix BoneMatrix = XMLoadFloat4x4(pBone);

			_float4x4 FinalMatrix;
			XMStoreFloat4x4(&FinalMatrix, BoneMatrix * WorldMatrix);

			m_pFeather->Update_WorldMatrix(FinalMatrix);
		}
	}

	return S_OK;
}

void CMonster_Albatoss::Take_Damage(_float _fDamage, _vector _vHitPos)
{
	if (m_fHP < 0.f)
		return;

	m_fHP -= _fDamage;

	/* CHANGE PATTERN */
	if (m_fHP < 2.f)
	{
		m_pTransformCom->Set_SpeedMulti(5.f);

		m_PatternQueue = {
			PATTERN::RUSH,
			PATTERN::RUSH,
			PATTERN::CLAW,
			PATTERN::FLAPPING
		};
	}

	/* STATE */
	if (m_fHP <= 0.f)
	{
		RequestToChangeState(ENUM_TO_UINT(STATE_ID::DEAD));
		//Start_DeleteVFX();
		//Drop_Rupee();
		return;
	}

	_uint iCurState = m_pState->Get_StateID();
	if (iCurState == ENUM_TO_UINT(STATE_ID::RUSH))
	{
		// TODO : EFFECT
		return;
	}

	if (m_pState->Get_StateID() != ENUM_TO_UINT(STATE_ID::DAMAGE))
	{
		m_iPreState = m_pState->Get_StateID();
	}
	Start_HitFlash();
	RequestToChangeState(ENUM_TO_UINT(STATE_ID::DAMAGE));
}

void CMonster_Albatoss::Drop_Rupee()
{
	_int iCount = 50;
	
	for (_int i = 0; i < iCount; ++i)
	{
		_float x = -3.f + (rand() % 600) * 0.01f; // -3 ~ 3
		_float y = 1.f;
		_float z = 0.3f;

		/* Init Position */
		_vector vSpawnPos = XMVectorSet(x, y, z, 1.f);

		/* Burst */
		_vector vDir = XMVectorSet(
			x * 0.3f,                  
			1.5f + (rand() % 100) * 0.01f,
			0.f,
			0.f
		);
		vDir = XMVector3Normalize(vDir);

		Spawn_Rupee(vSpawnPos, vDir);
	}
}

void CMonster_Albatoss::Spawn_Rupee(_vector vPos, _vector vDir)
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

void CMonster_Albatoss::Start_DeleteVFX()
{
	/* Effect Pos */
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	//vPos += XMVectorSet(-0.5f, 1.5f, -2.f, 0.f); // For. Flapping
	vPos += XMVectorSet(0.f, 1.5f, 0.f, 0.f); // For. Claw

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
	tEvent.fScale = 5.f;
	m_pGameInstance->Push_EffectEVENT(tEvent);

	//m_tDeleteTime.bActive = true;
}

CMonster_Albatoss* CMonster_Albatoss::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CMonster_Albatoss* pInstance = new CMonster_Albatoss(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMonster_Albatoss");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMonster_Albatoss::Clone(void* _pArg)
{
	CMonster_Albatoss* pInstance = new CMonster_Albatoss(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster_Albatoss");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMonster_Albatoss::Free()
{
	__super::Free();

	Safe_Release(m_pClawCollider_L);
	Safe_Release(m_pClawCollider_R);
	Safe_Release(m_pWindCollider);
	Safe_Release(m_pBodyCollider);
}