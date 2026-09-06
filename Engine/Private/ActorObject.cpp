#include "ActorObject.h"
#include "GameInstance.h"
#include "Cell.h"

CActorObject::CActorObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CContainerObject { _pDevice, _pDeviceContext }
{
    memset(m_pStateArr, 0, sizeof(m_pStateArr));
}

CActorObject::CActorObject(const CActorObject& _Prototype)
    : CContainerObject (_Prototype)
{
    memset(m_pStateArr, 0, sizeof(m_pStateArr));
    m_pState = nullptr;
}

HRESULT CActorObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CActorObject::Initialize(void* _pArg)
{
    /* 1. CONTAINER DESC */
    CONTAINERR_INIT_DESC* pInitDesc = static_cast<CONTAINERR_INIT_DESC*>(_pArg);
    pInitDesc->tContainerDesc.iTotalPartsCnt = ENUM_TO_UINT(PART::END);
    if (FAILED(CContainerObject::Initialize(pInitDesc)))
        return E_FAIL;

    /* Connet */
    m_pActionController = CActionController::Create(this);
    if (!m_pActionController)
        return E_FAIL;

    return S_OK;
}

void CActorObject::Priority_Update(_float _fTimeDelta)
{
    /* 1. Action_Controller */
    if (m_pActionController)
        m_pActionController->Priority_Update(_fTimeDelta);

    CContainerObject::Priority_Update(_fTimeDelta);
}

void CActorObject::Update(_float _fTimeDelta)
{
    /* EFFECT */
    if (m_tKnockBack.bActive)
    {
        MoveWithKnockBack(_fTimeDelta);
    }
    else
    {
        if (m_pState)
            m_pState->OnStateStay(this, _fTimeDelta);
    }

    /* 1. 전달된 값을 FSM Context 에 기록 */
    if (m_pActionController)
        m_pActionController->Update(_fTimeDelta);

    /* 2. 현재 상태와 Context 를 기반으로 전이 규칙 평가 */
    Evaluate_Transitions();

    /* 3. FSM 이 선택한 상태 전이 적용 */
    Change_State();

    /* Part Object */
    CContainerObject::Update(_fTimeDelta);
}

void CActorObject::Late_Update(_float _fTimeDelta)
{
    /* 1. Action_Controller */
    if (m_pActionController)
        m_pActionController->Late_Update(_fTimeDelta);

    CContainerObject::Late_Update(_fTimeDelta);
}

HRESULT CActorObject::Render()
{
    return S_OK;
}

void CActorObject::Register_State(IState* _pState)
{
    if (!_pState)
        return;

    m_pStateArr[ENUM_TO_UINT(_pState->Get_StateID())] = _pState;
}

void CActorObject::Change_State()
{
    if (!m_bRequestStateChange)
        return;

    m_bRequestStateChange = false;
    m_iNextStatePriority = 0;

    if (ENUM_TO_UINT(m_NextState) >= ENUM_TO_UINT(IState::STATE_ID::END))
        return;

    /* 기존 코드 */
    IState* pNextState = m_pStateArr[ENUM_TO_UINT(m_NextState)];
    if (!pNextState)
        return;

    if (pNextState == m_pState)
        return;

    if (m_pState)
        m_pState->OnStateExit(this);

    m_pState = pNextState;
    m_pState->OnStateEnter(this);
}

_bool CActorObject::IsLocomotionEnable()
{
    /* ex. like Shield (move + shield) */
    return !m_pState->Block_Locomotion();
}

void CActorObject::MoveWithRotation(_vector _vMoveDir, _float _fTimeDelta)
{
    if (m_pTransformCom)
        m_pTransformCom->MoveWithRotation(_vMoveDir, _fTimeDelta);
}

void CActorObject::MovewithCollision(_vector _vMoveDir, _float _fTimeDelta)
{
    if (!m_pBodyCollider)
        return;
 
    /* 1. Move */
    _vector prevPos = m_pTransformCom->Get_State(STATE::POSITION);
    /* Excep. KnockBack */
    if (m_tKnockBack.bActive)
    {
        m_pTransformCom->Go_Straight(_fTimeDelta);
        m_pTransformCom->LookAt(m_pTransformCom->Get_State(STATE::POSITION) + m_vKnockBackDir);
        
        _vector NewKnockBack = m_pTransformCom->Get_State(STATE::POSITION);

        // 기존 방법
        if (m_pNavCom && !m_pNavCom->IsMove(NewKnockBack))
        {
            m_pTransformCom->Set_State(STATE::POSITION, prevPos);
            m_tKnockBack.bActive = false;
        }

        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

        return;
    }
    
    if (XMVector3Equal(_vMoveDir, XMVectorZero()))
        return;

    _vector vMoveDir = XMVectorSetY(_vMoveDir, 0.f);
    vMoveDir = XMVector3Normalize(vMoveDir);

    _vector targetPos =
        prevPos + vMoveDir * m_pTransformCom->Get_MoveSpeed() * _fTimeDelta;

    if (m_pNavCom && !m_pNavCom->IsMove(targetPos))
    {
        // refactor
        CCell* pCell = m_pNavCom->Get_Cell(m_pNavCom->Find_CurrentCell(prevPos));
        if (!pCell)
            return;

        _vector vMove = targetPos - prevPos;
        vMove = XMVectorSetY(vMove, 0.f);
        vMove = XMVector3Normalize(vMove);

        _int* pNeighbors = pCell->Get_NeighborIndices();

        _vector vBestNormal = XMVectorZero();
        _float fMaxDot = -FLT_MAX;

        for (_int i = 0; i < 3; ++i)
        {
            if (pNeighbors[i] != -1)
                continue;

            _vector A = pCell->Get_Point((CCell::POINT)i);
            _vector B = pCell->Get_Point((CCell::POINT)((i + 1) % 3));

            _vector vEdge = B - A;
            vEdge = XMVectorSetY(vEdge, 0.f);

            _vector vNormal = XMVector3Cross(vEdge, XMVectorSet(0.f, 1.f, 0.f, 0.f));
            vNormal = XMVectorSetY(vNormal, 0.f);
            vNormal = XMVector3Normalize(vNormal);

            _float fDot = XMVectorGetX(XMVector3Dot(vMove, vNormal));
            if (fDot > fMaxDot)
            {
                fMaxDot = fDot;
                vBestNormal = vNormal;
            }
        }

        if (fMaxDot > 0.f)
        {
            _vector vFullMove = targetPos - prevPos;
            vFullMove = XMVectorSetY(vFullMove, 0.f);

            _float fDot = XMVectorGetX(XMVector3Dot(vFullMove, vBestNormal));
            _vector vSlide = vFullMove - vBestNormal * fDot;

            if (XMVector3LengthSq(vSlide).m128_f32[0] > 0.000001f)
            {
                _vector vSlidePos = prevPos + vSlide;

                if (m_pNavCom->IsMove(vSlidePos))
                {
                    m_pTransformCom->Set_State(STATE::POSITION, vSlidePos);
                    m_pTransformCom->LookAt(vSlidePos - XMVector3Normalize(vSlide)); // 반대 방향..
                    m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
                }
            }
        }
        return;
    }

    m_pTransformCom->Set_State(STATE::POSITION, targetPos);
    m_pTransformCom->LookAt(targetPos - vMoveDir);
    m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* 2. Normal */
    _vector vNormal = XMVectorZero();
    _float fDepth = 0.f;
    if (!m_pGameInstance->Check_Block(m_pBodyCollider, &vNormal, &fDepth))
        return;

    /* 3. Move Return */
    m_pTransformCom->Set_State(STATE::POSITION, prevPos);
    m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* 4. Slide vector (v - dot(v, n) * n )*/
    _vector vMove = targetPos - prevPos;

    vMove = XMVectorSetY(vMove, 0.f);
    vNormal = XMVectorSetY(vNormal, 0.f);
    vNormal = XMVector3Normalize(vNormal);

    float fDot = XMVectorGetX(XMVector3Dot(vMove, vNormal));
    _vector vSlide = vMove - vNormal * fDot;

    if (XMVector3LengthSq(vSlide).m128_f32[0] < 0.000001f)
        return;

    _vector vSlidePos = prevPos + vSlide;

    if (m_pNavCom && !m_pNavCom->IsMove(vSlidePos))
        return;

    m_pTransformCom->Set_State(STATE::POSITION, vSlidePos);
    m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* 5. Stuck */
    if (fDepth > 0.0001f)
    {
        _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
        vPos += vNormal * (fDepth * 0.5f);
        m_pTransformCom->Set_State(STATE::POSITION, vPos);
        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    }
}

_vector CActorObject::Filter_KnockBackAxis(_vector _vDir)
{
    /* For. Side-View 넉백 시, Z 축 고정을 위한 함수 */
    return _vDir;
}

void CActorObject::Go_Up(_float _fTimeDelta)
{
    if (m_pTransformCom)
        m_pTransformCom->Go_Up(_fTimeDelta);
}

void CActorObject::Go_Down(_float _fTimeDelta)
{
    if (m_pTransformCom)
        m_pTransformCom->Go_Down(_fTimeDelta);
}

void CActorObject::Set_SpeedMulti(_float _fMulti)
{
    if (m_pTransformCom)
        m_pTransformCom->Set_SpeedMulti(_fMulti);
}

_bool CActorObject::Is_OnGround() const
{   
    if (!m_pTransformCom)
        return false;

    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vStart = vPos + XMVectorSet(0.f, 0.05f, 0.f, 0.f);
    
    const _float fRayLength = 0.35f;
    _vector vEnd = vStart + XMVectorSet(0.f, -1.f, 0.f, 0.f) * fRayLength;

    return m_pGameInstance->Raycast_GroundPlane(vStart, vEnd, 0.f);
}

_bool CActorObject::Is_ActionPressed(IState::TRANSITION_INPUT _eInput) const
{
    return m_pActionController && m_pActionController->Is_ActionPressed(_eInput);
}

_bool CActorObject::Is_MoveInputEnable()
{
    return m_pActionController->Is_MoveInputEnable();
}

void CActorObject::Start_KnockBack(_vector _vHitPos, _float _fPower, _float _fDuration)
{
    _vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vDir = vMyPos - _vHitPos;

    /* Filtering for Side-View */
    _vector vFilteredDir = Filter_KnockBackAxis(vDir);
    m_vKnockBackDir = XMVector3Normalize(vFilteredDir);

    m_fKnockBackSpeed = _fPower;
    m_tKnockBack.bActive = true;
    m_tKnockBack.fAccTime = 0.f;
    m_tKnockBack.fAccDurationTime = _fDuration;
}

void CActorObject::MoveWithKnockBack(_float _fTimeDelta)
{
    if (!m_tKnockBack.bActive)
        return;
    m_tKnockBack.fAccTime += _fTimeDelta;

    _float t = m_tKnockBack.fAccTime / m_tKnockBack.fAccDurationTime;
    _float fRatio = EaseOutCubic(t);

    _float endMulti = 0.2f;
    _float curMulti = m_fKnockBackSpeed * (1.f - fRatio) + endMulti * fRatio;
    m_pTransformCom->Set_SpeedMulti(curMulti);

    /* 툭 */
    if (m_tKnockBack.fAccTime < 0.06f)
        m_pTransformCom->Set_SpeedMulti(curMulti * 1.6f);

    MovewithCollision(m_vKnockBackDir, _fTimeDelta);

    if (t >= 1.f)
    {
        m_tKnockBack.bActive = false;
        m_pTransformCom->Set_SpeedMulti(1.f);
    }
}

void CActorObject::Free()
{
    __super::Free();

    for (_uint i = 0; i < ENUM_TO_UINT(IState::STATE_ID::END); ++i)
    {
        Safe_Delete(m_pStateArr[i]);
        m_pStateArr[i] = nullptr;
    }
    m_pState = nullptr;

    Safe_Release(m_pNavCom);
    Safe_Release(m_pBodyCollider);
    Safe_Release(m_pActionController);

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
