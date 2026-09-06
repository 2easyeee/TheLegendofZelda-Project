#include "Channel.h"
#include "Model.h"
#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(CModel* _pModel, CHANNEL_DESC _Desc)
{
    m_iBoneIndex = _pModel->Get_BoneIndex(_Desc.boneName.c_str());
    if (-1 == m_iBoneIndex)
        return E_FAIL;
    
    /* KeyFrame 을 모아서 정리해주는 기능 */
    
    m_iNumKeyFrames = max(_Desc.iNumScalingKeys, _Desc.iNumRoationKeys);
    m_iNumKeyFrames = max(m_iNumKeyFrames, _Desc.iNumPositionKeys);

    _float3 vScale = {};
    _float4 vRotation = {};
    _float3 vTranslation = {};

    for (size_t i = 0; i < m_iNumKeyFrames; i++)
    {
        KEYFRAME KeyFrame = {};
        if (i < _Desc.iNumScalingKeys)
        {
            memcpy(&vScale, &_Desc.vecScalingKeys[i].vValue, sizeof(_float3));
            KeyFrame.fTrackPosition = _Desc.vecScalingKeys[i].fTime;
        }

        if (i < _Desc.iNumRoationKeys)
        {
            vRotation.x = _Desc.vecRotationKeys[i].vValue.x;
            vRotation.y = _Desc.vecRotationKeys[i].vValue.y;
            vRotation.z = _Desc.vecRotationKeys[i].vValue.z;
            vRotation.w = _Desc.vecRotationKeys[i].vValue.w;
            KeyFrame.fTrackPosition = _Desc.vecRotationKeys[i].fTime;
        }

        if (i < _Desc.iNumPositionKeys)
        {
            memcpy(&vTranslation, &_Desc.vecPositionKeys[i].vValue, sizeof(_float3));
            KeyFrame.fTrackPosition = _Desc.vecPositionKeys[i].fTime;
        }

        KeyFrame.vScale = vScale;
        KeyFrame.vRotation = vRotation;
        KeyFrame.vTranslation = vTranslation;

        m_vecKeyFrames.push_back(KeyFrame);
    }
    return S_OK;
}

void CChannel::Update_TransformationMatrices(_uint* _pCurrentKeyFrameIndex, _float _fCurrentTrackPosition, const vector<class CBone*>& _vecBones)
{
    if (0.f == _fCurrentTrackPosition)
        (*_pCurrentKeyFrameIndex) = 0;

    KEYFRAME tLastKeyFrame = m_vecKeyFrames.back();
    _vector vScale, vRotation, vTranslation;

    if (_fCurrentTrackPosition >= tLastKeyFrame.fTrackPosition)
    {
        /* 보간 필요없이 마지막 상태를 취한다. */
        vScale = XMLoadFloat3(&tLastKeyFrame.vScale);
        vRotation = XMLoadFloat4(&tLastKeyFrame.vRotation);
        vTranslation = XMVectorSetW(XMLoadFloat3(&tLastKeyFrame.vTranslation), 1.f);
    }
    else
    {
        /* 사이 보간이 필요하다. */
        while (_fCurrentTrackPosition >= m_vecKeyFrames[(*_pCurrentKeyFrameIndex) + 1].fTrackPosition)
            ++(*_pCurrentKeyFrameIndex);

        _float fRatio = {
            (_fCurrentTrackPosition - m_vecKeyFrames[(*_pCurrentKeyFrameIndex)].fTrackPosition) /
            (m_vecKeyFrames[(*_pCurrentKeyFrameIndex) + 1].fTrackPosition - m_vecKeyFrames[(*_pCurrentKeyFrameIndex)].fTrackPosition)
        };

        /* 선형 보간 */
        _vector vSourScale = XMLoadFloat3(&m_vecKeyFrames[(*_pCurrentKeyFrameIndex)].vScale);
        _vector vDestScale = XMLoadFloat3(&m_vecKeyFrames[(*_pCurrentKeyFrameIndex) + 1].vScale);
        vScale = XMVectorLerp(vSourScale, vDestScale, fRatio);

        _vector	vSourRotate = XMLoadFloat4(&m_vecKeyFrames[(*_pCurrentKeyFrameIndex)].vRotation);
        _vector	vDestRotate = XMLoadFloat4(&m_vecKeyFrames[(*_pCurrentKeyFrameIndex) + 1].vRotation);
        vRotation = XMQuaternionSlerp(vSourRotate, vDestRotate, fRatio);
        vRotation = XMQuaternionNormalize(vRotation);


        _vector vSourTranslation = XMVectorSetW(XMLoadFloat3(&m_vecKeyFrames[(*_pCurrentKeyFrameIndex)].vTranslation), 1.f);
        _vector vDestTranslation = XMVectorSetW(XMLoadFloat3(&m_vecKeyFrames[(*_pCurrentKeyFrameIndex) + 1].vTranslation), 1.f);
        vTranslation = XMVectorLerp(vSourTranslation, vDestTranslation, fRatio);
    }

    /* 이 Channel 에 해당하는 뼈만의 상태변환행렬을 구하고, 해당 행렬을 진짜 CBone 에게 전달해준다. */
    _matrix TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vTranslation);
    _vecBones[m_iBoneIndex]->Set_TransformationMatrix(TransformationMatrix);
}

CChannel* CChannel::Create(CModel* _pModel, CHANNEL_DESC _Desc)
{
    CChannel* pInstance = new CChannel();
    if (FAILED(pInstance->Initialize(_pModel, _Desc)))
    {
        MSG_BOX("FAILED TO CREATED : CChannel");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CChannel* CChannel::Deep_Clone()
{
    return new CChannel(*this);
}

void CChannel::Free()
{
    __super::Free();

    m_vecKeyFrames.clear();
}
