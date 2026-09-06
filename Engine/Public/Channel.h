#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CChannel final : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(class CModel* _pModel, CHANNEL_DESC _Desc);
	void Update_TransformationMatrices(_uint* _pCurrentKeyFrameIndex, _float _fCurrentTrackPosition, const vector<class CBone*>& _vecBones);

	KEYFRAME Get_KeyFrame(_uint _iIndex) const { return m_vecKeyFrames[_iIndex]; }
	_uint Get_NumKeyFrames() const { return m_iNumKeyFrames; }
	_int Get_BoneIndex() const { return m_iBoneIndex; }

private:
	_uint m_iNumKeyFrames = {};
	vector<KEYFRAME> m_vecKeyFrames;
	_int m_iBoneIndex = { -1 };

public:
	static CChannel* Create(class CModel* _pModel, CHANNEL_DESC _Desc);
	CChannel* Deep_Clone();
	virtual void Free() override;
};
NS_END