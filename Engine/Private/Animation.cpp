#include "Animation.h"
#include "Channel.h"

CAnimation::CAnimation()
{
}

HRESULT CAnimation::Initialize(CModel* _pModel, ANIMATION_DESC _Desc)
{
    m_fDuration = _Desc.fDuration;
    m_fTickPerSecond = _Desc.fTicksPerSecond;
    m_iNumChannels = _Desc.iNumChannels;
    m_CurrentKeyFrameIndices.resize(m_iNumChannels);

    for (size_t i = 0; i < m_iNumChannels; i++)
    {
        CChannel* pChannel = CChannel::Create(_pModel, _Desc.vecChannelDesc[i]);
        if (nullptr == pChannel)
            return E_FAIL;

        m_Channels.push_back(pChannel);
    }
    return S_OK;
}

_bool CAnimation::Update_TransformationMatrices(_bool _isLoop, _float _fTimeDelta, const vector<class CBone*>& _vecBones)
{
    m_fCurrentTrackPosition += m_fTickPerSecond * _fTimeDelta * m_fAnimSpeedMulti;

    if (m_fCurrentTrackPosition >= m_fDuration)
    {
        if (false == _isLoop)
        {
            m_bIsAnimFinished = true;
            return true;
        }
        m_fCurrentTrackPosition = 0.f;
    }

    _uint iNumChannels = {};
    for (auto& pChannel : m_Channels)
        pChannel->Update_TransformationMatrices(&m_CurrentKeyFrameIndices[iNumChannels++], m_fCurrentTrackPosition, _vecBones);

    return false;
}

void CAnimation::Reset_TrackPosition(_bool _bSetKeyFrameIndexZero)
{
    /* (기본) 재생 시간 0 */
    m_fCurrentTrackPosition = 0.f;
    m_bIsAnimFinished = false;

    if (_bSetKeyFrameIndexZero)
    {
        if (m_CurrentKeyFrameIndices.size() != m_iNumChannels)
            m_CurrentKeyFrameIndices.resize(m_iNumChannels);

        for (auto& Index : m_CurrentKeyFrameIndices)
            Index = 0;
    }
}

KEYFRAME CAnimation::Get_CurrentKeyFrame(_uint _iChannelIndex) const
{
    return m_Channels[_iChannelIndex]->Get_KeyFrame(m_CurrentKeyFrameIndices[_iChannelIndex]);
}

KEYFRAME CAnimation::Get_FirstKeyFrame(_uint _iChannelIndex) const
{
    return m_Channels[_iChannelIndex]->Get_KeyFrame(0);
}

_float CAnimation::Get_PlayRatio() const
{
    if (m_fDuration <= 0.f)
        return 0.f;

    _float fPlayRatio = m_fCurrentTrackPosition / m_fDuration;

    if (fPlayRatio > 1.f)
        fPlayRatio = 1.f;
    if (fPlayRatio < 0.f)
        fPlayRatio = 0.f;

    return fPlayRatio;
}

CAnimation* CAnimation::Create(CModel* _pModel, ANIMATION_DESC _Desc)
{
    CAnimation* pInstance = new CAnimation();
    if (FAILED(pInstance->Initialize(_pModel, _Desc)))
    {
        MSG_BOX("FAILED TO CREATED : CAnimation");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CAnimation* CAnimation::Deep_Clone()
{
    CAnimation* pInstance = new CAnimation();
    pInstance->m_fDuration = m_fDuration;
    pInstance->m_fTickPerSecond = m_fTickPerSecond;
    pInstance->m_iNumChannels = m_iNumChannels;

    for (auto& pChannel : m_Channels)
        pInstance->m_Channels.push_back(pChannel->Deep_Clone());

    pInstance->m_fCurrentTrackPosition = 0.f;
    pInstance->m_CurrentKeyFrameIndices.clear();
    pInstance->m_CurrentKeyFrameIndices.resize(m_iNumChannels, 0);
       
    return pInstance;
}

void CAnimation::Free()
{
    __super::Free();

    for (auto& pChannel : m_Channels)
        Safe_Release(pChannel);
    m_Channels.clear();

    m_CurrentKeyFrameIndices.clear();
}
