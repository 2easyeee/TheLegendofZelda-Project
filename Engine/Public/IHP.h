#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class IHP
{
public:
	virtual ~IHP() = default;

public:
	virtual void Initialize_HP(_float _fHP) = 0;
	virtual void Damage_HP(_float _fHP) = 0;
	virtual void Add_HP(_float _fHP) = 0;
	virtual void Add_MAXHP(_float _fHP) = 0;

	virtual _float Get_CurrentHP() const = 0;
};
NS_END