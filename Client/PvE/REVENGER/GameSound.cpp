#include "stdafx.h"
#include "GameSound.h"
GameSound::GameSound()
{
	result = FMOD::System_Create(&soundSystem);

	result = soundSystem->init(128, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/Shooting.mp3", FMOD_DEFAULT, 0, &shootSound);
	result = shootSound->setMode(FMOD_LOOP_OFF);

	result = soundSystem->init(64, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/CollisionSound.wav", FMOD_3D, 0, &ColliSound);
	result = ColliSound->setMode(FMOD_LOOP_OFF);

	result = soundSystem->init(64, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/Rotor.mp3", FMOD_DEFAULT, 0, &speakSound);
	result = speakSound->setMode(FMOD_LOOP_BIDI);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/RainingSound.mp3", FMOD_3D, 0, &bgmSound);
	result = bgmSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("sound_walk.mp3", FMOD_DEFAULT, 0, &walkSound);
	result = walkSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("sound_run.mp3", FMOD_DEFAULT, 0, &runSound);
	result = runSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("sound_boss.mp3", FMOD_DEFAULT, 0, &bossSound);
	result = bossSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->playSound(walkSound, 0, false, &walkChannel);
	walkChannel->setVolume(0.6f);
	walkChannel->setPaused(true);
	result = soundSystem->playSound(runSound, 0, false, &runChannel);
	runChannel->setVolume(0.6f);
	runChannel->setPaused(true);
	result = soundSystem->playSound(bossSound, 0, false, &bossChannel);
	bossChannel->setVolume(0.3f);
	bossChannel->setPaused(true);


}

GameSound::~GameSound()
{
	result = shootSound->release();
	result = ColliSound->release();
	result = speakSound->release();
	result = bgmSound->release();
	result = walkSound->release();
	result = bossSound->release();

	result = soundSystem->close();
	result = soundSystem->release();

	
	//Common_Close();
}

void GameSound::shootingSound()
{
	result = soundSystem->playSound(shootSound, 0, false, &shootChannel);
	shootChannel->setVolume(0.2f);
}
void GameSound::backGroundMusic()
{
	result = soundSystem->playSound(bgmSound, 0, false, &bgmChannel);
	bgmChannel->setVolume(0.45f);
}
void GameSound::SpeakMusic()
{
	result = soundSystem->playSound(speakSound, 0, m_bStopSound, &speakChannel);
	speakChannel->setVolume(0.06f);
}
void GameSound::walkingSound()
{
	walkChannel->setPaused(false);
}

void GameSound::collisionSound()
{
	result = soundSystem->playSound(ColliSound, 0, false, &ColliChannel);
	ColliChannel->setVolume(0.22f);
}

void GameSound::pauseWalking()
{
	walkChannel->setPaused(true);
}

void GameSound::runningSound()
{
	runChannel->setPaused(false);
}

void GameSound::pauseRunning()
{
	runChannel->setPaused(true);
}

void GameSound::startBossSound()
{
	bossChannel->setPaused(false);
}

void GameSound::pauseBossSound()
{
	bossChannel->setPaused(true);
}
