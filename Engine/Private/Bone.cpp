#include "Bone.h"

CBone::CBone()
{
}

HRESULT CBone::Initialize(_string _name, _int _iParent, _float4x4 _transform)
{
	strcpy_s(m_szName, _name.c_str());
	m_TransformationMatrix = _transform;
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());
	m_iParentBoneIndex = _iParent;

	return S_OK;
}

void CBone::Update_CombinedTransformationMatrix(const vector<CBone*>& _Bones, _fmatrix _PreTransformMatrix)
{
	if (-1 == m_iParentBoneIndex)
		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			XMLoadFloat4x4(&m_TransformationMatrix) * _PreTransformMatrix);
	else
		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&_Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
}

CBone* CBone::Create(_string _name, _int _iParent, _float4x4 _transform)
{
	CBone* pInstance = new CBone();
	if (FAILED(pInstance->Initialize(_name, _iParent, _transform)))
	{
		MSG_BOX("FAILED TO CREATED : CBone");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CBone* CBone::Deep_Clone() const
{
	return new CBone(*this);
}

void CBone::Free()
{
	__super::Free();
}
