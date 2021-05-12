#ifndef MAIN_H
#define MAIN_H


#include <SOIL/SOIL.h>

#include <SDL/SDL_mixer.h>
#include <SDL/SDL_thread.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <map>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "Game.h"
#include "Sound.h"
//#include "Text.h"

using namespace glm;

#define FONTSIZE 60
#define FONTNAME "SF Intellivised.ttf"

struct Character {
	GLuint TextureID; // ID handle of the glyph texture
	ivec2 Size; // Size of glyph
	ivec2 Bearing; // Offset from baseline to left/top of glyph
	GLuint Advance; // Offset to advance to next glyph
};

class Main :

	public Engine::Game
{
public:
	Main();
	~Main();
	virtual void Init();
	virtual void DeInit();
	virtual void Update(float deltaTime);
	virtual void Render();
private:
	void InitText();
	void RenderText(string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color);
	void InitValues();
	void InitSquare();
	void InitChoice();
	void InitLine();
	void InitButton();
	void RenderSquare();
	void RenderChoice();
	void RenderLine();
	void RenderButton();
	void CheckMatch();
	void NewLine(int type, int x, int y);
	map<GLchar, Character> Characters;
	GLuint  VBO, VBO2, VAO, VAO2, program;
	GLuint square_texture[6], square_hover_texture[3];
	GLuint button_texture[2], button_hover_texture[2];
	GLuint choice_texture[2], line_texture[8];
	int boardSize = 6, playerTurn = 0, choice = -1;
	bool showChoice, mouseClick;
	int  playerScore[2];
	int squareValues[9 * 9], lineTypes[8 + 6 * (9 - 3) * 2], linePlayers[8 + 6 * (9 - 3) * 2];
	vec2 activeSquare, hoverSquare;
	vec2 lines[8 + 6 * (9 - 3) * 2];
	Sound sound;
};
#endif

