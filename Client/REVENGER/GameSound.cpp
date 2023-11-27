#include "stdafx.h"
#include "GameSound.h"
GameSound::GameSound()
{
	result = FMOD::System_Create(&soundSystem);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/Shooting.mp3", FMOD_CREATESTREAM, 0, &shotSound);
	shotSound->setMode(FMOD_CREATESTREAM);

	result = soundSystem->init(16, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/HeliShot.wav", FMOD_UNIQUE, 0, &HelishotSound);
	HelishotSound->setMode(FMOD_UNIQUE);

	result = soundSystem->init(64, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/PlayerHittingSound.wav", FMOD_UNIQUE, 0, &HumanColliSound);
	result = HumanColliSound->setMode(FMOD_UNIQUE);

	result = soundSystem->init(64, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/CollisionSound.wav", FMOD_UNIQUE, 0, &HeliColliSound);
	result = HeliColliSound->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/helicopterfieldwithbirds.mp3", FMOD_DEFAULT, 0, &speakSound);
	result = speakSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/footStep.wav", FMOD_DEFAULT, 0, &bgmSound);
	result = bgmSound->setMode(FMOD_LOOP_OFF);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/Rifle_Reload.wav", FMOD_UNIQUE, 0, &reloadSounds);
	result = BackGroundSound->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/HeartBeat.mp3", FMOD_LOOP_NORMAL, 0, &HartbeatSound);
	result = HartbeatSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/AlamSound.mp3", FMOD_UNIQUE, 0, &WarnningSound);
	result = WarnningSound->setMode(FMOD_UNIQUE);

	result = soundSystem->init(16, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/HealingSound.wav", FMOD_UNIQUE, 0, &HealingSounds);
	result = HealingSounds->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/HighLimit.mp3", FMOD_UNIQUE, 0, &HeliWarrningHighSounds);
	result = HeliWarrningHighSounds->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/RotorLoop.wav", FMOD_DEFAULT, 0, &RotorSound);
	result = RotorSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = soundSystem->createSound("Sound/RobbyScene.mp3", FMOD_DEFAULT, 0, &BackGroundSound);
	result = BackGroundSound->setMode(FMOD_LOOP_NORMAL);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/Shooting.mp3", FMOD_UNIQUE, 0, &NpcshotSound);
	NpcshotSound->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/Explosive.mp3", FMOD_UNIQUE, 0, &HeliShotDownSounds);
	HeliShotDownSounds->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/BulletFallDown.wav", FMOD_UNIQUE, 0, &FallDownEmptyBulletSounds);
	FallDownEmptyBulletSounds->setMode(FMOD_UNIQUE);

	result = soundSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, extradriverdata);
	result = soundSystem->createSound("Sound/EmptyFire.mp3", FMOD_UNIQUE, 0, &EmptyShotSounds);
	EmptyShotSounds->setMode(FMOD_UNIQUE);

	result = soundSystem->playSound(RotorSound, 0, true, &RotorChannel);
	RotorChannel->setVolume(0.2f);

	result = soundSystem->playSound(HartbeatSound, 0, true, &HartbeatChannel);
	HartbeatChannel->setVolume(1.5f);

	result = soundSystem->playSound(WarnningSound, 0, true, &WarnningChannel);
	WarnningChannel->setVolume(0.5f);

	result = soundSystem->playSound(BackGroundSound, 0, true, &BackGroundChannel);
	BackGroundChannel->setVolume(0.05f);

	result = soundSystem->playSound(NpcshotSound, 0, true, &NpcshootChannel);
	NpcshootChannel->setVolume(0.25f);

	result = soundSystem->playSound(HeliWarrningHighSounds, 0, true, &HeliWarrningHighChannel);
	HeliWarrningHighChannel->setVolume(0.05f);


	result = soundSystem->playSound(HealingSounds, 0, true, &HealingChannel);
	HealingChannel->setVolume(0.9f);

	shotChannel->setVolume(0.1f);
	HelishotChannel->setVolume(0.3f);
	reloadChannel->setVolume(0.5f);
}

GameSound::~GameSound()
{
	result = shotSound->release();
	result = FallDownEmptyBulletSounds->release();
	result = HumanColliSound->release();
	result = HeliWarrningHighSounds->release();
	result = HeliColliSound->release();
	result = WarnningSound->release();
	result = HeliShotDownSounds->release();
	result = EmptyShotSounds->release();
	result = HealingSounds->release();
	result = speakSound->release();
	result = bgmSound->release();
	result = HartbeatSound->release();
	result = WarnningSound->release();
	result = BackGroundSound->release();
	result = reloadSounds->release();
	result = soundSystem->close();
	result = soundSystem->release();


	//Common_Close();
}

void GameSound::PauseHeliWarnningSound()
{
	WarnningChannel->setPaused(true);
}
void GameSound::PlayHeliWarnningSound()
{
	WarnningChannel->setPaused(false);
}
void GameSound::PlayHealingSound()
{
	result = soundSystem->playSound(HealingSounds, 0, false, &HealingChannel);
	HealingChannel->setVolume(1.2f);
}
void GameSound::PlayFallDownEmptyBullet()
{
	result = soundSystem->playSound(FallDownEmptyBulletSounds, 0, false, &FallDownEmptyBulletChannel);
	FallDownEmptyBulletChannel->setVolume(0.8f);
}
void GameSound::PlayEmptyShot()
{
	result = soundSystem->playSound(EmptyShotSounds, 0, false, &EmptyShotChannel);
	EmptyShotChannel->setVolume(1.5f);
}

void GameSound::PlayShotSound()
{
	result = soundSystem->playSound(shotSound, 0, false, &shotChannel);
}
void GameSound::PlayHeliShotSound()
{
	result = soundSystem->playSound(HelishotSound, 0, false, &HelishotChannel);
}
void GameSound::backGroundMusic()
{
	result = soundSystem->playSound(bgmSound, 0, false, &bgmChannel);
	bgmChannel->setVolume(0.05f);
}
void GameSound::SpeakMusic()
{
	result = soundSystem->playSound(speakSound, 0, false, &speakChannel);
	speakChannel->setVolume(1.0f);
}
void GameSound::HartBeatSound()
{
}
void GameSound::HumancollisionSound()
{
	result = soundSystem->playSound(HumanColliSound, 0, false, &HumanColliChannel);
	HumanColliChannel->setVolume(0.5f);
}

void GameSound::HeliiShotDownSound()
{
	result = soundSystem->playSound(HeliShotDownSounds, 0, false, &HeliShotDownChannel);
	HeliShotDownChannel->setVolume(1.5f);
}

void GameSound::HelicollisionSound()
{
	result = soundSystem->playSound(HeliColliSound, 0, false, &HeliColliChannel);
	HeliColliChannel->setVolume(0.5f);
}

void GameSound::reloadSound()
{
	result = soundSystem->playSound(reloadSounds, 0, false, &reloadChannel);

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

void GameSound::PlayHightLimitSound()
{
	//HeliWarrningHighChannel->setPaused(false);
	result = soundSystem->playSound(HeliWarrningHighSounds, 0, false, &HeliWarrningHighChannel);
	HeliWarrningHighChannel->setVolume(0.15f);
}
void GameSound::PauseHightLimitSound()
{
	HeliWarrningHighChannel->setPaused(true);
}