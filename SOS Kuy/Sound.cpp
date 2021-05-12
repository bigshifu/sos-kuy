#include "Sound.h"

Sound::Sound()
{
}

Sound::~Sound()
{
	if (Mix_Playing(sfx_channel) == 0) {
		Mix_FreeChunk(sound0);
		Mix_FreeChunk(sound1);
	}
	if (music != NULL) {
		Mix_FreeMusic(music);
	}
	Mix_CloseAudio();
}

void Sound::Init()
{
	InitAudio();
}

void Sound::DeInit()
{
}

void Sound::Update(float deltaTime)
{
}

void Sound::Render()
{
	playMusic();
}

void Sound::InitAudio()
{
	int flags = MIX_INIT_MP3 | MIX_INIT_FLAC | MIX_INIT_OGG;
	if (flags != Mix_Init(flags)) {
		Err("Unable to initialize mixer: " + string(Mix_GetError()));
	}

	int audio_rate = 22050; Uint16 audio_format = AUDIO_S16SYS; int audio_channels = 2; int audio_buffers = 4096;

	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
		Err("Unable to initialize audio: " + string(Mix_GetError()));
	}


	music = Mix_LoadMUS("background_music.ogg");
	if (music == NULL) {
		Err("Unable to load Music file: " + string(Mix_GetError()));
	}

	sound0 = Mix_LoadWAV("sfx_hover.wav");
	if (sound0 == NULL) {
		Err("Unable to load WAV file: " + string(Mix_GetError()));
	}

	sound1 = Mix_LoadWAV("sfx_line.wav");
	if (sound1 == NULL) {
		Err("Unable to load WAV file: " + string(Mix_GetError()));
	}
}

void Sound::playMusic()
{
	if (Mix_PlayingMusic() == 0)
	{
		//Play the music
		Mix_PlayMusic(music, -1);
		SDL_Delay(150);
	}
	//If music is being played
	else
	{
		//If the music is paused
		if (Mix_PausedMusic() == 1)
		{
			//Resume the music
			Mix_ResumeMusic();
			SDL_Delay(150);
		}
		//If the music is playing
		else
		{
			//Pause the music
			Mix_PauseMusic();
			SDL_Delay(150);
		}
	}
}

void Sound::playSound(int index)
{
	if (Mix_Playing(sfx_channel) == 0) {
		if (index == 0)	sfx_channel = Mix_PlayChannel(-1, sound0, 0);
		else sfx_channel = Mix_PlayChannel(-1, sound1, 0);
		if (sfx_channel == -1) {
			Err("Unable to play WAV file: " + string(Mix_GetError()));
		}
	}
}

