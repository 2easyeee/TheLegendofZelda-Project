#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class IPawnState
{
public:
	virtual ~IPawnState() = default;

public:
	virtual _uint Get_StateID() const = 0;
	virtual void OnStateEnter(class CPawnObject* _pActor) = 0;
	virtual void OnStateStay(class CPawnObject* _pActor, _float _fTimeDelta) = 0;
	virtual void OnStateExit(class CPawnObject* _pActor) = 0;
};
NS_END