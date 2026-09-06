#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CDialogueManager final : public CBase
{
public:
	typedef struct tagDialogueChoice
	{
		_int iChoiceID;
		_int iNextNodeID;
		_wstring wstrText;
	}DIALOGUE_CHOICE;

	typedef struct tagDialogueNode
	{
		_int iNodeID;
		_int iNextNodeID;
		_wstring wstrActor;
		_wstring wstrText;
		vector<DIALOGUE_CHOICE> vecChoices;
	}DIALOGUE_NODE;

	typedef struct tagDialogueData
	{
		_wstring wstrDialogueID;
		map<_int, DIALOGUE_NODE> nodes;
	}DIALOGUE_DATA;

private:
	CDialogueManager();
	virtual ~CDialogueManager() = default;

public:
	HRESULT Load_XMLFile(const _wstring& _wstrFileName);

	HRESULT Start_Dialogue(const _wstring& _wstrDialogueID);
	void EndDialogue();

	void Update(_float _fTimeDelta);
	void Render();

	void NextNode(); // A 버튼 등으로 대사 넘기기
	void SelectChoice(_int _iIndex);

	_bool IsDialogueRunning() const { return m_bIsRunning; }
	_bool IsNodeFinished() const { return m_bIsNodeFinished; }
	const DIALOGUE_NODE* GetCurrentNode() const { return m_pCurrentDialogueNode; }

	/* SFX */
	void Play_ChatVoice(_float _fTimeDelta);

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	DIALOGUE_DATA m_tCurrentDialogueData = {};
	DIALOGUE_NODE* m_pCurrentDialogueNode = { nullptr };
	
	_bool m_bIsRunning = { false };

	_bool m_bIsNodeFinished = { false }; // 타이핑 효과 완료 여부
	_uint m_iCurrentNodeID = {};

	_wstring m_wstrDisplayText = {}; // UI 에게 넘기는 텍스트
	
	/* Typing */
	_float m_fTypingSpeed = 0.03f;
	_float m_fTypingAcc = 0.f;
	_uint m_iTypingIndex = 0;

	/* SFX */
	STATE_TIME m_tVoiceSFX;

private:
	void Convert_XML_NewLine(_wstring& _wstr);
	void Notify_ChoiceUI();
	void Notify_MessageBox_ON();
	void Notify_GetMessageBoxUI();
	void Notify_MessageBox_All_OFF();
	void Notify_NextIconUI();
	void Notify_CameraFocus_ON();
	void Notify_CameraFocus_OFF();

public:
	static CDialogueManager* Create();
	virtual void Free() override;
};
NS_END