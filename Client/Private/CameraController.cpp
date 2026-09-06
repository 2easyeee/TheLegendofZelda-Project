#include "CameraController.h"
#include "GameInstance.h"
#include "Camera.h"
#include "GameObject/Player.h"

CCameraController::CCameraController()
    : CBase {}
    , m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CCameraController::Initialize()
{
    /* Offest */
    //m_vOffset = XMVectorSet(0.f, 6.f, -8.f, 0.f); // °¡±îÀÌ
    m_vOffset = XMVectorSet(0.f, 8.f, -10.f, 0.f);
    //m_vOffset = XMVectorSet(0.f, 6.f, -8.f, 0.f); // ´õ ³·°Ô
    m_fFollowSpeed = 10.f;
    m_fRoomMoveSpeed = 4.f;
    m_fEventSpeed = 3.f;

    return S_OK;
}

void CCameraController::Update(_float _fTimeDelta)
{
    _float fDelta = m_pGameInstance->Get_GlobalDelta(TEXT("Timer_Camera"));

    Reset_Camera();

    switch (m_eMode)
    {
    case Client::CCameraController::MODE::PLAYER: 
    {
        Update_Player(_fTimeDelta);
    }
        break;
    case Client::CCameraController::MODE::DUNGEON:
    {
        Update_Dungeon(_fTimeDelta);
    }
        break;
    case Client::CCameraController::MODE::EVENT:
    {
        Update_Event(_fTimeDelta);
    }
        break;
    case Client::CCameraController::MODE::STATIC:
    {
        Update_Static(_fTimeDelta);
    }
    break;
    case Client::CCameraController::MODE::ZOOM:
    {
        Update_Zoom(_fTimeDelta);
    }
    break;
    case Client::CCameraController::MODE::SHOULDERVIEW:
    {
        Update_ShoulderView(fDelta);
    }
    break;
    }
}

void CCameraController::Set_Camera(CCamera* _pCamera)
{
    m_pCamera = _pCamera;
    m_pCamTransform = static_cast<CTransform*>(_pCamera->Get_Component(TEXT("Com_Transform")));
}

void CCameraController::Set_Target(CGameObject* _pTarget)
{
    m_pTarget = _pTarget;

    /* Snap */
    if (!m_pTarget || !m_pCamTransform)
        return;
    CTransform* pTargetTransform =
        static_cast<CTransform*>(m_pTarget->Get_Component(TEXT("Com_Transform")));
    if (!pTargetTransform)
        return;

    _vector vTargetPos = pTargetTransform->Get_State(STATE::POSITION);
    _vector vSnapPos = vTargetPos + m_vOffset;

    m_pCamTransform->Set_State(STATE::POSITION, vSnapPos);
    m_pCamTransform->LookAt(vTargetPos);
}

void CCameraController::Set_Angle(_fvector _vAxis, _float _fAngle)
{
    m_pCamTransform->Rotation(_vAxis, _fAngle);
}

void CCameraController::Set_LockAxis(LOCK_AXIS _eType)
{
    m_eLockAxis = _eType;
}

void CCameraController::Set_Mode(MODE _eMode)
{
    if (m_ePreMode == MODE::END)
    {
        /* Init */
        m_ePreMode = _eMode;
    }

    m_eMode = _eMode;
}

void CCameraController::Set_RoomCenter(_vector _vCenter)
{
    m_vRoomCenter = _vCenter;
}

void CCameraController::Set_Event(_vector _vPos)
{
    m_vEventPos = _vPos;
}

void CCameraController::Set_Offset(_vector _vOffset)
{
    m_vOffset = _vOffset;
}

void CCameraController::Start_Shake(_float _fDuration, _float _fPower)
{
    m_tShakeTime.bActive = true;
    m_tShakeTime.fAccTime = 0.f;
    m_tShakeTime.fAccDurationTime = _fDuration;
    m_fShakePower = _fPower;
}

void CCameraController::Set_StaticPos(_vector _vCamPos, _vector _vCamLook, _float _fSpeed)
{
    m_vCamPos = _vCamPos;
    m_vCamLook = _vCamLook;
    m_fReturnSpeed = _fSpeed;
}

void CCameraController::Set_Zoom( _float _fDuration, _float _fPower, _float _fDelay)
{
    m_tZoomTime.bActive = true;
    m_tZoomTime.fAccTime = 0.f;
    m_tZoomTime.fAccDurationTime = _fDuration;

    m_tDelayTime.bActive = true;
    m_tDelayTime.fAccTime = 0.f;
    m_tDelayTime.fAccDurationTime = _fDelay;
    
    m_fShakePower = _fPower;

    m_vZoomStartPos = m_pCamTransform->Get_State(STATE::POSITION);
    m_pCamTransform->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), 45.f); // ´¯Çô¼­ º¸°í ´Ù½Ã º¹±Í
}

void CCameraController::Enter_ShoulderView()
{
    if (!m_pTarget) return;

    CTransform* pPlayerTransform =
        static_cast<CTransform*>(m_pTarget->Get_Component(TEXT("Com_Transform")));

    _vector vPlayerPos = pPlayerTransform->Get_State(STATE::POSITION);

    _matrix pParent = XMLoadFloat4x4(pPlayerTransform->Get_WorldMatrixPtr());

    _vector vLook = XMVector3Normalize(
        pPlayerTransform->Get_State(STATE::LOOK)
    );
    _vector vRight = XMVector3Normalize(
        XMVector3Cross(XMVectorSet(0, 1, 0, 0), vLook)
    );
    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    _vector vBase = vPlayerPos + XMVectorSet(0.f, 1.5f, 0.f, 0.f);

    _vector vOffset = XMVectorSet(-0.25f, 1.f, 3.9f, 0.f);

    _vector vTargetPos =
        vBase
        + vRight * XMVectorGetX(vOffset)
        + vUp * XMVectorGetY(vOffset)
        + vLook * XMVectorGetZ(vOffset);

    m_vFreezePos = vTargetPos;
    m_vFreezeLook = vLook;

    m_bShoulderFreeze = true;
}

void CCameraController::Exit_ShoulderView()
{
    m_bShoulderFreeze = false;
}

void CCameraController::Update_Player(_float _fTimeDelta)
{
    if (!m_pTarget)
        return;

    CTransform* pPlayerTransform = static_cast<CTransform*>(m_pTarget->Get_Component(TEXT("Com_Transform")));
    _vector vPlayerPos = pPlayerTransform->Get_State(STATE::POSITION);

    _vector vTargetPos = vPlayerPos + m_vOffset;

    /* Lock Axis */
    switch (m_eLockAxis)
    {
    case Client::CCameraController::LOCK_AXIS::HORIZONTAL:
    {
        vTargetPos = XMVectorSetY(vTargetPos, 0.f);
    }
        break;
    case Client::CCameraController::LOCK_AXIS::VERTICAL:
    {
        vTargetPos = XMVectorSetX(vTargetPos, 0.f);
    }
        break;
    }

    /* Camera */
    _vector vCamPos = m_pCamTransform->Get_State(STATE::POSITION);
    vCamPos = XMVectorLerp(vCamPos, vTargetPos, m_fFollowSpeed * _fTimeDelta);

    Apply_Shake(vCamPos, _fTimeDelta);

    m_pCamTransform->Set_State(STATE::POSITION, vCamPos);
}

void CCameraController::Update_Dungeon(_float _fTimeDelta)
{
    if (!m_pTarget)
        return;

    CTransform* pTargetTransform = static_cast<CTransform*>(m_pTarget->Get_Component(TEXT("Com_Transform")));
    _vector vPlayerPos = pTargetTransform->Get_State(STATE::POSITION);

    _int iRoomX = (_int)floor((XMVectorGetX(vPlayerPos) + m_iRoomSizeX * 0.5f) / m_iRoomSizeX);
    _int iRoomZ = (_int)floor((XMVectorGetZ(vPlayerPos) + m_iRoomSizeZ * 0.5f) / m_iRoomSizeZ);

    if (iRoomX != m_iCurrentRoomX || iRoomZ != m_iCurrentRoomZ)
    {
        m_iCurrentRoomX = iRoomX;
        m_iCurrentRoomZ = iRoomZ;

        _float fCenterX = (iRoomX * m_iRoomSizeX) + (m_iRoomSizeX * 0.f);
        _float fCenterZ = (iRoomZ * m_iRoomSizeZ) - (m_iRoomSizeZ * 0.38f);

        m_vRoomCenter = XMVectorSet(fCenterX, 0.f, fCenterZ, 1.f);
        m_bRoomMoving = true;
    }

    _vector vCamPos = m_vCurCamPos;    

    if (m_bRoomMoving)
    {
        _vector vTargetPos = m_vRoomCenter + m_vOffset;
        vCamPos = XMVectorLerp(vCamPos, vTargetPos, m_fRoomMoveSpeed * _fTimeDelta);

        if (XMVectorGetX(XMVector3Length(vTargetPos - vCamPos)) < 0.05f)
        {
            vCamPos = vTargetPos;
            m_bRoomMoving = false;
        }
    }

    m_vCurCamPos = vCamPos;
    _vector vFinalPos = vCamPos;
    Apply_Shake(vFinalPos, _fTimeDelta);

    m_pCamTransform->Set_State(STATE::POSITION, vFinalPos);
}

void CCameraController::Update_Event(_float _fTimeDelta)
{
    _vector vCamPos = m_pCamTransform->Get_State(STATE::POSITION);
    vCamPos = XMVectorLerp(vCamPos, m_vEventPos, m_fEventSpeed * _fTimeDelta);

    m_pCamTransform->Set_State(STATE::POSITION, vCamPos);
}

void CCameraController::Update_Static(_float _fTimeDelta)
{
    CTransform* pCamTr =
        static_cast<CTransform*>(m_pCamera->Get_Component(TEXT("Com_Transform")));

    if (!pCamTr)
        return;

    _vector vCurrentPos = pCamTr->Get_State(STATE::POSITION);

    _float time = 1.f - expf(-m_fReturnSpeed * _fTimeDelta);

    _vector vLerpPos = XMVectorLerp(vCurrentPos, m_vCamPos, time);

    Apply_Shake(vLerpPos, _fTimeDelta);

    pCamTr->Set_State(STATE::POSITION, vLerpPos);

    // µµÂø ÆÇÁ¤
    _float fDistance = XMVectorGetX(XMVector3Length(m_vCamPos - vLerpPos));

    if (fDistance < 0.01f)
    {
        _vector vFinal = m_vCamPos;
        Apply_Shake(vFinal, _fTimeDelta);

        pCamTr->Set_State(STATE::POSITION, m_vCamPos);
    }
}

void CCameraController::Update_Zoom(_float _fTimeDelta)
{
    if (!m_tZoomTime.bActive)
        return;

    m_tZoomTime.fAccTime += _fTimeDelta;
    m_tDelayTime.fAccTime += _fTimeDelta;

    float time = m_tZoomTime.fAccTime / m_tZoomTime.fAccDurationTime;
    time = min(time, 1.f);

    float eased = 1.f - powf(1.f - time, 3.f);

    _vector vDir = XMVector3Normalize(m_vEventPos - m_vZoomStartPos);

    float zoomDistance = 6.f;

    _vector vNewEye = m_vZoomStartPos + vDir * (zoomDistance * eased);
    _vector vOffset = XMVectorSet(0.f, -2.f, -3.f, 0.f);
    _vector vFinal = vNewEye + vOffset;

    /* EFFECT : Shake */
    if (m_tShakeTime.bActive)
    {
        m_tShakeTime.fAccTime += _fTimeDelta;
        if (m_tShakeTime.fAccTime >= m_tShakeTime.fAccDurationTime)
        {
            m_tShakeTime.bActive = false;
        }
        else
        {
            _float fPower = m_fShakePower;

            _float fRandX = ((rand() % 1000) / 1000.f - 0.5f) * 2.f * fPower;
            _float fRandY = ((rand() % 1000) / 1000.f - 0.5f) * 2.f * fPower;
            _float fRandZ = ((rand() % 1000) / 1000.f - 0.5f) * 2.f * fPower;

            _vector vShake = XMVectorSet(fRandX, 0.f, fRandZ, 0.f);
            vFinal += vShake;
        }
    }

    m_pCamTransform->Set_State(STATE::POSITION, vFinal);

    if (m_tZoomTime.fAccTime >= m_tZoomTime.fAccDurationTime)
    {
        if (m_tDelayTime.fAccTime >= m_tDelayTime.fAccDurationTime)
        {
            /* Static Delay */
            m_tZoomTime.bActive = false;
            Set_Mode(MODE::DUNGEON);
            m_pCamTransform->Set_State(STATE::POSITION, m_vZoomStartPos);
            this->Set_Offset(XMVectorSet(0.f, 14.f, 0.f, 0.f));
            this->Set_Angle(XMVectorSet(1.f, 0.f, 0.f, 0.f), 72.f);
        }
    }
}

void CCameraController::Update_ShoulderView(_float _fTimeDelta)
{
    // if (!m_pTarget)
    //     return;
    // 
    // if (m_bShoulderFreeze)
    // {
    //     m_pCamTransform->Set_State(STATE::POSITION, m_vFreezePos);
    // 
    //     _vector vLookTarget = m_vFreezePos + m_vFreezeLook * -1.f;
    //     m_pCamTransform->LookAt(vLookTarget);
    // 
    //     return;
    // }
    // 
    // auto pPlayer = dynamic_cast<CPlayer*>(m_pTarget);
    // if (!pPlayer)
    // {
    //     Set_Mode(MODE::PLAYER);
    //     return;
    // }
    // 
    // CTransform* pPlayerTransform = static_cast<CTransform*>(m_pTarget->Get_Component(TEXT("Com_Transform")));
    // _vector vPlayerPos = pPlayerTransform->Get_State(STATE::POSITION);
    // 
    // /* Socket */
    // const _float4x4* pParentMatrix = pPlayerTransform->Get_WorldMatrixPtr();
    // _matrix pParent = XMLoadFloat4x4(pParentMatrix);
    // const _float4x4* pSocket = pPlayer->Get_SocketMatrixPtr("spine_c");
    // _vector vSpine = XMVector3TransformCoord(XMVectorZero(), XMLoadFloat4x4(pSocket) * pParent);
    // 
    // _vector vSpineY = XMVectorSetY(XMVectorZero(), XMVectorGetY(vSpine));
    // _vector vBase = vPlayerPos + vSpineY;
    // 
    // _vector vRight = XMVector3Normalize(pParent.r[0]);
    // _vector vUp = XMVector3Normalize(pParent.r[1]);
    // _vector vLook = XMVector3Normalize(pParent.r[2]);
    // 
    // /* Shoulder */
    // _vector vOffset = XMVectorSet(-0.25f, 1.f, 3.9f, 0.f);
    // 
    // _vector vTargetPos =
    //     vBase
    //     + vRight * XMVectorGetX(vOffset)
    //     + vUp * XMVectorGetY(vOffset)
    //     + vLook * XMVectorGetZ(vOffset);
    // 
    // /* Camera */
    // _vector vCamPos = m_pCamTransform->Get_State(STATE::POSITION);
    // _float fAlpha = m_fFollowSpeed * _fTimeDelta;
    // fAlpha = min(fAlpha, 1.f);
    // vCamPos = XMVectorLerp(vCamPos, vTargetPos, fAlpha);
    // 
    // Apply_Shake(vCamPos, _fTimeDelta);
    // 
    // m_pCamTransform->Set_State(STATE::POSITION, vCamPos);
    // 
    // /* For. Look */
    // if (XMVector3Equal(m_vSmoothLook, XMVectorZero()))
    //     m_vSmoothLook = vLook;
    // _float fLerp = 1.f - expf(-8.f * _fTimeDelta);
    // m_vSmoothLook = XMVector3Normalize(XMVectorLerp(m_vSmoothLook, vLook, fLerp));
    // 
    // /* Look */
    // _vector vLookTarget = vCamPos + m_vSmoothLook * -1.f;
    // m_pCamTransform->LookAt(vLookTarget);

    if (!m_pTarget)
        return;

    auto pPlayer = static_cast<CPlayer*>(m_pTarget);
    if (!pPlayer)
    {
        Set_Mode(MODE::PLAYER);
        return;
    }

    CTransform* pPlayerTransform =
        static_cast<CTransform*>(m_pTarget->Get_Component(TEXT("Com_Transform")));

    _vector vPlayerPos = pPlayerTransform->Get_State(STATE::POSITION);

    _matrix pParent = XMLoadFloat4x4(pPlayerTransform->Get_WorldMatrixPtr());

    _vector vLook = XMVector3Normalize(pParent.r[2]);
    _vector vRight = XMVector3Normalize(pParent.r[0]);
    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    _vector vBase = vPlayerPos + XMVectorSet(0.f, 1.5f, 0.f, 0.f);

    _vector vOffset = XMVectorSet(-0.25f, 0.f, 3.9f, 0.f);

    _vector vCamPos =
        vBase
        + vRight * XMVectorGetX(vOffset)
        + vUp * XMVectorGetY(vOffset)
        + vLook * XMVectorGetZ(vOffset);

    m_pCamTransform->Set_State(STATE::POSITION, vCamPos);

    _vector vLookTarget = vCamPos + vLook * -1.f;
    m_pCamTransform->LookAt(vLookTarget);
}

void CCameraController::Reset_Camera()
{
    if (m_pGameInstance->Get_DIKeyUp(DIK_R))
    {
        m_eMode = m_ePreMode;
    }
}

void CCameraController::Apply_Shake(_vector& _vCamPos, _float _fTimeDelta)
{
    /* EFFECT : Shake */
    if (!m_tShakeTime.bActive)
        return;

    m_tShakeTime.fAccTime += _fTimeDelta;
    if (m_tShakeTime.fAccTime >= m_tShakeTime.fAccDurationTime)
    {
        m_tShakeTime.bActive = false;
    }
    else
    {
        _float fPower = m_fShakePower;

        _float fRandX = ((rand() % 1000) / 1000.f - 0.5f) * 2.f * fPower;
        _float fRandY = ((rand() % 1000) / 1000.f - 0.5f) * 2.f * fPower;
        _float fRandZ = ((rand() % 1000) / 1000.f - 0.5f) * 2.f * fPower;

        _vector vShake = XMVectorSet(fRandX, fRandY, fRandZ, 0.f);
        _vCamPos += vShake;
    }
}

CCameraController* CCameraController::Create()
{
    CCameraController* pInstance = new CCameraController();
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CCameraController");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCameraController::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
