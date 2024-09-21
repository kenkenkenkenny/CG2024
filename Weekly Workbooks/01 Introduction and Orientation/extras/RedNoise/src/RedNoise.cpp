#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>

#define WIDTH 320
#define HEIGHT 240

//判断圆
bool hit(double r, int x,int y, int a ,int b){
	// (x-a)²+(y-b)²=r²
	auto dis = std::pow(x-a,2) + std::pow(y-b,2);
	return dis <= r*r;
}

void draw(DrawingWindow &window,int r) {
	window.clearPixels();

	auto radiu = r;
	auto a = WIDTH/2;
	auto b = HEIGHT/2;

	uint32_t colour;

	for (int y = 0; y < window.height; y++) {
		for (int x = 0; x < window.width; x++) {

			if(hit(radiu,x,y,a,b)) {
				auto red = rand() % 256;
				auto green =  0.0;
				auto blue = 0.0;
				colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			}else {
				auto red = rand() % 256;
				auto green =  rand() % 256;
				auto blue = rand() % 256;
				colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			}


			window.setPixelColour(x, y, colour);
		}
	}
}



void handleEvent(SDL_Event event, DrawingWindow &window,int &radiu) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) {
			std::cout << "LEFT" << std::endl;
			radiu += 10;
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			std::cout << "RIGHT" << std::endl;
			radiu -= 10;
		}
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;

	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	auto ratiu = 10;
	while (true) {

		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window,ratiu);
		draw(window,ratiu);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();

	}


}
