#include "application.h"
#include "mesh.h"
#include "shader.h"
#include "utils.h" 

#include <SDL.h>
#include <SDL_filesystem.h>
#include <SDL_messagebox.h>




Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	this->mouse_state = 0;
	this->time = 0.f;
	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(nullptr);

	this->framebuffer.Resize(w, h);



	fillMode = false;
	ImageFruit = false;
	borderWi = 2;

	//Image toolbar;
	//toolbar.LoadPNG("images/toolbar.png", false);
}

Application::~Application()
{
}

void Application::Init(void)
{
	std::cout << "Initiating app..." << std::endl;

	particleSystem.Init(framebuffer.width, framebuffer.height);

	lineButton = Button("images/line.png", 500, 10);
	circleButton = Button("images/circle.png", 550, 10);
	rectangleButton = Button("images/rectangle.png", 600, 10);
	clearButton = Button("images/clear.png", 10, 10);
	saveButton = Button("images/save.png", 50, 10);
	ColorRed = Button("images/red.png", 100, 10);
	ColorGreen = Button("images/green.png", 150, 10);
	ColorBlue = Button("images/blue.png", 200, 10);
	ColorCyan = Button("images/cyan.png", 250, 10);
	eraserButton = Button("images/eraser.png", 350, 10);
	loadImage = Button("images/load.png", 400, 10);
	ColorPurple = Button("images/pink.png", 300, 10);
	triangleButton = Button("images/triangle.png", 650, 10);
	fillButton = Button("images/fill.png", 450, 10);


	currentColor = Color::RED;
}

//Image Toolbar;

// Render one frame
void Application::Render(void) {

	if (tecla == 6) {

		particleSystem.Render(&framebuffer);
	}

	framebuffer.Render();
}


// Called after render
void Application::Update(float seconds_elapsed)
{

	if (tecla == 1) {

		framebuffer.Fill(Color(0, 0, 0));
		framebuffer.DrawLineDDA(100, 100, 100 + 50 * cos(time), 100 + 50 * sin(time), Color::RED);

	}

	if (tecla == 2) {

		framebuffer.Fill(Color(0, 0, 0));
		framebuffer.DrawRectUpdate(300, 300, 100, 50, Color::GREEN, borderWi, fillMode, Color::BLUE);

	}

	if (tecla == 3) {

		framebuffer.Fill(Color(0, 0, 0));
		framebuffer.DrawCircle(200, 400, 50, Color::PURPLE, borderWi, fillMode, Color::BLUE);

	}

	if (tecla == 4) {

		framebuffer.Fill(Color(0, 0, 0));
		framebuffer.DrawTriangle(Vector2(500.0f, 500.0f), Vector2(200.0f, 100.0f), Vector2(500.0f, 100.0f), Color::RED, borderWi, fillMode, Color::BLUE);
	}

	if (tecla == 5) {
		framebuffer.Fill(Color(0, 0, 0));
		if (ImageFruit) {

			Image Fruits;
			// Cargar y dibujar la imagen aquí
			const char* imagePath = "images/fruits.png";
			bool success = framebuffer.LoadPNG(imagePath, false);

			if (success) {
				framebuffer.DrawImage(Fruits, 0, 0, success);
				std::cout << "Image loaded and drawn successfully!" << std::endl;
			}
			else {
				std::cerr << "Error loading the image." << std::endl;
			}

		}

		framebuffer.DrawImage(lineButton.image, lineButton.position.x, lineButton.position.y, false);
		framebuffer.DrawImage(circleButton.image, circleButton.position.x, circleButton.position.y, false);
		framebuffer.DrawImage(rectangleButton.image, rectangleButton.position.x, rectangleButton.position.y, false);
		framebuffer.DrawImage(clearButton.image, clearButton.position.x, clearButton.position.y, false);
		framebuffer.DrawImage(saveButton.image, saveButton.position.x, saveButton.position.y, false);
		framebuffer.DrawImage(ColorRed.image, ColorRed.position.x, ColorRed.position.y, false);
		framebuffer.DrawImage(ColorGreen.image, ColorGreen.position.x, ColorGreen.position.y, false);
		framebuffer.DrawImage(ColorBlue.image, ColorBlue.position.x, ColorBlue.position.y, false);
		framebuffer.DrawImage(ColorCyan.image, ColorCyan.position.x, ColorCyan.position.y, false);
		framebuffer.DrawImage(eraserButton.image, eraserButton.position.x, eraserButton.position.y, false);
		framebuffer.DrawImage(loadImage.image, loadImage.position.x, loadImage.position.y, false);
		framebuffer.DrawImage(ColorPurple.image, ColorPurple.position.x, ColorPurple.position.y, false);
		framebuffer.DrawImage(triangleButton.image, triangleButton.position.x, triangleButton.position.y, false);
		framebuffer.DrawImage(fillButton.image, fillButton.position.x, fillButton.position.y, false);

		fillMode = false;

	}

	if (tecla == 6) {
		particleSystem.Update(seconds_elapsed);

	}

}

//keyboard press event 


void Application::OnKeyPressed(SDL_KeyboardEvent event)
{
	// KEY CODES: https://wiki.libsdl.org/SDL2/SDL_Keycode
	switch (event.keysym.sym) {
	case SDLK_ESCAPE: exit(0); break; // ESC key, kill the app
	case SDLK_1: tecla = 1; break;
	case SDLK_2: tecla = 2; break;
	case SDLK_3: tecla = 3; break;
	case SDLK_4: tecla = 4; break;
	case SDLK_5: tecla = 5; break;
	case SDLK_6: tecla = 6; break;
	case SDLK_f:
		if (fillMode) { fillMode = false; }
		else { fillMode = true; }
		break;

	case SDLK_PLUS: case SDLK_KP_PLUS: borderWi++; break;
	case SDLK_MINUS: case SDLK_KP_MINUS:
		if (borderWi > 1) {
			borderWi--;
		}
		break;
	}

}

void Application::OnMouseButtonDown(SDL_MouseButtonEvent event)
{

	if (event.button == SDL_BUTTON_LEFT) {
		Vector2 mousePosition(event.x, float(event.y) - float(framebuffer.height));
		std::cout << "Mouse Position: (" << mouse_position.x << ", " << mouse_position.y << ")" << std::endl;

		if (clearButton.IsMouseInside(mousePosition)) {
			framebuffer.Fill(Color(0, 0, 0));
			ImageFruit = false;
			tecla = 5;
			return;
		}

		else if (saveButton.IsMouseInside(mousePosition)) {
			const char* filename = "output.tga";
			// Call the SaveTGA function
			if (framebuffer.SaveTGA(filename))
			{
				// Image saved successfully
				// You can add additional logic or UI feedback here
				std::cout << "Image saved successfully!" << std::endl;
			}
		}

		else if (loadImage.IsMouseInside(mousePosition)) {
			ImageFruit = true;
		}

		else if (ColorBlue.IsMouseInside(mousePosition)) {
			currentColor = Color::BLUE;
		}
		else if (ColorRed.IsMouseInside(mousePosition)) {
			currentColor = Color::RED;
		}
		else if (ColorCyan.IsMouseInside(mousePosition)) {
			currentColor = Color::CYAN;
		}
		else if (ColorGreen.IsMouseInside(mousePosition)) {
			currentColor = Color::GREEN;
		}

		else if (ColorPurple.IsMouseInside(mousePosition)) {
			currentColor = Color::PURPLE;
		}

		else if (fillButton.IsMouseInside(mousePosition)) {
			if (!fillMode) {
				fillMode = true;
			}

			else {
				fillMode = false;
			}
		}
		else if (lineButton.IsMouseInside(mousePosition)) {
			framebuffer.DrawLineDDA(line_start.x, -line_start.y, line_end.x, -line_end.y, currentColor);
		}
		else if (circleButton.IsMouseInside(mousePosition)) {
			float scale_factor = 1.2f;
			float radius = sqrt(pow(line_end.x - line_start.x, 2) + pow(-(line_end.y - line_start.y), 2));
			framebuffer.DrawCircle(line_start.x, -line_start.y, radius, currentColor, 2, fillMode, currentColor);
		}
		else if (rectangleButton.IsMouseInside(mousePosition)) {
			framebuffer.DrawRectUpdate(line_start.x, -line_start.y, line_end.x, -line_end.y, currentColor, 2, fillMode, currentColor);
		}

		else if (triangleButton.IsMouseInside(mousePosition)) {

			framebuffer.DrawTriangle(Vector2(line_end.x,-line_end.y), Vector2(line_end.x, -line_start.y), Vector2(line_start.x, -line_start.y), currentColor, 2,fillMode, currentColor);
		}

		else if (eraserButton.IsMouseInside(mousePosition)) {
			erase1 = true;
			return;
		}



		else {
			// Set the starting point for drawing lines
			line_start.x = event.x;
			line_start.y = float(event.y) - float(framebuffer.height);
			draw = true;

			if (erase1) {
				erase = true;
			}
			tecla = -1;
		}
	}
}

void Application::OnMouseButtonUp(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_LEFT && draw == true) {
		line_end.x = event.x;
		line_end.y = float(event.y) - float(framebuffer.height);
		draw = false;
		tecla = -1;
		erase = false;
		erase1 = false;

	}

}


void Application::OnMouseMove(SDL_MouseButtonEvent event)
{
	if (draw) {
		mouse_delta.x = event.x - mouse_position.x;
		mouse_delta.y = event.y - mouse_position.y;

		// Update the mouse position
		mouse_position.x = event.x;
		mouse_position.y = event.y;
		if (erase1) {
			int erase_radius = 5; // Set your desired erase radius here

			// Erase pixels in a square around the mouse position
			for (int i = -erase_radius; i <= erase_radius; ++i) {
				for (int j = -erase_radius; j <= erase_radius; ++j) {
					int x = event.x + i;
					int y = event.y + j - int(framebuffer.height);

					// Check if the pixel is within the framebuffer bounds
					if (x >= 0 && x < framebuffer.width && -y >= 0 && -y < framebuffer.height) {
						framebuffer.SetPixel(x, -y, Color::BLACK);
					}
				}
			}
		}


	}


}

void Application::OnWheel(SDL_MouseWheelEvent event)
{
	float dy = event.preciseY;

	// ...
}

void Application::OnFileChanged(const char* filename)
{
	Shader::ReloadSingleShader(filename);
}