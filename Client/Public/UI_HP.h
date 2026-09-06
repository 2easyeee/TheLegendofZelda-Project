#pragma once
#include "Client_Defines.h"
#include "Base.h"
#include "IHP.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)
class CUI_HP final : public CBase, public IHP
{
public:
	typedef struct tagHPLayout
	{
		_float2 vStartPosition;
		_float2 vSize;
		_float2 vGap;
		_uint iMaxRow;
	}HP_Layout;

private:
	CUI_HP();
	virtual ~CUI_HP() = default;

public:
	virtual void Initialize_HP(_float _fHP) override;
	virtual void Damage_HP(_float _fHP) override;
	virtual void Add_HP(_float _fHP) override;
	virtual void Add_MAXHP(_float _fHP) override;
	
	virtual _float Get_CurrentHP() const override;

public:
	void Hide_HP();
	void PopUp_HP();
	void AlwaysOn_HP();

	void Update(_float _fDeltaTime);
	_bool IsUpdatable() const;

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	vector<class CUI_Heart*> m_vecHP;
	_uint m_iMaxHP = { 20 };
	_uint m_iOwnedHP = { 3 };
	_float m_fCurrentHP = { 0.f };

	HP_Layout m_tHPLayout = {};

private:
	HRESULT Create_HP(_float2 _fPosition, _uint _iIndex);
	_float2 Compute_Position(_uint _iIndex);

public:
	static CUI_HP* Create();
	virtual void Free() override;
};
NS_END