#include "stdafx.h"
#include "GameSound.h"
GameSound::GameSound()
{
	result = FMOD::System_Create(&soundSystem);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/Machin.wav", FMOD_CREATESTREAM, 0, &shotSound);
	shotSound->setMode(FMOD_CREATESTREAM);


	result = soundSystem->init(64, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/PlayerHittingSound.wav", FMOD_UNIQUE, 0, &ColliSound);
	result = ColliSound->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/helicopterfieldwithbirds.mp3", FMOD_DEFAULT, 0, &speakSound);
	result = speakSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/footStep.wav", FMOD_DEFAULT, 0, &bgmSound);
	result = bgmSound->setMode(FMOD_LOOP_OFF);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/Reload.wav", FMOD_UNIQUE, 0, &reloadSounds);
	result = BackGroundSound->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/HeartBeat.mp3", FMOD_UNIQUE, 0, &HartbeatSound);
	result = HartbeatSound->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/RotorLoop.wav", FMOD_DEFAULT, 0, &RotorSound);
	result = RotorSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/BackGround.mp3", FMOD_DEFAULT, 0, &BackGroundSound);
	result = BackGroundSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/Shooting.mp3", FMOD_CREATESTREAM, 0, &NpcshotSound);
	NpcshotSound->setMode(FMOD_CREATESTREAM);

	result = soundSystem->playSound(RotorSound, 0, true, &RotorChannel);
	RotorChannel->setVolume(0.5f);

	result = soundSystem->playSound(HartbeatSound, 0, true, &HartbeatChannel);
	HartbeatChannel->setVolume(0.5f);

	result = soundSystem->playSound(BackGroundSound, 0, true, &BackGroundChannel);
	BackGroundChannel->setVolume(0.1f);

	result = soundSystem->playSound(NpcshotSound, 0, true, &NpcshootChannel);
	NpcshootChannel->setVolume(0.25f);

	//result = soundSystem->playSound(shotSound, 0, true, &shotChannel);
	//shotChannel->setVolume(0.5f);
}

GameSound::~GameSound()
{
	result = shotSound->release();
	result = ColliSound->release();
	result = speakSound->release();
	result = bgmSound->release();
	result = HartbeatSound->release();
	result = BackGroundSound->release();
	result = reloadSounds->release();
	result = soundSystem->close();
	result = soundSystem->release();


	//Common_Close();
}

void GameSound::PlayShotSound()
{
	result = soundSystem->playSound(shotSound, 0, false, &shotChannel);
	shotChannel->setVolume(0.5f);

}
void GameSound::PauseshotingSound()
{
	shotChannel->setPaused(false);

}
void GameSound::backGroundMusic()
{
	result = soundSystem->playSound(bgmSound, 0, false, &bgmChannel);
	bgmChannel->setVolume(0.5f);
}
void GameSound::SpeakMusic()
{
	result = soundSystem->playSound(speakSound, 0, false, &speakChannel);
	speakChannel->setVolume(0.7f);
}
void GameSound::HartBeatSound()
{

}

void GameSound::collisionSound()
{
	result = soundSystem->playSound(ColliSound, 0, false, &ColliChannel);
	ColliChannel->setVolume(0.5f);
}

void GameSound::reloadSound()
{
	result = soundSystem->playSound(reloadSounds, 0, false, &reloadChannel);
	reloadChannel->setVolume(0.5f);
	shotChannel->setPaused(true);

}
void GameSound::pauseHeartBeat()
{
	HartbeatChannel->setPaused(true);
}

void GameSound::PlayHearBeatSound()
{
	HartbeatChannel->setPaused(false);
}

void GameSound::HelicopterLoop()
{
	RotorChannel->setPaused(false);
}

void GameSound::PauseRotorLoop()
{
	RotorChannel->setPaused(true);
}

void GameSound::pauseOpeningSound()
{
	BackGroundChannel->setPaused(true);
}

void GameSound::PlayOpeningSound()
{
	BackGroundChannel->setPaused(false);

}

void GameSound::PlayNpcShotSound()
{
	NpcshootChannel->setPaused(false);
}

void GameSound::PauseNpcShotSound()
{
	NpcshootChannel->setPaused(true);
}
