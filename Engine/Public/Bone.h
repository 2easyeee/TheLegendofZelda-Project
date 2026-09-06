#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CBone final : public CBase
{
private:
	CBone();
	virtual ~CBone() = default;

public:
	HRESULT Initialize(_string _name, _int _iParent, _float4x4 _transform);
	void	Update_CombinedTransformationMatrix(const vector<CBone*>& _Bones, _fmatrix _PreTransformMatrix);

	/* Matrix */
	_matrix				Get_CombinedTransformationMatrix() { return XMLoadFloat4x4(&m_CombinedTransformationMatrix); }
	const _float4x4*	Get_CombinedTransformMatrixPrt() { return &m_CombinedTransformationMatrix;  }
	void				Set_TransformationMatrix(_fmatrix _TransformationMarix) { XMStoreFloat4x4(&m_TransformationMatrix, _TransformationMarix); }
	_fmatrix			Get_TransformationMatrix() const { return XMLoadFloat4x4(&m_TransformationMatrix); }

	/* Index */
	_int				Get_ParentIndex() { return m_iParentBoneIndex; }

	/* Name */
	_bool				Compare_Name(const _char* _pName) { return !strcmp(_pName, m_szName); }
	const _char*		Get_Name() const { return m_szName; }

private:
	_char		m_szName[MAX_PATH] = {};
	_float4x4	m_TransformationMatrix = {};
	_float4x4	m_CombinedTransformationMatrix = {};
	_int		m_iParentBoneIndex = { -1 };

public:
	static CBone* Create(_string _name, _int _iParent, _float4x4 _transform);
	CBone* Deep_Clone() const;
	virtual void Free() override;



};
NS_END