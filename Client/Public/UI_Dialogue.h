#pragma once
#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)
class CUI_Dialogue final : public CBase
{
private:
	CUI_Dialogue();
	virtual ~CUI_Dialogue() = default;

public:
	void Initialize_Dialogue();
	void Update(_float _fDeltaTime);

public:
	void Hide_AllMessageBox();
	void Hide_ChoiceMessageBox();
	void PopUp_MessageBox();
	void PopUp_GetMessageBox();
	void PopUp_ChoiceMessageBox();
	void PopUp_MessageNext();

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	class CUI_Image* m_pMessageBox = { nullptr };
	class CUI_Image* m_pGetMessageBox = { nullptr };
	class CUI_Image* m_pChoiceMessageBox_A = { nullptr };
	class CUI_Image* m_pChoiceMessageBox_B = { nullptr };
	class CUI_Image* m_pMessageNext = { nullptr };

	/* Anim */
	float m_fNextAnimTime = { 0.f };

private:
	HRESULT Create_MessageBox_Image(_wstring _wstrTextureTag, _wstring _wstrObjectID, 
		_float2 _vPosition, _float _fScale,
		class CUI_Image*& _out,
		_float _fSizeX = 700.f, _float _fSizeY = 200.f);

public:
	static CUI_Dialogue* Create();
	virtual void Free();
};
NS_END