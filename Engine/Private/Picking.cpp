#include "Picking.h"
#include "GameInstance.h"

CPicking::CPicking(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: m_pGameInstance { CGameInstance::GetInstance() }
	, m_pDevice { _pDevice }
	, m_pDeviceContext { _pDeviceContext }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pDeviceContext);
}

HRESULT CPicking::Initialize(HWND _hWnd, _uint _iWinSizeX, _uint _iWinSizeY)
{
	m_hWnd = _hWnd;
	m_iWinSizeX = _iWinSizeX;
	m_iWinSizeY = _iWinSizeY;

	return S_OK;
}

void CPicking::Update()
{
	POINT ptMouse = {};

	/* 스크린좌표상의 마우스 위치 */
	GetCursorPos(&ptMouse);

	/* 뷰포트 마우스 위치 */
	ScreenToClient(m_hWnd, &ptMouse);

	if (ptMouse.x < 0 || ptMouse.x > m_iWinSizeX ||
		ptMouse.y < 0 || ptMouse.y > m_iWinSizeY)
		return;

	/* 투영스페이스 상의 마우스 위치 */
	/* 로컬 * 월드 * 뷰 * 투영 * (/w) */
	_float3 vMousePos = {};
	vMousePos.x = ptMouse.x / (m_iWinSizeX * 0.5f) - 1.f;
	vMousePos.y = ptMouse.y / (m_iWinSizeY * -0.5f) + 1.f;
	vMousePos.z = 0.f; /* Near 평면을 클릭했기 때문에 */

	m_pGameInstance->Get_Transform(D3DTS::PROJ);
	m_pGameInstance->Get_Transform(D3DTS::VIEW);

	_matrix ViewMatrix = XMLoadFloat4x4(m_pGameInstance->Get_Transform(D3DTS::VIEW));
	_matrix ProjMatrix = XMLoadFloat4x4(m_pGameInstance->Get_Transform(D3DTS::PROJ));
	_matrix ViewInvMatrix = XMLoadFloat4x4(m_pGameInstance->Get_InvTransform(D3DTS::VIEW));
	_matrix ProjInvMatrix = XMLoadFloat4x4(m_pGameInstance->Get_InvTransform(D3DTS::PROJ));

	/* 뷰스페이스 상의 마우스 위치 */
	/* 로컬 * 월드 * 뷰 * 투영 * (/w) * (투영^-1) */
	//_vector vMouse
	_vector vMousePos_Proj = XMLoadFloat3(&vMousePos);
	_vector vMousePos_View = XMVector3TransformCoord(vMousePos_Proj, ProjInvMatrix);

	_vector vRayPos_View = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	_vector vRayDir_View = XMVector3Normalize(vMousePos_View - vRayPos_View);

	/* 월드스페이스 상의 마우스 위치 */
	/* 로컬 * 월드 * 뷰 * (뷰^-1) */
	/* 로컬 * 월드 */ 
	_vector vRayPos_World = XMVector3TransformCoord(vRayPos_View, ViewInvMatrix);
	_vector vRayDir_World = XMVector3Normalize(XMVector3TransformNormal(vRayDir_View, ViewInvMatrix));

	XMStoreFloat3(&m_vWorldRayPos, vRayPos_World);
	XMStoreFloat3(&m_vWorldRayDir, vRayDir_World);
}

void CPicking::Transform_Picking_ToLocalSpace(const _float4x4* _pWorldMatrix)
{
	/* 로컬 * 월드 * (월드^-1) */
	/* 로컬 */
	_matrix WorldMatrix = XMLoadFloat4x4(_pWorldMatrix);
	_matrix WorldInvMatrix = XMMatrixInverse(nullptr, WorldMatrix);

	_vector vPos = XMLoadFloat3(&m_vWorldRayPos);
	_vector vDir = XMLoadFloat3(&m_vWorldRayDir);

	vPos = XMVector3TransformCoord(vPos, WorldInvMatrix);
	vDir = XMVector3Normalize(XMVector3TransformNormal(vDir, WorldInvMatrix));

	XMStoreFloat3(&m_vLocalRayPos, vPos);
	XMStoreFloat3(&m_vLocalRayDir, vDir);
}

_bool CPicking::isPicked_InLocalSpace(const _float3* _pPointA, const _float3* _pPointB, const _float3* _pPointC, _float3* _pOut)
{
	_vector vRayPos = XMLoadFloat3(&m_vLocalRayPos);
	_vector vRayDir = XMLoadFloat3(&m_vLocalRayDir);

	_vector PointA = XMLoadFloat3(_pPointA);
	_vector PointB = XMLoadFloat3(_pPointB);
	_vector PointC = XMLoadFloat3(_pPointC);

	_float fDist = 0.f;

	//        inline bool XM_CALLCONV Intersects(
	// FXMVECTOR Origin, FXMVECTOR Direction, FXMVECTOR V0,
	//	GXMVECTOR V1,
	//	HXMVECTOR V2, float& Dist) noexcept
	
	if (DirectX::TriangleTests::Intersects(vRayPos, vRayDir, PointA, PointB, PointC, fDist))
	{
		if (_pOut)
		{
			_vector vHit = vRayPos + (vRayDir * fDist);
			XMStoreFloat3(_pOut, vHit);
		}
		return true;
	}
	
	return false;
}

_bool CPicking::isPicked_InWorldSpace(const _float3* _pPointA, const _float3* _pPointB, const _float3* _pPointC, _float3* _pOut, _float* _pOutDist)
{
	_vector vRayPos = XMLoadFloat3(&m_vWorldRayPos);
	_vector vRayDir = XMLoadFloat3(&m_vWorldRayDir);

	float len = XMVectorGetX(XMVector3Length(vRayDir));
	if (len < 0.0001f)
		return false;

	vRayDir = XMVectorScale(vRayDir, 1.0f / len);

	_vector A = XMLoadFloat3(_pPointA);
	_vector B = XMLoadFloat3(_pPointB);
	_vector C = XMLoadFloat3(_pPointC);

	float fDist = 0.f;

	if (DirectX::TriangleTests::Intersects(vRayPos, vRayDir, A, B, C, fDist))
	{
		if (_pOutDist)
			*_pOutDist = fDist;

		if (_pOut)
		{
			_vector vHit = vRayPos + vRayDir * fDist;
			XMStoreFloat3(_pOut, vHit);
		}
		return true;
	}

	return false;
}

_bool CPicking::Picking_PlaneY(_float _fY, _float3& _outPos)
{
	const _float3& O = m_vWorldRayPos;
	const _float3& D = m_vWorldRayDir;

	if (fabs(D.y) < 0.0001f)
		return false;

	_float t = (_fY - O.y) / D.y;
	if (t < 0.f)
		return false;

	_outPos.x = O.x + D.x * t;
	_outPos.y = _fY;
	_outPos.z = O.z + D.z * t;

	return true;
}

_bool CPicking::isPicked_AABB(const _float3& vMin, const _float3& vMax, _float3* _pOutHitPos)
{
	float tMin = 0.f;
	float tMax = FLT_MAX;

	const float* rayPos = &m_vLocalRayPos.x;
	const float* rayDir = &m_vLocalRayDir.x;
	const float* boxMin = &vMin.x;
	const float* boxMax = &vMax.x;

	for (int i = 0; i < 3; ++i)
	{
		if (fabs(rayDir[i]) < 1e-6f)
		{
			// Ray가 이 축에 대해 평행
			if (rayPos[i] < boxMin[i] || rayPos[i] > boxMax[i])
				return false;
		}
		else
		{
			float t1 = (boxMin[i] - rayPos[i]) / rayDir[i];
			float t2 = (boxMax[i] - rayPos[i]) / rayDir[i];

			if (t1 > t2) std::swap(t1, t2);

			tMin = max(tMin, t1);
			tMax = min(tMax, t2);

			if (tMin > tMax)
				return false;
		}
	}

	if (_pOutHitPos)
	{
		_pOutHitPos->x = m_vLocalRayPos.x + m_vLocalRayDir.x * tMin;
		_pOutHitPos->y = m_vLocalRayPos.y + m_vLocalRayDir.y * tMin;
		_pOutHitPos->z = m_vLocalRayPos.z + m_vLocalRayDir.z * tMin;
	}

	return true;
}

CPicking* CPicking::Create(HWND _hWnd, ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, _uint _iWinSizeX, _uint _iWinSizeY)
{
	CPicking* pInstance = new CPicking(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize(_hWnd, _iWinSizeX, _iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CPicking");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPicking::Free()
{
	__super::Free();

	Safe_Release(m_pDeviceContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
