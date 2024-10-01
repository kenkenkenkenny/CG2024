#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

#define WIDTH 320
#define HEIGHT 240

std::vector<float> interpolateSingleFloats(float from, float to,int numberOfValues) {
    //(to - from ) / (numberOfValues - 1)
    std::vector<float> result;
    float step = (to - from) / (numberOfValues - 1.0);

    for (int i = 0; i < numberOfValues; i++) {
        result.push_back(from + i * step);
    }

    return result;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from,glm::vec3 to, int numberOfValues) {
    std::vector<glm::vec3> result;
    glm::vec3 step = (to - from) / static_cast<float>(numberOfValues - 1.0);
    for (int i = 0; i < numberOfValues; i++) {
        result.push_back(from + static_cast<float>(i) * step);
    }
    return result;
}

// void draw(DrawingWindow &window) {
//     window.clearPixels();
//     std::vector<float> values = interpolateSingleFloats(0.0, 255.0, WIDTH);
//
//
//     for (size_t y = 0; y < window.height; y++) {
//         for (size_t x = 0; x < window.width; x++) {
//
//             float red = values[x];
//             float green =  values[x];
//             float blue =  values[x];
//
//             uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
//             window.setPixelColour(x, y, colour);
//         }
//     }
// }

void draw(DrawingWindow &window) {
    window.clearPixels();
    glm::vec3 topLeft(255, 0, 0);        // red
    glm::vec3 topRight(0, 0, 255);       // blue
    glm::vec3 bottomRight(0, 255, 0);    // green
    glm::vec3 bottomLeft(255, 255, 0);   // yellow

    std::vector<glm::vec3> leftColumn = interpolateThreeElementValues(topLeft, bottomLeft, HEIGHT);
    std::vector<glm::vec3> rightColumn = interpolateThreeElementValues(topRight, bottomRight, HEIGHT);

    for (size_t y = 0; y < window.height; y++) {
        std::vector<glm::vec3> rowColors = interpolateThreeElementValues(leftColumn[y], rightColumn[y], WIDTH);
        for (size_t x = 0; x < window.width; x++) {

            float red = rowColors[x].r;
            float green = rowColors[x].g;
            float blue = rowColors[x].b;

            uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
            window.setPixelColour(x, y, colour);
        }
    }
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}



int main(int argc, char *argv[]) {


    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, true);
    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        draw(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }

    // //test interpolateThreeElementValues
    // glm::vec3 from(1.0, 4.0, 9.2);
    // glm::vec3 to(4.0, 1.0, 9.8);
    // std::vector<glm::vec3> result;
    // result = interpolateThreeElementValues(from, to, 4);
    // for (const auto& vec : result) {
    //     std::cout << "Vec3: (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
    // }


    // //test interpolateSingleFloats
    // std::vector<float> result;
    // result = interpolateSingleFloats(2.2, 8.5, 7);
    // for(size_t i=0; i<result.size(); i++) std::cout << result[i] << " ";
    // std::cout << std::endl;


}