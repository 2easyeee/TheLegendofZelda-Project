#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CAnimation final : public CBase
{
private:
	CAnimation();
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize(class CModel* _pModel, ANIMATION_DESC _Desc);
	_bool Update_TransformationMatrices(_bool _isLoop, _float _fTimeDelta, const vector<class CBone*>& _vecBones);
	void Reset_TrackPosition(_bool _bSetKeyFrameIndexZero);

	KEYFRAME Get_CurrentKeyFrame(_uint _iChannelIndex) const;
	KEYFRAME Get_FirstKeyFrame(_uint _iChannelIndex) const;
	_uint Get_NumChannels() const { return m_iNumChannels; }
	class CChannel* Get_Channel(_int _iChannelIndex) const { return m_Channels[_iChannelIndex]; }
	void Set_AnimSpeedMulti(_float _fAnimSpeedMulti) { m_fAnimSpeedMulti = _fAnimSpeedMulti; }
	_bool IsAnimFinished() const { return m_bIsAnimFinished; }
	_float Get_PlayRatio() const;

private:
	_uint m_iNumChannels = {}; // 이 Anim 을 위해 컨트롤해야하는 뼈의 개수
	vector<class CChannel*> m_Channels; // 이 Anim 을 위해 컨트롤해야하는 뼈들
	vector< _uint> m_CurrentKeyFrameIndices;

	// 이 Anim 의 시작과 끝에 해당하는 전체 거리
	_float m_fDuration = {};
	_float m_fTickPerSecond = {};
	_float m_fCurrentTrackPosition = {};

	_float m_fAnimSpeedMulti = { 1.f };
	_bool m_bIsAnimFinished = { false };

public:
	static CAnimation* Create(class CModel* _pModel, ANIMATION_DESC _Desc);
	CAnimation* Deep_Clone();
	virtual void Free() override;
};
NS_END