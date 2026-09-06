#include "DialogueManager.h"
#include "GameInstance.h"

CDialogueManager::CDialogueManager()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CDialogueManager::Load_XMLFile(const _wstring& _wstrFileName)
{
    _int size = WideCharToMultiByte(CP_UTF8, 0, _wstrFileName.c_str(), -1, nullptr, 0, nullptr, nullptr);
    _string strFileName(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, _wstrFileName.c_str(), -1, &strFileName[0], size, nullptr, nullptr);

    /* Load XML */
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(strFileName.c_str()) != tinyxml2::XML_SUCCESS)
        return E_FAIL;

    /* 1. ID */
    tinyxml2::XMLElement* dialogueElement = doc.FirstChildElement("dialogue");
    if (!dialogueElement)
        return E_FAIL;

    const char* id = dialogueElement->Attribute("id");
    if (id)
    {
        _int length = MultiByteToWideChar(CP_UTF8, 0, id, -1, nullptr, 0);
        m_tCurrentDialogueData.wstrDialogueID.resize(length - 1);
        MultiByteToWideChar(CP_UTF8, 0, id, -1, &m_tCurrentDialogueData.wstrDialogueID[0], length);
    }

    /* 2. node */
    for (tinyxml2::XMLElement* nodeElement = dialogueElement->FirstChildElement("node");
        nodeElement != nullptr;
        nodeElement = nodeElement->NextSiblingElement("node"))
    {
        DIALOGUE_NODE node = {};

        /* ID */
        nodeElement->QueryIntAttribute("id", &node.iNodeID);

        /* NEXT ID */
        _int nextNode = -1;
        if (nodeElement->QueryIntAttribute("next", &nextNode) == tinyxml2::XML_SUCCESS)
            node.iNextNodeID = nextNode;
        else
            node.iNextNodeID = -1;

        /* Actor */
        tinyxml2::XMLElement* actorElement = nodeElement->FirstChildElement("actor");
        if (actorElement && actorElement->GetText())
        {
            const char* actor = actorElement->GetText();
            _int length = MultiByteToWideChar(CP_UTF8, 0, actor, -1, nullptr, 0);
            node.wstrActor.resize(length - 1);
            MultiByteToWideChar(CP_UTF8, 0, actor, -1, &node.wstrActor[0], length);
        }

        /* Dialogue Text */
        tinyxml2::XMLElement* textElement = nodeElement->FirstChildElement("text");
        if (textElement && textElement->GetText())
        {
            const char* text = textElement->GetText();
            _int length = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
            node.wstrText.resize(length - 1);
            MultiByteToWideChar(CP_UTF8, 0, text, -1, &node.wstrText[0], length);

            /* EFFECT : NewLine */
            Convert_XML_NewLine(node.wstrText);
        }

        /* Choices */
        tinyxml2::XMLElement* choicesElement = nodeElement->FirstChildElement("choices");
        if (choicesElement)
        {
            node.iNextNodeID = -1; // 선택지가 있으면 자동 진행 없음 (의심)

            /* Choice */
            for (tinyxml2::XMLElement* choiceElement = choicesElement->FirstChildElement("choice");
                choiceElement != nullptr;
                choiceElement = choiceElement->NextSiblingElement("choice"))
            {
                DIALOGUE_CHOICE choice = {};

                /* Choice ID */
                choiceElement->QueryIntAttribute("id", &choice.iChoiceID);

                /* Next Node */
                _int nextChoiceNode = -1;
                choiceElement->QueryIntAttribute("next", &nextChoiceNode);
                choice.iNextNodeID = nextChoiceNode;

                /* Choice Text */
                const char* choiceText = choiceElement->GetText();
                if (choiceText)
                {
                    _int length = MultiByteToWideChar(CP_UTF8, 0, choiceText, -1, nullptr, 0);
                    choice.wstrText.resize(length - 1);
                    MultiByteToWideChar(CP_UTF8, 0, choiceText, -1, &choice.wstrText[0], length);
                }

                node.vecChoices.push_back(choice);
            }
        }
        m_tCurrentDialogueData.nodes[node.iNodeID] = node;
    }

    if (m_tCurrentDialogueData.nodes.empty())
        return E_FAIL;
    else
        return S_OK;
}

HRESULT CDialogueManager::Start_Dialogue(const _wstring& _wstrDialogueID)
{
    if (m_tCurrentDialogueData.nodes.empty())
        return E_FAIL;

    m_bIsRunning = true;
    m_iCurrentNodeID = 0; // Start First Node

    auto iter = m_tCurrentDialogueData.nodes.find(m_iCurrentNodeID);
    if (iter == m_tCurrentDialogueData.nodes.end())
        return E_FAIL;

    m_pCurrentDialogueNode = &iter->second;

    /* Event */
    CEventManager::CAMERA_EVENT tEvent = {};
    tEvent.eType = CEventManager::CAMERA_EVENT_TYPE::DIALOGUE;
    tEvent.wstrValue = m_pCurrentDialogueNode->wstrActor;
    m_pGameInstance->Push_CameraEVENT(tEvent);

    /* Reset Typing FX */
    m_wstrDisplayText.clear();
    m_fTypingAcc = 0.f;
    m_iTypingIndex = 0;
    m_bIsNodeFinished = false;
    Notify_MessageBox_ON();

    /* SFX */
    m_tVoiceSFX.Start(0.05);
    m_tVoiceSFX.fAccTime = m_tVoiceSFX.fAccDurationTime; // 바로 실행 보정

    return S_OK;
}

void CDialogueManager::EndDialogue()
{
    /* SFX */
    m_pGameInstance->Play_Sound(L"UI_Chat_End.wav", SOUND::EFFECT);

    /* All Clear */
    m_bIsRunning = false;
    m_pCurrentDialogueNode = nullptr;
    m_wstrDisplayText.clear();
    m_iCurrentNodeID = 0;
    m_fTypingAcc = 0.f;
    m_iTypingIndex = 0;
    m_bIsNodeFinished = false;
    Notify_MessageBox_All_OFF();
    Notify_CameraFocus_OFF();
}

void CDialogueManager::Update(_float _fTimeDelta)
{
    if (!m_bIsRunning || !m_pCurrentDialogueNode)
        return;

    /* Typing... */
    if (!m_bIsNodeFinished)
    {
        m_fTypingAcc += _fTimeDelta;

        if (m_fTypingAcc >= m_fTypingSpeed)
        {
            m_fTypingAcc = 0.f;

            if (m_iTypingIndex < m_pCurrentDialogueNode->wstrText.length())
            {
                m_wstrDisplayText += m_pCurrentDialogueNode->wstrText[m_iTypingIndex];
                Play_ChatVoice(_fTimeDelta);
                m_iTypingIndex++;
            }
            else
            {
                m_bIsNodeFinished = true;

                Notify_ChoiceUI();
            }
        }
    }
}

void CDialogueManager::Render()
{
    if (!m_bIsRunning || !m_pCurrentDialogueNode)
        return;

    // TODO : 하드코딩 위치 바꿔야함.
    /* actor */
    if (!m_pCurrentDialogueNode->wstrActor.empty())
    {
        m_pGameInstance->Add_Message(
            TEXT("Quicksand-Bold"),
            m_pCurrentDialogueNode->wstrActor.c_str(),
            XMVectorSet(1280.f * 0.25f, 720.f * 0.75f, 0.f, 1.f),
            1.f
        );
    }

    /* Dialogue Text */
    m_pGameInstance->Add_Message(
        TEXT("Quicksand-Bold"),
        m_wstrDisplayText.c_str(),
        XMVectorSet(1280.f * 0.5f, 720.f * 0.85f, 0.f, 1.3f),
        0.9f);

    /* Choice */
    // 선택지가 있고 타이핑이 끝났으면 선택지도 렌더링
    if (m_bIsNodeFinished && !m_pCurrentDialogueNode->vecChoices.empty())
    {
        for (size_t i = 0; i < m_pCurrentDialogueNode->vecChoices.size(); ++i)
        {
            _float fYOffset = 0.8f + (i * 0.1f);
            _wstring choiceText = m_pCurrentDialogueNode->vecChoices[i].wstrText;
            m_pGameInstance->Add_Message(
                TEXT("Quicksand-Medium"),
                choiceText.c_str(),
                XMVectorSet(1280.f * 0.9f, 720.f * fYOffset, 0.f, 1.f),
                0.9f);
        }
    }
    else if (m_bIsNodeFinished)
    {
        Notify_NextIconUI();
    }
}

void CDialogueManager::NextNode()
{
    if (!m_bIsRunning || !m_pCurrentDialogueNode)
        return;

    /* Typing Skip */
    if (!m_bIsNodeFinished)
    {
        m_wstrDisplayText = m_pCurrentDialogueNode->wstrText;
        m_iTypingIndex = m_pCurrentDialogueNode->wstrText.length();
        m_bIsNodeFinished = true;
        return;
    }

    /* if Exist Choice (Player must select) */
    if (!m_pCurrentDialogueNode->vecChoices.empty())
        return;

    /* Next Node */
    if (m_pCurrentDialogueNode->iNextNodeID == -1)
    {
        EndDialogue(); // next == -1
        return;
    }
    else
    {
        m_iCurrentNodeID = m_pCurrentDialogueNode->iNextNodeID;

        auto iter = m_tCurrentDialogueData.nodes.find(m_iCurrentNodeID);
        if (iter != m_tCurrentDialogueData.nodes.end())
        {
            m_pCurrentDialogueNode = &iter->second;

            /* SFX */
            m_pGameInstance->Play_Sound(L"UI_Chat_Next.wav", SOUND::EFFECT);
            m_tVoiceSFX.Start(0.05);
            m_tVoiceSFX.fAccTime = m_tVoiceSFX.fAccDurationTime;

            Notify_CameraFocus_ON();

            /* Typing Reset */
            m_wstrDisplayText.clear();
            m_fTypingAcc = 0.f;
            m_iTypingIndex = 0;
            m_bIsNodeFinished = false;
        }
        else
        {
            EndDialogue();
        }
    }
}

void CDialogueManager::SelectChoice(_int _iIndex)
{
    if (!m_bIsRunning || !m_pCurrentDialogueNode)
        return;

    if (!m_bIsNodeFinished)
        return;

    if (m_pCurrentDialogueNode->vecChoices.empty() ||
        _iIndex >= m_pCurrentDialogueNode->vecChoices.size())
        return;

    /* Choice Node */
    _int nextNodeID = m_pCurrentDialogueNode->vecChoices[_iIndex].iNextNodeID;
    
    auto iter = m_tCurrentDialogueData.nodes.find(nextNodeID);
    if (iter != m_tCurrentDialogueData.nodes.end())
    {
        m_iCurrentNodeID = nextNodeID;
        m_pCurrentDialogueNode = &iter->second;
        Notify_CameraFocus_ON();

        /* Typing Reset */
        m_wstrDisplayText.clear();
        m_fTypingAcc = 0.f;
        m_iTypingIndex = 0;
        m_bIsNodeFinished = false;

        Notify_ChoiceUI();
    }
    else
    {
        EndDialogue();
        return;
    }
}

void CDialogueManager::Play_ChatVoice(_float _fTimeDelta)
{
    if (!m_pCurrentDialogueNode)
        return;

    if (m_tVoiceSFX.Tick(_fTimeDelta))
    {
        const _wstring& actor = m_pCurrentDialogueNode->wstrActor;
        wchar_t ch = m_pCurrentDialogueNode->wstrText[m_iTypingIndex];
        if (ch == L' ' || ch == L'\n')
            return;

        /* Rand */
        //if (rand() % 2 == 0)
        //    return;

        if (actor == L"마린")
        {
            m_pGameInstance->Play_RandomSound(L"NPC_Marin_Talk", 3);
        }
        else if (actor == L"타린")
        {
            m_pGameInstance->Play_RandomSound(L"NPC_Tarin_Talk", 3);
        }
        else if (actor == L"울리라 할아버지")
        {
            if (rand() % 3 != 0)
                return;

            m_pGameInstance->Play_RandomSound(L"NPC_Ulrira", 11);
        }
    }
}

void CDialogueManager::Convert_XML_NewLine(_wstring& _wstr)
{
    size_t iPosition = 0;
    while ((iPosition = _wstr.find(L"\\n", iPosition)) != _wstring::npos)
    {
        _wstr.replace(iPosition, 2, L"\n");
        iPosition += 1;
    }
}

void CDialogueManager::Notify_ChoiceUI()
{
    if (!m_pCurrentDialogueNode)
        return;

    CEventManager::UI_EVENT tUIEvent = {};
    if (m_pCurrentDialogueNode->vecChoices.empty())
        tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::DIALOGUE_CHOICE_OFF;
    else
        tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::DIALOGUE_CHOICE_ON;

    m_pGameInstance->Push_UIEVENT(tUIEvent);
}

void CDialogueManager::Notify_MessageBox_ON()
{
    CEventManager::UI_EVENT tUIEvent = {};
    tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::DIALOGUE_MESSAGEBOX_ON;
    m_pGameInstance->Push_UIEVENT(tUIEvent);
}

void CDialogueManager::Notify_GetMessageBoxUI()
{
    if (!m_pCurrentDialogueNode)
        return;

    CEventManager::UI_EVENT tUIEvent = {};
    tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::DIALOGUE_GETMESSAGEBOX_ON;

    m_pGameInstance->Push_UIEVENT(tUIEvent);
}

void CDialogueManager::Notify_MessageBox_All_OFF()
{
    /* Event (UI) */
    CEventManager::UI_EVENT tUIEvent = {};
    tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::DIALOGUE_ALL_OFF;
    m_pGameInstance->Push_UIEVENT(tUIEvent);
}

void CDialogueManager::Notify_NextIconUI()
{
    /* Event (UI) */
    CEventManager::UI_EVENT tUIEvent = {};
    tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::DIALOGUE_NEXT;
    m_pGameInstance->Push_UIEVENT(tUIEvent);
}

void CDialogueManager::Notify_CameraFocus_ON()
{
    /* Event */
    CEventManager::CAMERA_EVENT tEvent = {};
    tEvent.eType = CEventManager::CAMERA_EVENT_TYPE::DIALOGUE;
    tEvent.wstrValue = m_pCurrentDialogueNode->wstrActor;
    m_pGameInstance->Push_CameraEVENT(tEvent);
}

void CDialogueManager::Notify_CameraFocus_OFF()
{
    /* Event (Camera) */
    CEventManager::CAMERA_EVENT tCameraEvent = {};
    tCameraEvent.eType = CEventManager::CAMERA_EVENT_TYPE::DIALOGUE_END;
    m_pGameInstance->Push_CameraEVENT(tCameraEvent);
}

CDialogueManager* CDialogueManager::Create()
{
    return new CDialogueManager();
}

void CDialogueManager::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
