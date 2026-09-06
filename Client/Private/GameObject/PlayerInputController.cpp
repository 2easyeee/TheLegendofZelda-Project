#include "GameObject/PlayerInputController.h"
#include "GameInstance.h"

CPlayerInputController::CPlayerInputController(CActionController* _pActionController)
    : m_pGameInstance{ CGameInstance::GetInstance() }
    , m_pActionController { _pActionController }
{
    Safe_AddRef(m_pGameInstance);

    /* KeyMapping */
    m_ActionKeys[DIK_SPACE]     = IState::TRANSITION_INPUT::JUMP_SIDEVIEW_PRESSED;
    m_ActionKeys[DIK_T]         = IState::TRANSITION_INPUT::JUMP_DODGE_PRESSED;

    m_ActionKeys[DIK_LSHIFT]    = IState::TRANSITION_INPUT::SHIELD_PRESSED;
    m_ActionKeys[DIK_G]         = IState::TRANSITION_INPUT::GET_ITEM_REQUESTED;

    m_ActionKeys[DIK_K]         = IState::TRANSITION_INPUT::ATTACK_PRESSED;
    m_ActionKeys[DIK_L]         = IState::TRANSITION_INPUT::BOOMERANG_PRESSED;

    /* ... */
}

void CPlayerInputController::Update()
{
    if (m_bInputLock)
        return;

    Update_MoveInput();
    Update_ActionInput();
}

void CPlayerInputController::Set_InputMode(INPUT_MODE _eMode)
{
    m_eInputMode = _eMode;
}

void CPlayerInputController::Set_InputLock(_bool _bActive)
{
    m_bInputLock = _bActive;

    if (_bActive && m_pActionController)
    {
        m_pActionController->Clear_MoveInput();
        m_pActionController->Force_StopAction();
        m_pActionController->Set_AimDirection(XMVectorZero());
    }
}

void CPlayerInputController::Update_MoveInput()
{
    if (!m_pActionController)
        return;

    switch (m_eInputMode)
    {
    case Client::CPlayerInputController::INPUT_MODE::TOP:
        Update_TopViewInput();
        break;
    case Client::CPlayerInputController::INPUT_MODE::SIDE:
        Update_SideViewInput();
        break;
    case Client::CPlayerInputController::INPUT_MODE::LADDER:
        Update_LadderViewInput();
        break;
    }
}

void CPlayerInputController::Update_ActionInput()
{
    if (!m_pActionController)
        return;

    for (auto& [KeyInput, Action] : m_ActionKeys)
    {
        if (m_pGameInstance->Get_DIKeyPressing(KeyInput))
            m_pActionController->Set_ActionPressed(Action, true);

        if (m_pGameInstance->Get_DIKeyDown(KeyInput))
        {
            m_pActionController->Push_Action(Action);
            m_pActionController->Set_ActionPressed(Action, true);
        }
        
        if (m_pGameInstance->Get_DIKeyUp(KeyInput))
            m_pActionController->Set_ActionPressed(Action, false);
    }
}

void CPlayerInputController::Update_TopViewInput()
{
    _vector vDiretion = XMVectorZero();
    _bool bMove = { false };

    /* MOVE */
    if (m_pGameInstance->Get_DIKeyPressing(DIK_D))
    {
        bMove = true;
        vDiretion += XMVectorSet(1.f, 0.f, 0.f, 0.f);
    }
    if (m_pGameInstance->Get_DIKeyPressing(DIK_A))
    {
        bMove = true;
        vDiretion += XMVectorSet(-1.f, 0.f, 0.f, 0.f);
    }
    if (m_pGameInstance->Get_DIKeyPressing(DIK_W))
    {
        bMove = true;
        vDiretion += XMVectorSet(0.f, 0.f, 1.f, 0.f);
    }
    if (m_pGameInstance->Get_DIKeyPressing(DIK_S))
    {
        bMove = true;
        vDiretion += XMVectorSet(0.f, 0.f, -1.f, 0.f);
    }

    if (bMove)
    {
        vDiretion = XMVector3Normalize(vDiretion);
        m_pActionController->Set_MoveInput(vDiretion);
    }
    else
    {
        m_pActionController->Clear_MoveInput();
    }
}

void CPlayerInputController::Update_SideViewInput()
{
    /* 누르면서 부메랑을 쏴야 가능함. */
    _vector vMoveDir = XMVectorZero();
    _vector vAimDir = XMVectorZero();
    _bool bMove = false;

    if (m_pGameInstance->Get_DIKeyPressing(DIK_D))
    {
        bMove = true;
        vMoveDir += XMVectorSet(1.f, 0.f, 0.f, 0.f);
        vAimDir += XMVectorSet(-1.f, 0.f, 0.f, 0.f);
    }

    if (m_pGameInstance->Get_DIKeyPressing(DIK_A))
    {
        bMove = true;
        vMoveDir += XMVectorSet(-1.f, 0.f, 0.f, 0.f);
        vAimDir += XMVectorSet(1.f, 0.f, 0.f, 0.f);
    }

    if (m_pGameInstance->Get_DIKeyPressing(DIK_W))
    {
        vAimDir += XMVectorSet(0.f, -1.f, 0.f, 0.f);
    }

    /* Move */
    if (bMove)
    {
        vMoveDir = XMVector3Normalize(vMoveDir);
        m_pActionController->Set_MoveInput(vMoveDir);
    }
    else
    {
        m_pActionController->Clear_MoveInput();
    }

    /* Aim */
    if (!XMVector3Equal(vAimDir, XMVectorZero()))
    {
        vAimDir = XMVector3Normalize(vAimDir);
        m_pActionController->Set_AimDirection(vAimDir);
    }
    else
    {
        m_pActionController->Set_AimDirection(XMVectorZero());
    }
}

void CPlayerInputController::Update_LadderViewInput()
{
    _vector vMoveDir = XMVectorZero();
    _vector vAimDir = XMVectorZero();
    _bool bMove = false;

    if (m_pGameInstance->Get_DIKeyPressing(DIK_W))
    {
        bMove = true;
        vMoveDir += XMVectorSet(0.f, -1.f, 0.f, 0.f);
        vAimDir += XMVectorSet(0.f, -1.f, 0.f, 0.f);
    }

    if (m_pGameInstance->Get_DIKeyPressing(DIK_S))
    {
        bMove = true;
        vMoveDir += XMVectorSet(0.f, 1.f, 0.f, 0.f);
        vAimDir += XMVectorSet(0.f, 1.f, 0.f, 0.f);
    }

    /* Move */
    if (bMove)
    {
        vMoveDir = XMVector3Normalize(vMoveDir);
        m_pActionController->Set_MoveInput(vMoveDir);
    }
    else
    {
        m_pActionController->Clear_MoveInput();
    }

    /* Aim */
    if (!XMVector3Equal(vAimDir, XMVectorZero()))
    {
        vAimDir = XMVector3Normalize(vAimDir);
        m_pActionController->Set_AimDirection(vAimDir);
    }
    else
    {
        m_pActionController->Set_AimDirection(XMVectorZero());
    }
}

CPlayerInputController* CPlayerInputController::Create(CActionController* _pActionController)
{
    return new CPlayerInputController(_pActionController);
}

void CPlayerInputController::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
