#ifndef SOUND_H
#define SOUND_H


#include <SOIL/SOIL.h>

#include <SDL/SDL_mixer.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "Game.h"

using namespace glm;

class Sound :
	public Engine::Game
{
public:
	Sound();
	~Sound();
	virtual void Init();
	virtual void DeInit();
	virtual void Update(float deltaTime);
	virtual void Render();
	void InitAudio();
	void playMusic();
	void playSound(int index);
private:
	Mix_Chunk* sound0 = NULL;
	Mix_Chunk* sound1 = NULL;
	Mix_Music* music = NULL;
	int sfx_channel = -1;
};
#endif

