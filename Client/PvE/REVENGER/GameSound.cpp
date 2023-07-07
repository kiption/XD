#include "stdafx.h"
#include "GameSound.h"
GameSound::GameSound()
{
	result = FMOD::System_Create(&soundSystem);

	result = soundSystem->init(60, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/Shooting.mp3", FMOD_UNIQUE, 0, &shootSound);
	shootSound->setMode(FMOD_UNIQUE);


	result = soundSystem->init(64, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/CollisionSound.wav", FMOD_DEFAULT, 0, &ColliSound);
	result = ColliSound->setMode(FMOD_LOOP_OFF);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/helicopterfieldwithbirds.mp3", FMOD_DEFAULT, 0, &speakSound);
	result = speakSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/footStep.wav", FMOD_DEFAULT, 0, &bgmSound);
	result = bgmSound->setMode(FMOD_LOOP_OFF);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/Reload.wav", FMOD_UNIQUE, 0, &reloadSounds);
	result = bossSound->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("sound_walk.mp3", FMOD_DEFAULT, 0, &walkSound);
	result = walkSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("sound_run.mp3", FMOD_DEFAULT, 0, &runSound);
	result = runSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("sound_boss.mp3", FMOD_DEFAULT, 0, &bossSound);
	result = bossSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->playSound(walkSound, 0, true, &walkChannel);
	walkChannel->setVolume(0.6f);
	walkChannel->setPaused(true);
	result = soundSystem->playSound(runSound, 0, true, &runChannel);
	runChannel->setVolume(0.6f);
	runChannel->setPaused(true);
	result = soundSystem->playSound(bossSound, 0, true, &bossChannel);
	bossChannel->setVolume(0.3f);
	bossChannel->setPaused(true);

	//// 총 발사 사운드 채널 생성
	//result = soundSystem->playSound(shootSound, 0, false, &shootChannel);
	//shootChannel->setVolume(0.04f);
	//shootChannel->setMode(FMOD_LOOP_NORMAL);
	//shootChannel->setLoopCount(0);
	//shootChannel->setPaused(true);
}

GameSound::~GameSound()
{
	result = shootSound->release();
	result = ColliSound->release();
	result = speakSound->release();
	result = bgmSound->release();
	result = walkSound->release();
	result = bossSound->release();
	result = reloadSounds->release();

	result = soundSystem->close();
	result = soundSystem->release();


	//Common_Close();
}

void GameSound::shootingSound(bool Stop)
{
	bool isPlaying = true;

	//	shootChannel->stop();
	//	shootChannel->setMode(FMOD_LOOP_NORMAL);
	//	result = soundSystem->playSound(shootSound, 0, false, &shootChannel);
	//	shootChannel->setVolume(0.04f);
	//}


	result = soundSystem->playSound(shootSound, 0, false, &shootChannel);
	shootChannel->setVolume(0.04f);



}
void GameSound::backGroundMusic()
{
	result = soundSystem->playSound(bgmSound, 0, false, &bgmChannel);
	bgmChannel->setVolume(0.05f);
}
void GameSound::SpeakMusic()
{

	result = soundSystem->playSound(speakSound, 0, false, &speakChannel);
	speakChannel->setVolume(0.1f);
}
void GameSound::walkingSound()
{
	walkChannel->setPaused(false);
}

void GameSound::collisionSound()
{
	result = soundSystem->playSound(ColliSound, 0, false, &ColliChannel);
	ColliChannel->setVolume(0.02f);
}
void GameSound::reloadSound()
{
	result = soundSystem->playSound(reloadSounds, 0, false, &reloadChannel);
	reloadChannel->setVolume(0.3f);
	shootChannel->setPaused(true);

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


