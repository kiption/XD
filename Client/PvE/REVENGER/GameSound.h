#pragma once
#include "inc/inc/fmod.hpp"
#include "inc/inc/fmod_errors.h"
// fmod dll
#pragma comment(lib, "lib/lib/64/fmod_vc.lib")
#pragma comment(lib, "lib/lib/64/fmodL_vc.lib")


class GameSound
{
private:	
	FMOD::System* soundSystem;
	FMOD::Sound* shootSound;
	FMOD::Channel* shootChannel;

	FMOD::Sound* ColliSound;
	FMOD::Channel* ColliChannel;

	FMOD::Sound* speakSound;
	FMOD::Channel* speakChannel;

	FMOD::Sound* bgmSound;
	FMOD::Channel* bgmChannel;

	FMOD::Sound* walkSound;
	FMOD::Channel* walkChannel;

	FMOD::Sound* runSound;
	FMOD::Channel* runChannel;

	FMOD::Sound* bossSound;
	FMOD::Channel* bossChannel;

	FMOD_RESULT  result;
	void* extradriverdata = 0;
	
public:
	GameSound();
	~GameSound();
public:

	bool m_bStopSound = false;
	void SpeakMusic();
	void shootingSound();
	void backGroundMusic();
	void walkingSound();
	void collisionSound();
	void pauseWalking();
	void runningSound();
	void pauseRunning();
	void startBossSound();
	void pauseBossSound();

};