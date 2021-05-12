#include "Main.h"


Main::Main()
{
}


Main::~Main()
{

}


void Main::Init()
{
	InitValues();
	InitText();
	InitSquare();
	InitChoice();
	InitLine();
	InitButton();
	this->program = BuildShader("shader.vert", "shader.frag");
	InputMapping("MouseLeft", SDL_BUTTON_LEFT);
	sound.Init();
	sound.playMusic();
}

void Main::DeInit() {

}

void Main::Update(float deltaTime)
{
	if (IsKeyUp("MouseLeft"))
	{
		mouseClick = true;
		SDL_Delay(150);
	}
}

void Main::Render()
{
	//Setting Viewport
	glViewport(0, 0, GetScreenWidth(), GetScreenHeight());

	//Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Set the background color
	glClearColor(11.0f / 255.0f, 30.0f / 255.0f, 63.0f / 255.0f, 1.0f);

	// Set orthographic projection
	mat4 projection;
	projection = ortho(0.0f, static_cast<GLfloat>(GetScreenWidth()), static_cast<GLfloat>(GetScreenHeight()), 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(glGetUniformLocation(this->program, "projection"), 1, GL_FALSE, value_ptr(projection));

	vec3 red = vec3(221.0f / 255.0f, 67.0f / 255.0f, 67.0f / 255.0f);
	vec3 blue = vec3(10.0f / 255.0f, 142.0f / 255.0f, 243.0f / 255.0f);
	vec3 white = vec3(1.0f, 1.0f, 1.0f);
	RenderText("SOS Kuy", 630, 30, 1.0f, white);
	RenderText("Player 1", 700, 120, 0.5f, blue);
	RenderText(to_string(playerScore[0]), 730, 170, 1.0f, blue);
	RenderText("Player 2", 700, 250, 0.5f, red);
	RenderText(to_string(playerScore[1]), 730, 300, 1.0f, red);
	RenderText("Board Size", 675, 500, 0.5f, white);
	RenderText(to_string(boardSize), 730, 530, 1.0f, white);

	RenderSquare();
	if (showChoice) RenderChoice();
	RenderLine();
	RenderButton();
}

void Main::InitValues() {
	showChoice = false;
	mouseClick = false;
	activeSquare = vec2(-1, -1);

	for (int i = 0; i < boardSize * boardSize; i++) {
		squareValues[i] = -1;
	}

	for (int i = 0; i < 8 + 6 * (boardSize - 3) * 2; i++) {
		lineTypes[i] = -1;
	}
}

void Main::InitText() {
	// Init Freetype
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		Err("ERROR::FREETYPE: Could not init FreeType Library");
	}
	FT_Face face;
	if (FT_New_Face(ft, FONTNAME, 0, &face)) {
		Err("ERROR::FREETYPE: Failed to load font");
	}

	FT_Set_Pixel_Sizes(face, 0, FONTSIZE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(pair<GLchar, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, NULL,
		GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Main::RenderText(string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color)
{
	// Activate corresponding render state
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	UseShader(this->program);

	glUniform3f(glGetUniformLocation(this->program, "ourColor"), color.x, color.y, color.z);
	glUniform1i(glGetUniformLocation(this->program, "text"), 1);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(this->program, "ourTexture"), 0);
	mat4 model;
	glUniformMatrix4fv(glGetUniformLocation(this->program, "model"), 1, GL_FALSE, value_ptr(model));
	glBindVertexArray(VAO);

	// Iterate through all characters
	string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];
		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;
		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character

		GLfloat vertices[] = {
			// Positions			// Texture Coords
			xpos + w,	ypos + h,	1.0f, 1.0f, // Bottom Right
			xpos + w,	ypos,		1.0f, 0.0f, // Top Right
			xpos,		ypos,		0.0f, 0.0f, // Top Left
			xpos,		ypos + h,	0.0f, 1.0f  // Bottom Left 
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_QUADS, 0, 4);
		// Now advance cursors for next glyph (note that advance is number of 1 / 64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels(2 ^ 6 = 64)
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}



void Main::InitSquare() {
	string squares[6] = { "blue.png", "blue_s.png", "blue_o.png", "red.png", "red_s.png", "red_o.png" };
	string squares_hover[3] = { "yellow.png", "yellow_s.png", "yellow_o.png" };

	glGenTextures(6, &square_texture[0]);
	glGenTextures(3, &square_hover_texture[0]);

	for (int i = 0; i < 6; i++) {
		// Load and create a texture 
		glBindTexture(GL_TEXTURE_2D, square_texture[i]); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Load, create texture 
		int width, height;
		unsigned char* image = SOIL_load_image(squares[i].c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	}

	for (int i = 0; i < 3; i++) {
		// Load and create a texture 
		glBindTexture(GL_TEXTURE_2D, square_hover_texture[i]); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height;
		unsigned char* image = SOIL_load_image(squares_hover[i].c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	}

	GLfloat vertices[] = {
		// Positions	// Texture Coords
		1,  1,			1.0f, 1.0f, // Bottom Right
		1,  0,			1.0f, 0.0f, // Top Right
		0,  0,			0.0f, 0.0f, // Top Left
		0,  1,			0.0f, 1.0f  // Bottom Left 
	};


	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glBindVertexArray(VAO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Main::InitChoice() {
	string choices[2] = { "white_s.png", "white_o.png" };

	glGenTextures(2, &choice_texture[0]);

	for (int i = 0; i < 2; i++) {
		// Load and create a texture 
		glBindTexture(GL_TEXTURE_2D, choice_texture[i]); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Load, create texture 
		int width, height;
		unsigned char* image = SOIL_load_image(choices[i].c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	}

	GLfloat vertices[] = {
		// Positions	// Texture Coords
		1,  1,			1.0f, 1.0f, // Bottom Right
		1,  0,			1.0f, 0.0f, // Top Right
		0,  0,			0.0f, 0.0f, // Top Left
		0,  1,			0.0f, 1.0f  // Bottom Left 
	};


	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glBindVertexArray(VAO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Main::InitLine() {
	string lines[8] = { "blue_horizontal.png", "blue_vertical.png", "blue_diagonal_up.png", "blue_diagonal_down.png"
		,"red_horizontal.png", "red_vertical.png", "red_diagonal_up.png", "red_diagonal_down.png" };

	glGenTextures(8, &line_texture[0]);

	for (int i = 0; i < 8; i++) {
		// Load and create a texture 
		glBindTexture(GL_TEXTURE_2D, line_texture[i]); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Load, create texture 
		int width, height;
		unsigned char* image = SOIL_load_image(lines[i].c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	}

	GLfloat vertices[] = {
		// Positions	// Texture Coords
		1,  1,			1.0f, 1.0f, // Bottom Right
		1,  0,			1.0f, 0.0f, // Top Right
		0,  0,			0.0f, 0.0f, // Top Left
		0,  1,			0.0f, 1.0f  // Bottom Left 
	};


	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glBindVertexArray(VAO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Main::InitButton() {
	string buttons[2] = { "minus.png", "plus.png" };
	string buttons_hover[2] = { "minus_hover.png", "plus_hover.png" };

	glGenTextures(2, &button_texture[0]);
	glGenTextures(2, &button_hover_texture[0]);

	for (int i = 0; i < 2; i++) {
		// Load and create a texture 
		glBindTexture(GL_TEXTURE_2D, button_texture[i]); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Load, create texture 
		int width, height;
		unsigned char* image = SOIL_load_image(buttons[i].c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

		// Load and create a texture 
		glBindTexture(GL_TEXTURE_2D, button_hover_texture[i]); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		image = SOIL_load_image(buttons_hover[i].c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	}

	GLfloat vertices[] = {
		// Positions	// Texture Coords
		1,  1,			1.0f, 1.0f, // Bottom Right
		1,  0,			1.0f, 0.0f, // Top Right
		0,  0,			0.0f, 0.0f, // Top Left
		0,  1,			0.0f, 1.0f  // Bottom Left 
	};


	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glBindVertexArray(VAO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Main::RenderSquare() {
	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate shader
	UseShader(this->program);
	glUniform1i(glGetUniformLocation(this->program, "text"), 0);

	glBindVertexArray(VAO2);
	for (int y = 0; y < boardSize; y++) {
		for (int x = 0; x < boardSize; x++) {
			int squareIndex = y * boardSize + x;
			int mouse_x = GetMouseCoords().x;
			int mouse_y = GetMouseCoords().y;
			float size = GetScreenHeight() / boardSize;
			float square_x = x * size;
			float square_y = y * size;
			bool isMouseInside = mouse_x > square_x && mouse_x < square_x + size
				&& mouse_y > square_y && mouse_y < square_y + size;
			if (isMouseInside && mouseClick) {
				if (squareValues[squareIndex] == -1) {
					showChoice = true;
					activeSquare.x = x;
					activeSquare.y = y;
				}
				mouseClick = false;
			}
			if (isMouseInside)
			{
				if (!(hoverSquare.x == x && hoverSquare.y == y))
				{
					sound.playSound(0);
				}
				hoverSquare.x = x;
				hoverSquare.y = y;
			}

			int value = squareValues[squareIndex];

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, isMouseInside || (activeSquare.x == x && activeSquare.y == y) ?
				square_hover_texture[(playerTurn * 3 + value + 1) % 3] : square_texture[playerTurn * 3 + value + 1]);
			glUniform1i(glGetUniformLocation(this->program, "ourTexture"), 0);

			mat4 model;
			model = translate(model, vec3(square_x, square_y, 0.0f));
			model = scale(model, vec3(size, size, 1));
			glUniformMatrix4fv(glGetUniformLocation(this->program, "model"), 1, GL_FALSE, value_ptr(model));

			glDrawArrays(GL_QUADS, 0, 4);
		}
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}

void Main::RenderChoice() {
	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate shader
	UseShader(this->program);
	glUniform1i(glGetUniformLocation(this->program, "text"), 0);

	glBindVertexArray(VAO2);
	for (int i = 0; i < 2; i++) {

		int x = GetMouseCoords().x;
		int y = GetMouseCoords().y;
		float size = GetScreenHeight() / 6;
		float choice_x = 600 + (300 - size * 2) / 2 + size * i;
		float choice_y = 370;
		bool isMouseInside = x > choice_x && x < choice_x + size && y > choice_y && y < choice_y + size;
		if (isMouseInside && mouseClick) {
			int active_x = activeSquare.x;
			int active_y = activeSquare.y;

			squareValues[active_y * boardSize + active_x] = i;
			CheckMatch();
			activeSquare.x = -1;
			activeSquare.y = -1;
			showChoice = false;
			mouseClick = false;
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (isMouseInside) ? square_hover_texture[i + 1] : choice_texture[i]);
		glUniform1i(glGetUniformLocation(this->program, "ourTexture"), 0);

		mat4 model;
		model = translate(model, vec3(choice_x, choice_y, 0.0f));
		model = scale(model, vec3(size, size, 1));
		glUniformMatrix4fv(glGetUniformLocation(this->program, "model"), 1, GL_FALSE, value_ptr(model));

		glDrawArrays(GL_QUADS, 0, 4);
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}

void Main::RenderLine() {
	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate shader
	UseShader(this->program);
	glUniform1i(glGetUniformLocation(this->program, "text"), 0);

	glBindVertexArray(VAO2);
	for (int i = 0; i < 8 + 6 * (boardSize - 3) * 2; i++) {
		if (lineTypes[i] == -1)continue;
		float height = lineTypes[i] == 0 ? GetScreenHeight() / boardSize : GetScreenHeight() / boardSize * 3;
		float width = lineTypes[i] == 1 ? GetScreenHeight() / boardSize : GetScreenHeight() / boardSize * 3;

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, line_texture[lineTypes[i] + 4 * linePlayers[i]]);
		glUniform1i(glGetUniformLocation(this->program, "ourTexture"), 0);

		mat4 model;
		model = translate(model, vec3(lines[i].x, lines[i].y, 0.0f));
		model = scale(model, vec3(width, height, 1));
		glUniformMatrix4fv(glGetUniformLocation(this->program, "model"), 1, GL_FALSE, value_ptr(model));

		glDrawArrays(GL_QUADS, 0, 4);
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}

void Main::RenderButton() {
	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate shader
	UseShader(this->program);
	glUniform1i(glGetUniformLocation(this->program, "text"), 0);

	glBindVertexArray(VAO2);
	for (int i = 0; i < 2; i++) {

		int x = GetMouseCoords().x;
		int y = GetMouseCoords().y;
		float size = 40;
		float choice_x = 600 + (300 - size * 4) / 2 + i * 3 * size;
		float choice_y = 530;
		bool isMouseInside = x > choice_x && x < choice_x + size && y > choice_y && y < choice_y + size;
		if (isMouseInside && mouseClick) {
			if (i == 0 && boardSize > 3) {
				boardSize--;
				InitValues();
			}
			else if (i == 1 && boardSize < 9) {
				boardSize++;
				InitValues();
			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (isMouseInside) ? button_hover_texture[i] : button_texture[i]);
		glUniform1i(glGetUniformLocation(this->program, "ourTexture"), 0);

		mat4 model;
		model = translate(model, vec3(choice_x, choice_y, 0.0f));
		model = scale(model, vec3(size, size, 1));
		glUniformMatrix4fv(glGetUniformLocation(this->program, "model"), 1, GL_FALSE, value_ptr(model));

		glDrawArrays(GL_QUADS, 0, 4);
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}

void Main::CheckMatch() {
	int x = activeSquare.x;
	int y = activeSquare.y;
	int size = GetScreenHeight() / boardSize;
	bool isMatch = false;

	if (squareValues[y * boardSize + x] == 0) { //if value = S
		//check left side
		if (x > 1 && squareValues[y * boardSize + (x - 1)] == 1 && squareValues[y * boardSize + (x - 2)] == 0) {
			NewLine(0, (x - 2) * size, y * size);
			isMatch = true;
		}

		//check right side
		if (x < boardSize - 2 && squareValues[y * boardSize + (x + 1)] == 1 && squareValues[y * boardSize + (x + 2)] == 0) {
			NewLine(0, x * size, y * size);
			isMatch = true;
		}

		//check above
		if (y > 1 && squareValues[(y - 1) * boardSize + x] == 1 && squareValues[(y - 2) * boardSize + x] == 0) {
			NewLine(1, x * size, (y - 2) * size);
			isMatch = true;
		}

		//check below
		if (y < boardSize - 2 && squareValues[(y + 1) * boardSize + x] == 1 && squareValues[(y + 2) * boardSize + x] == 0) {
			NewLine(1, x * size, y * size);
			isMatch = true;
		}

		//check diagonal left above
		if (x > 1 && y > 1
			&& squareValues[(y - 1) * boardSize + (x - 1)] == 1 && squareValues[(y - 2) * boardSize + (x - 2)] == 0) {
			NewLine(3, (x - 2) * size, (y - 2) * size);
			isMatch = true;
		}

		//check diagonal right above
		if (x < boardSize - 2 && y > 1
			&& squareValues[(y - 1) * boardSize + (x + 1)] == 1 && squareValues[(y - 2) * boardSize + (x + 2)] == 0) {
			NewLine(2, x * size, (y - 2) * size);
			isMatch = true;
		}

		//check diagonal left below
		if (x > 1 && y < boardSize - 2
			&& squareValues[(y + 1) * boardSize + (x - 1)] == 1 && squareValues[(y + 2) * boardSize + (x - 2)] == 0) {
			NewLine(2, (x - 2) * size, y * size);
			isMatch = true;
		}

		//check diagonal right below
		if (x < boardSize - 2 && y < boardSize - 2
			&& squareValues[(y + 1) * boardSize + (x + 1)] == 1 && squareValues[(y + 2) * boardSize + (x + 2)] == 0) {
			NewLine(3, x * size, y * size);
			isMatch = true;
		}
	}
	else { //value = O
		//check horizontal
		if (x > 0 && x < boardSize - 1
			&& squareValues[y * boardSize + (x - 1)] == 0 && squareValues[y * boardSize + (x + 1)] == 0) {
			NewLine(0, (x - 1) * size, y * size);
			isMatch = true;
		}

		//check vertical
		if (y > 0 && y < boardSize - 1
			&& squareValues[(y - 1) * boardSize + x] == 0 && squareValues[(y + 1) * boardSize + x] == 0) {
			NewLine(1, x * size, (y - 1) * size);
			isMatch = true;
		}

		//check diagonal up
		if (x > 0 && x < boardSize - 1 && y > 0 && y < boardSize - 1
			&& squareValues[(y + 1) * boardSize + (x - 1)] == 0 && squareValues[(y - 1) * boardSize + (x + 1)] == 0) {
			NewLine(2, (x - 1) * size, (y - 1) * size);
			isMatch = true;
		}

		//check diagonal down
		if (x > 0 && x < boardSize - 1 && y > 0 && y < boardSize - 1
			&& squareValues[(y - 1) * boardSize + (x - 1)] == 0 && squareValues[(y + 1) * boardSize + (x + 1)] == 0) {
			NewLine(3, (x - 1) * size, (y - 1) * size);
			isMatch = true;
		}
	}

	if (!isMatch) playerTurn = playerTurn == 0 ? 1 : 0; //if doesnt match, change turn
	else sound.playSound(1);
}

void Main::NewLine(int type, int x, int y) {
	int lineIndex = playerScore[0] + playerScore[1];
	linePlayers[lineIndex] = playerTurn;
	lineTypes[lineIndex] = type;
	lines[lineIndex] = vec2(x, y);
	playerScore[playerTurn]++;
}



int main(int argc, char** argv) {
	Main main;
	Engine::Game& game = main;
	game.Start("SOS Kuy", 900, 600, true, WindowFlag::WINDOWED, 60, 1);

	return 0;
}
