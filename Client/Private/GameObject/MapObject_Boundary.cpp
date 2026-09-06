#include "GameObject/MapObject_Boundary.h"
#include "GameInstance.h"

CMapObject_Boundary::CMapObject_Boundary(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CMapObject{ _pDevice, _pDeviceContext }
{
}

CMapObject_Boundary::CMapObject_Boundary(const CMapObject_Boundary& _Prototype)
	: CMapObject(_Prototype)
{
}

HRESULT CMapObject_Boundary::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapObject_Boundary::Initialize(void* _pArg)
{
	if (FAILED(CMapObject::Initialize(_pArg)))
		return E_FAIL;

	MAP_INIT_DESC* pInitDesc = static_cast<MAP_INIT_DESC*>(_pArg);

	/* Collider */
	if (FAILED(Ready_Collider(&pInitDesc->tWorldDesc)))
		return E_FAIL;

	return S_OK;
}

void CMapObject_Boundary::Priority_Update(_float _fTimeDelta)
{
}

void CMapObject_Boundary::Update(_float _fTimeDelta)
{
	if (m_pColliderCom)
	{
		_float3 vScale = m_pTransformCom->Get_Scaled();

		m_pColliderCom->Set_Extents(_float3(
			m_vBaseExtents.x * vScale.x,
			m_vBaseExtents.y * vScale.y,
			m_vBaseExtents.z * vScale.z));

		_matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
		WorldMatrix.r[0] = XMVector3Normalize(WorldMatrix.r[0]);
		WorldMatrix.r[1] = XMVector3Normalize(WorldMatrix.r[1]);
		WorldMatrix.r[2] = XMVector3Normalize(WorldMatrix.r[2]);

		m_pColliderCom->Update(WorldMatrix);
	}
}

void CMapObject_Boundary::Late_Update(_float _fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
		return;
}

HRESULT CMapObject_Boundary::Render()
{
	if (!m_bActive)
		return S_OK;

#ifdef _DEBUG
	/* Collider */
	//if (m_pColliderCom) m_pColliderCom->Render();
#endif // _DEBUG

	return S_OK;
}

HRESULT CMapObject_Boundary::Ready_Collider(WORLD_DESC* _Desc)
{
	_float3 vScale = m_pTransformCom->Get_Scaled();
	m_vBaseExtents = _float3(
		(_Desc->vColliderExtents.x * 0.5f) / vScale.x,
		(_Desc->vColliderExtents.y * 0.5f) / vScale.y,
		(_Desc->vColliderExtents.z * 0.5f) / vScale.z);

	/* Collider */
	CBounding_AABB::AABB_DESC tAABBDesc = {};
	tAABBDesc.vExtents = m_vBaseExtents;
	tAABBDesc.vCenter = _Desc->vColliderCenter;

	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider"),
		reinterpret_cast<CComponent**>(&m_pColliderCom),
		&tAABBDesc)))
		return E_FAIL;

	m_pColliderCom->Set_Owner(this);
	m_pColliderCom->Set_Group(GROUP::BOUNDARY);
	m_pColliderCom->Set_Active(true);
	m_pGameInstance->Register_Collider(m_pColliderCom);

	return S_OK;
}

CMapObject_Boundary* CMapObject_Boundary::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CMapObject_Boundary* pInstance = new CMapObject_Boundary(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("FAILED TO CREATED : CMapObject_Boundary");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMapObject_Boundary::Clone(void* _pArg)
{
	CMapObject_Boundary* pInstance = new CMapObject_Boundary(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("FAILED TO CLONED : CMapObject_Boundary");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMapObject_Boundary::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
}
