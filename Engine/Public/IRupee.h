#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class IRupee
{
public:
	virtual ~IRupee() = default;

public:
	virtual void Initialize_Rupee(_uint _iRupee) = 0;
	virtual void Add_Rupee(_uint _iRupee) = 0;
	virtual void Spend_Rupee(_uint _iRupee) = 0;

	virtual _uint Get_Rupee() const = 0;
};
NS_END