#include "PipeLine.h"
#include "GameInstance.h"

CPipeLine::CPipeLine()
{
}

HRESULT CPipeLine::Initialize()
{
	/* Init (Inverse)Matrix */
	for (size_t i = 0; i < ENUM_TO_UINT(D3DTS::END); i++)
	{
		XMStoreFloat4x4(&m_Transform[i], XMMatrixIdentity());
		XMStoreFloat4x4(&m_InvTransform[i], XMMatrixIdentity());
	}

	/* Init Camera */
	XMStoreFloat4(&m_vCameraPosition, XMVectorSet(0.f, 0.f, 0.f, 1.f));

	return S_OK;
}

void CPipeLine::Update()
{
	/* View / Proj 행렬의 역행렬을 매 프레임 계산 (Transform -> InvTransform) */
	for (size_t i = 0; i < ENUM_TO_UINT(D3DTS::END); i++)
	{
		XMStoreFloat4x4(&m_InvTransform[i], XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_Transform[i])));
	}

	/* View 의 역행렬로 카메라 월드 위치 추출 */
	memcpy(&m_vCameraPosition, &m_InvTransform[ENUM_TO_UINT(D3DTS::VIEW)].m[3], sizeof(_float4));
}

HRESULT CPipeLine::Bind_CameraPosition(CShader* _pShader, const _char* _pConstantName)
{
	return _pShader->Bind_RawValue(_pConstantName, &m_vCameraPosition, sizeof m_vCameraPosition);
}

HRESULT CPipeLine::Bind_TransformState(CShader* _pShader, const _char* _pConstantName, D3DTS _eTransformState)
{
	return _pShader->Bind_Matrix(_pConstantName, &m_Transform[ENUM_TO_UINT(_eTransformState)]);
}

HRESULT CPipeLine::Bind_InvTransformState(CShader* _pShader, const _char* _pConstantName, D3DTS _eTransformState)
{
	return _pShader->Bind_Matrix(_pConstantName, &m_InvTransform[ENUM_TO_UINT(_eTransformState)]);
}

void CPipeLine::Set_Transform(D3DTS _eTransformState, _fmatrix _TransformMatrix)
{
	XMStoreFloat4x4(&m_Transform[ENUM_TO_UINT(_eTransformState)], _TransformMatrix);
}

const _float4x4* CPipeLine::Get_Transform(D3DTS _eTransformState)
{
	return &m_Transform[ENUM_TO_UINT(_eTransformState)];
}

const _float4x4* CPipeLine::Get_InvTransform(D3DTS _eTransformState)
{
	return &m_InvTransform[ENUM_TO_UINT(_eTransformState)];
}

const _float4* CPipeLine::Get_CameraPosition()
{
	return &m_vCameraPosition;
}

CPipeLine* CPipeLine::Create()
{
	CPipeLine* pInstance = new CPipeLine();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("FAILED TO CREATED : CPipeLine");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPipeLine::Free()
{
	__super::Free();
}
