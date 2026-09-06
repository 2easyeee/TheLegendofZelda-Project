#pragma once
#include "Client_Defines.h"
#include "Base.h"
#include "ActionController.h"
#include "IState.h"

NS_BEGIN(Engine)
class CGameInstance;
class CActionController;
class IState;
NS_END

NS_BEGIN(Client)
class CPlayerInputController final : public CBase
{
public:
	enum class INPUT_MODE { TOP, SIDE, LADDER, END };

public:
	CPlayerInputController(class CActionController* _pActionController);
	virtual ~CPlayerInputController() = default;

public:
	void Update();

	void Set_InputMode(INPUT_MODE _eMode);
	void Set_InputLock(_bool _bActive);

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	class CActionController* m_pActionController = { nullptr };
	map<_int, IState::TRANSITION_INPUT> m_ActionKeys;

	INPUT_MODE m_eInputMode = { INPUT_MODE::TOP };

	_bool m_bInputLock = { false };

private:
	void Update_MoveInput();
	void Update_ActionInput();

	void Update_TopViewInput();
	void Update_SideViewInput();
	void Update_LadderViewInput();

public:
	static CPlayerInputController* Create(class CActionController* _pActionController);
	virtual void Free() override;
};
NS_END