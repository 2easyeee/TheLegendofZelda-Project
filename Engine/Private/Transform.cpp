#include "Transform.h"
#include "GameInstance.h"

CTransform::CTransform(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CComponent { _pDevice, _pDeviceContext }
{
}

CTransform::CTransform(const CTransform& _Prototype)
    : CComponent (_Prototype)
{
}

HRESULT CTransform::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTransform::Initialize(void* _pArg)
{
    XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

    if (_pArg)
        m_tTransformDesc = *static_cast<TRANSFORM_DESC*>(_pArg);

    return S_OK;
}

HRESULT CTransform::Bind_WorldMatrix(CShader* _pShader, const _char* _pConstantName)
{
    return _pShader->Bind_Matrix(_pConstantName, &m_WorldMatrix);
}

_float3 CTransform::Get_Scaled()
{
    return _float3(
        XMVectorGetX(XMVector3Length(Get_State(STATE::RIGHT))),
        XMVectorGetX(XMVector3Length(Get_State(STATE::UP))),
        XMVectorGetX(XMVector3Length(Get_State(STATE::LOOK)))
    );
}

_vector CTransform::Get_State(STATE _eState)
{
    return XMLoadFloat4(reinterpret_cast<_float4*>(&m_WorldMatrix.m[ENUM_TO_UINT(_eState)]));
}

void CTransform::Set_Scale(_float _fX, _float _fY, _float _fZ)
{
    /* New */
    Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * _fX);
    Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * _fY);
    Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * _fZ);
}

void CTransform::Set_State(STATE _eState, _fvector _vState)
{
    XMStoreFloat4(reinterpret_cast<_float4*>(&m_WorldMatrix.m[ENUM_TO_UINT(_eState)]), _vState);
}

void CTransform::Scaling(_float _fX, _float _fY, _float _fZ)
{
    /* 누적 */
    Set_State(STATE::RIGHT, Get_State(STATE::RIGHT) * _fX);
    Set_State(STATE::UP, Get_State(STATE::UP) * _fY);
    Set_State(STATE::LOOK, Get_State(STATE::LOOK) * _fZ);
}

void CTransform::Go_Straight(_float _fTimeDelta)
{
    _vector vPosition = Get_State(STATE::POSITION);
    _vector vLook = Get_State(STATE::LOOK);

    vPosition += XMVector3Normalize(vLook) * Get_MoveSpeed() * _fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Backward(_float _fTimeDelta)
{
    _vector vPosition = Get_State(STATE::POSITION);
    _vector vLook = Get_State(STATE::LOOK);

    vPosition -= XMVector3Normalize(vLook) * Get_MoveSpeed() * _fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Left(_float _fTimeDelta)
{
    _vector vPosition = Get_State(STATE::POSITION);
    _vector vRight = Get_State(STATE::RIGHT);

    vPosition -= XMVector3Normalize(vRight) * Get_MoveSpeed() * _fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Right(_float _fTimeDelta)
{
    _vector vPosition = Get_State(STATE::POSITION);
    _vector vRight = Get_State(STATE::RIGHT);

    vPosition += XMVector3Normalize(vRight) * Get_MoveSpeed() * _fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Up(_float _fTimeDelta)
{
    _vector vPosition = Get_State(STATE::POSITION);
    
    vPosition += XMVectorSet(0.f, 1.f, 0.f, 0.f) * Get_MoveSpeed() * _fTimeDelta;
    
    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Down(_float _fTimeDelta)
{
    _vector vPosition = Get_State(STATE::POSITION);

    vPosition += XMVectorSet(0.f, -1.f, 0.f, 0.f) * Get_MoveSpeed() * _fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Rotation(_fvector _vAxis, _float _fDegree)
{
    _float fRadian = XMConvertToRadians(_fDegree);

    _float3 vScaled = Get_Scaled();

    _vector vRight  = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScaled.x;
    _vector vUp     = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScaled.y;
    _vector vLook   = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScaled.z;

    _matrix RotationMatrix = XMMatrixRotationAxis(_vAxis, fRadian);

    Set_State(STATE::RIGHT, XMVector4Transform(vRight, RotationMatrix));
    Set_State(STATE::UP, XMVector4Transform(vUp, RotationMatrix));
    Set_State(STATE::LOOK, XMVector4Transform(vLook, RotationMatrix));
}

void CTransform::Turn(_fvector _vAxis, _float _fTimeDelta)
{
    _float fRadian = XMConvertToRadians(Get_RotationSpeed() * _fTimeDelta);
    
    _matrix RotationMatrix = XMMatrixRotationAxis(_vAxis, fRadian);

    Set_State(STATE::RIGHT, XMVector4Transform(Get_State(STATE::RIGHT), RotationMatrix));
    Set_State(STATE::UP, XMVector4Transform(Get_State(STATE::UP), RotationMatrix));
    Set_State(STATE::LOOK, XMVector4Transform(Get_State(STATE::LOOK), RotationMatrix));
}

void CTransform::LookAt(_fvector _vAt)
{
    _vector vPos = Get_State(STATE::POSITION);

    /* Look */
    _vector vLook = _vAt - vPos;
    if (XMVector3LengthSq(vLook).m128_f32[0] < 0.00001f)
        return;
    vLook = XMVector3Normalize(vLook);

    /* Up Axis */
    _vector vUpAxis = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    if (fabs(XMVectorGetY(vLook)) > 0.99f)
        vUpAxis = XMVectorSet(0.f, 0.f, 1.f, 0.f); // 거의 수직일 경우

    /* Right */
    _vector vRight = XMVector3Cross(vUpAxis, vLook);
    if (XMVector3LengthSq(vRight).m128_f32[0] < 0.00001f)
        return;
    vRight = XMVector3Normalize(vRight);

    /* Up */
    _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

    /* Scale */
    _float3 vScaled = Get_Scaled();

    Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScaled.x);
    Set_State(STATE::UP, XMVector3Normalize(vUp) * vScaled.y);
    Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScaled.z);
}

void CTransform::MoveTo(_fvector _vDestPos, _float _fTimeDelta, _float _fMinDistance)
{
    _vector vPosition = Get_State(STATE::POSITION);
    _vector vMoveDir = _vDestPos - vPosition;
    _float fDistance = XMVectorGetX(XMVector3Length(vMoveDir));

    vPosition += XMVector3Normalize(vMoveDir) * Get_MoveSpeed() * _fTimeDelta;

    if (fDistance > _fMinDistance)
        Set_State(STATE::POSITION, vPosition);
}

void CTransform::MoveWithRotation(_vector _vMoveDir, _float _fTimeDelta)
{
    if (XMVector3Equal(_vMoveDir, XMVectorZero()))
        return;

    /* Y 축 고정 */
    _vMoveDir = XMVectorSetY(_vMoveDir, 0.f);
    _vMoveDir = XMVector3Normalize(_vMoveDir);

    _float fX = XMVectorGetX(_vMoveDir);
    _float fZ = XMVectorGetZ(_vMoveDir);

    /* 1. 상하좌우 이동 */
    _vector vPos = Get_State(STATE::POSITION);
    vPos += _vMoveDir * Get_MoveSpeed() * _fTimeDelta;
    Set_State(STATE::POSITION, vPos);

    /* 2. 회전 */
    LookAt(vPos + (_vMoveDir * -1.f)); // 모델 반대방향
}

_float3 CTransform::GetEular(const _float4& q)
{
    float fqw = q.w * q.w;
    float fqx = q.x * q.x;
    float fqy = q.y * q.y;
    float fqz = q.z * q.z;

    float fYaw = atan2f(2.f * (q.x * q.z + q.w * q.y), (-fqx - fqy + fqz + fqw));
    float fPitch = asinf(2.f * (q.w * q.x - q.y * q.z));
    float fRoll = atan2f(2.f * (q.x * q.y + q.w * q.z), (-fqx + fqy - fqz + fqw));

    return { XMConvertToDegrees(fPitch), XMConvertToDegrees(fYaw), XMConvertToDegrees(fRoll) };
}

void CTransform::Update_ImGui_Inspector_Rotation(_float3 vEuler)
{
    _float3 vScale = Get_Scaled();

    _vector vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);

    // Yaw, Pitch, Roll (Y축, X축, Z축)
    _matrix matYaw = XMMatrixRotationAxis(vUp, XMConvertToRadians(vEuler.y));
    vRight = XMVector3TransformNormal(vRight, matYaw);
    vLook = XMVector3TransformNormal(vLook, matYaw);

    _matrix matPitch = XMMatrixRotationAxis(vRight, XMConvertToRadians(vEuler.x));
    vUp = XMVector3TransformNormal(vUp, matPitch);
    vLook = XMVector3TransformNormal(vLook, matPitch);

    _matrix matRoll = XMMatrixRotationAxis(vLook, XMConvertToRadians(vEuler.z));
    vRight = XMVector3TransformNormal(vRight, matRoll);
    vUp = XMVector3TransformNormal(vUp, matRoll);

    // 직교
    vLook = XMVector3Normalize(XMVector3Cross(vRight, vUp));
    vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));

    // 저장
    _float3 fRight, fUp, fLook;
    XMStoreFloat3(&fRight, vRight);
    XMStoreFloat3(&fUp, vUp);
    XMStoreFloat3(&fLook, vLook);

    Set_State(STATE::RIGHT, 
        { fRight.x * vScale.x, fRight.y * vScale.x, fRight.z * vScale.x });
    Set_State(STATE::UP, 
        { fUp.x * vScale.y, fUp.y * vScale.y, fUp.z * vScale.y });
    Set_State(STATE::LOOK, 
        { fLook.x * vScale.z, fLook.y * vScale.z, fLook.z * vScale.z });
}

CTransform* CTransform::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CTransform* pInstance = new CTransform(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CTransform");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CTransform::Clone(void* _pArg)
{
    CTransform* pInstance = new CTransform(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CTransform");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CTransform::Free()
{
    __super::Free();
}
