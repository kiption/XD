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

	FMOD::Sound* ColliSound;
	FMOD::Channel* ColliChannel;

	FMOD::Sound* speakSound;
	FMOD::Channel* speakChannel;

	FMOD::Sound* bgmSound;
	FMOD::Channel* bgmChannel;

	FMOD::Sound* HartbeatSound;
	FMOD::Channel* HartbeatChannel;

	FMOD::Sound* RotorSound;
	FMOD::Channel* RotorChannel;

	FMOD::Sound* BackGroundSound;
	FMOD::Channel* BackGroundChannel;

	FMOD::Sound* reloadSounds;
	FMOD::Channel* reloadChannel;

	FMOD_RESULT  result;
	void* extradriverdata = 0;
	
public:
	GameSound();
	~GameSound();
public:

	bool m_bStopSound = false;
	void SpeakMusic();
	void PlayShotSound();
	void PlayHeliShotSound();
	void backGroundMusic();
	void HartBeatSound();
	void collisionSound();
	void pauseHeartBeat();
	void PlayHearBeatSound();
	void HelicopterLoop();
	void PauseRotorLoop();
	void pauseOpeningSound();
	void PlayOpeningSound();
	void PlayNpcShotSound();
	void PauseNpcShotSound();
	void reloadSound();

};