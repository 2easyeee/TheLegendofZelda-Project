#pragma once
#include "Client_Defines.h"
#include "Base.h"
#include "IRupee.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)
class CUI_Rupee final : public CBase, public IRupee
{
public:
	typedef struct tagRupeeLayout
	{
		_float2 vStartPosition;
		_float2 vEndPosition;
		_float2 vSize;
		_float fNumberGap;
		_float fIconGap;
	}Rupee_Layout;

private:
	CUI_Rupee();
	virtual ~CUI_Rupee() = default;

public:
	virtual void Initialize_Rupee(_uint _iRupee) override;
	virtual void Add_Rupee(_uint _iRupee) override;
	virtual void Spend_Rupee(_uint _iRupee) override;

	virtual _uint Get_Rupee() const override;

public:
	void Hide_Rupee();
	void PopUp_Rupee();
	void AlwaysOn_Rupee();

	void Update(_float _fDeltaTime);

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	class CUI_Image* m_pRupeeImage = { nullptr };
	vector<class CUI_Number*> m_vecNumber;

	_uint m_iMaxRupee = { 9999 };
	_uint m_iRupee = {};

	Rupee_Layout m_tLayout = {};
	_bool m_bUpdatalbe = { false };

	/* Visible */
	STATE_TIME m_tVisibleTime = { false, 0.f, 2.f };

private:
	HRESULT Create_Rupee_Image(_float2 _fPosition, _float _fScale);
	HRESULT Create_Rupee_Number(_float2 _fPosition, _float _fScale);
	
	void Update_Layout(const _int* _iNumbers);

public:
	static CUI_Rupee* Create();
	virtual void Free() override;
};
NS_END