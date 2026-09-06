#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CSoundManager final : public CBase
{
private:
	CSoundManager();
	virtual ~CSoundManager() = default;

public:
	HRESULT Initialize();

	void Play_Sound(const TCHAR* _pSoundKey, SOUND _eID, float _fVolume = 1.f);
	void Play_BGM(const TCHAR* _pSoundKey, float _fVolume = 1.f, bool _bUseLoopPoint = false);
	void Play_Sound_Range(const TCHAR* _pSoundKey, float _fStartRatio, float _fEndRatio, float _fVolume = 1.f);
	void Stop_Sound(SOUND _eID);
	void Stop_All();
	void Set_ChannelVolume(SOUND _eID, float _fVolume);
	void Play_RandomSound(const TCHAR* _pBaseName, int _iCount, float _fVolume = 1.f);

private:
	map<TCHAR*, FMOD::Sound*> m_mapSound;
	
	FMOD::Channel* m_pChannelArr[ENUM_TO_UINT(SOUND::MAXCHANNEL)];

	FMOD::System* m_pSystem = { nullptr };

private:
	void Load_SoundFile();

public:
	static CSoundManager* Create();
	virtual void Free();
};
NS_END