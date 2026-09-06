#include "SoundManager.h"
#include <io.h>

CSoundManager::CSoundManager()
{
}

HRESULT CSoundManager::Initialize()
{
    FMOD::System_Create(&m_pSystem);

    m_pSystem->init(32, FMOD_INIT_NORMAL, nullptr);

    Load_SoundFile();

    return S_OK;
}

void CSoundManager::Play_Sound(const TCHAR* _pSoundKey, SOUND _eID, float _fVolume)
{
    auto iter = find_if(m_mapSound.begin(), m_mapSound.end(),
        [&](auto& iter)->bool {
            return !lstrcmp(_pSoundKey, iter.first);
        });

    if (iter == m_mapSound.end())
        return;

    _uint ID = ENUM_TO_UINT(_eID);

    //m_pSystem->playSound(iter->second, nullptr, false, &m_pChannelArr[ID]);

    //if (m_pChannelArr[ID])
    //    m_pChannelArr[ID]->setVolume(_fVolume);

    FMOD::Channel* pChannel = { nullptr };
    m_pSystem->playSound(iter->second, nullptr, false, &pChannel);

    if (pChannel)
        pChannel->setVolume(_fVolume);

    m_pSystem->update();
}

void CSoundManager::Play_Sound_Range(const TCHAR* _pSoundKey, float _fStartRatio, float _fEndRatio, float _fVolume)
{
    auto iter = find_if(m_mapSound.begin(), m_mapSound.end(),
        [&](auto& iter)->bool {
            return !lstrcmp(_pSoundKey, iter.first);
        });

    if (iter == m_mapSound.end())
        return;

    _uint ID = ENUM_TO_UINT(SOUND::EFFECT);

    m_pSystem->playSound(iter->second, nullptr, true, &m_pChannelArr[ID]);

    FMOD::Channel* pChannel = m_pChannelArr[ID];
    if (!pChannel)
        return;

    pChannel->setVolume(_fVolume);

    unsigned int length = 0;
    iter->second->getLength(&length, FMOD_TIMEUNIT_MS);

    unsigned int start = (unsigned int)(length * _fStartRatio);
    unsigned int end = (unsigned int)(length * _fEndRatio);

    pChannel->setPosition(start, FMOD_TIMEUNIT_MS);

    unsigned long long dspClock = 0;
    unsigned long long parentClock = 0;
    pChannel->getDSPClock(&dspClock, &parentClock);

    int sampleRate = 48000;
    m_pSystem->getSoftwareFormat(&sampleRate, 0, 0);

    unsigned long long delayEnd =
        dspClock + (unsigned long long)((end - start) * sampleRate / 1000);

    pChannel->setDelay(0, delayEnd, false);
    pChannel->setPaused(false);
}

void CSoundManager::Play_BGM(const TCHAR* _pSoundKey, float _fVolume, bool _bUseLoopPoint)
{
    auto iter = find_if(m_mapSound.begin(), m_mapSound.end(),
        [&](auto& iter)->bool {
            return !lstrcmp(_pSoundKey, iter.first);
        });

    if (iter == m_mapSound.end())
        return;

    _uint BgmChannel = ENUM_TO_UINT(SOUND::BGM);
    m_pSystem->playSound(iter->second, nullptr, false, &m_pChannelArr[BgmChannel]);

    if (m_pChannelArr[BgmChannel])
    {
        m_pChannelArr[BgmChannel]->setVolume(_fVolume);

        if (_bUseLoopPoint)
        {
            unsigned int length = 0;
            iter->second->getLength(&length, FMOD_TIMEUNIT_MS);

            unsigned int loopStart = (unsigned int)(length * 0.1f);
            unsigned int loopEnd = (unsigned int)(length * 0.85f);

            m_pChannelArr[BgmChannel]->setMode(FMOD_LOOP_NORMAL);
            m_pChannelArr[BgmChannel]->setLoopPoints(
                loopStart, FMOD_TIMEUNIT_MS,
                loopEnd, FMOD_TIMEUNIT_MS
            );
        }
        else
        {
            m_pChannelArr[BgmChannel]->setMode(FMOD_LOOP_NORMAL);
        }
    }

    m_pSystem->update();
}

void CSoundManager::Stop_Sound(SOUND _eID)
{
    if (m_pChannelArr[ENUM_TO_UINT(_eID)])
        m_pChannelArr[ENUM_TO_UINT(_eID)]->stop();
}

void CSoundManager::Stop_All()
{
    for (int i = 0; i < ENUM_TO_UINT(SOUND::MAXCHANNEL); ++i)
    {
        if (m_pChannelArr[i])
            m_pChannelArr[i]->stop();
    }
}

void CSoundManager::Set_ChannelVolume(SOUND _eID, float _fVolume)
{
    if (m_pChannelArr[ENUM_TO_UINT(_eID)])
        m_pChannelArr[ENUM_TO_UINT(_eID)]->setVolume(_fVolume);

    m_pSystem->update();
}

void CSoundManager::Play_RandomSound(const TCHAR* _pBaseName, int _iCount, float _fVolume)
{
    /* Only. wav */
    if (_iCount <= 0)
        return;

    int iRand = rand() % _iCount;

    wstring fileName = wstring(_pBaseName) + L"_" + std::to_wstring(iRand) + L".wav";

    Play_Sound(fileName.c_str(), SOUND::EFFECT, _fVolume);
}

void CSoundManager::Load_SoundFile()
{
    _finddata_t fd;

    intptr_t handle = _findfirst("../../Resources/Sounds/*.*", &fd);
    if (handle == -1)
        return;

    int iResult = 0;
    char szCurPath[128] = "../../Resources/Sounds/";
    char szFullPath[128] = "";

    while (iResult != -1)
    {
        strcpy_s(szFullPath, szCurPath);
        strcat_s(szFullPath, fd.name);

        FMOD::Sound* pSound = nullptr;

        FMOD_RESULT eRes = m_pSystem->createSound(szFullPath, FMOD_DEFAULT, 0, &pSound);

        if (eRes == FMOD_OK)
        {
            int iLength = (int)strlen(fd.name) + 1;

            TCHAR* pSoundKey = new TCHAR[iLength];
            ZeroMemory(pSoundKey, sizeof(TCHAR) * iLength);

            MultiByteToWideChar(CP_ACP, 0, fd.name, iLength, pSoundKey, iLength);

            m_mapSound.emplace(pSoundKey, pSound);
        }

        iResult = _findnext(handle, &fd);
    }

    m_pSystem->update();

    _findclose(handle);
}

CSoundManager* CSoundManager::Create()
{
    CSoundManager* pInstance = new CSoundManager();
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CSoundManager");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CSoundManager::Free()
{
    __super::Free();

    for (auto& Mypair : m_mapSound)
    {
        delete[] Mypair.first;
        Mypair.second->release();
    }
    m_mapSound.clear();

    if (m_pSystem)
    {
        m_pSystem->close();
        m_pSystem->release();
        m_pSystem = nullptr;
    }
}
