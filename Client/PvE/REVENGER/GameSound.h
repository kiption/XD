#pragma once
#include "inc/inc/fmod.hpp"
#include "inc/inc/fmod_errors.h"
// fmod dll
#pragma comment(lib, "lib/lib/64/fmod_vc.lib")
#pragma comment(lib, "lib/lib/64/fmodL_vc.lib")


class GameSound
{
public:	
	FMOD::System* soundSystem;

	FMOD::Sound* shotSound;
	FMOD::Channel* shotChannel;

	FMOD::Sound* HelishotSound;
	FMOD::Channel* HelishotChannel;

	FMOD::Sound* NpcshotSound;
	FMOD::Channel* NpcshootChannel;

	FMOD::Sound* HumanColliSound;
	FMOD::Channel* HumanColliChannel;

	FMOD::Sound* HeliColliSound;
	FMOD::Channel* HeliColliChannel;

	FMOD::Sound* speakSound;
	FMOD::Channel* speakChannel;

	FMOD::Sound* bgmSound;
	FMOD::Channel* bgmChannel;

	FMOD::Sound* HartbeatSound;
	FMOD::Channel* HartbeatChannel;

	FMOD::Sound* WarnningSound;
	FMOD::Channel* WarnningChannel;

	FMOD::Sound* RotorSound;
	FMOD::Channel* RotorChannel;

	FMOD::Sound* BackGroundSound;
	FMOD::Channel* BackGroundChannel;

	FMOD::Sound* reloadSounds;
	FMOD::Channel* reloadChannel;

	FMOD::Sound* HeliShotDownSounds;
	FMOD::Channel* HeliShotDownChannel;

	FMOD::Sound* EmptyShotSounds;
	FMOD::Channel* EmptyShotChannel;

	FMOD::Sound* FallDownEmptyBulletSounds;
	FMOD::Channel* FallDownEmptyBulletChannel;
	
	FMOD::Sound* HeliWarrningHighSounds;
	FMOD::Channel* HeliWarrningHighChannel;

	FMOD::Sound* HealingSounds;
	FMOD::Channel* HealingChannel;

	FMOD_RESULT  result;
	void* extradriverdata = 0;
	
public:
	GameSound();
	~GameSound();
public:

	bool m_bStopSound = false;
	void SpeakMusic();
	void PlayHeliWarnningSound();
	void PlayHealingSound();
	void PauseHeliWarnningSound();
	void PlayFallDownEmptyBullet();
	void PlayEmptyShot();
	void PlayShotSound();
	void PlayHeliShotSound();
	void backGroundMusic();
	void HartBeatSound();
	void HumancollisionSound();
	void HeliiShotDownSound();
	void HelicollisionSound();
	void pauseHeartBeat();
	void PlayHearBeatSound();
	void HelicopterLoop();
	void PauseRotorLoop();
	void pauseOpeningSound();
	void PlayOpeningSound();
	void PlayNpcShotSound();
	void PauseNpcShotSound();
	void PlayHightLimitSound();
	void PauseHightLimitSound();
	void reloadSound();

};