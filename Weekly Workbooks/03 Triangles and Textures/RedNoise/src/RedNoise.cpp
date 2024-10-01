#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <TextureMap.h>


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

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour) {
    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float numberOfStep = std::max(abs(xDiff), abs(yDiff));
    float xStep = xDiff / numberOfStep;
    float yStep = yDiff / numberOfStep;

    uint32_t pixelColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;

    for (float i = 0.0; i <= numberOfStep; i++) {
        float x = from.x + i * xStep;
        float y = from.y + i * yStep;
        window.setPixelColour(round(x),round(y),pixelColour);
    }

}

void drawTriangle(DrawingWindow &window,CanvasTriangle triangleV, Colour colour) {

    drawLine(window,triangleV.vertices[0],triangleV.vertices[1],colour);
    drawLine(window,triangleV.vertices[1],triangleV.vertices[2],colour);
    drawLine(window,triangleV.vertices[2],triangleV.vertices[0],colour);

}

Colour generateRandomColour() {
    int red = rand() % 256;
    int green = rand() % 256;
    int blue = rand() % 256;
    return Colour(red, green, blue);
}


CanvasPoint generateRandomPoint() {
    float x = rand() % WIDTH;
    float y = rand() % HEIGHT;
    return CanvasPoint(x, y);
}

CanvasTriangle generateRandomTruangle() {
    CanvasPoint p1 = generateRandomPoint();
    CanvasPoint p2 = generateRandomPoint();
    CanvasPoint p3 = generateRandomPoint();

    CanvasTriangle randomTriangle(p1, p2, p3);
    return randomTriangle;
}


void drawFilledTriangles(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
    //排序
    if (triangle.vertices[0].y > triangle.vertices[1].y) {std::swap(triangle.vertices[0],triangle.vertices[1]);}
    if (triangle.vertices[0].y > triangle.vertices[2].y) {std::swap(triangle.vertices[0],triangle.vertices[2]);}
    if (triangle.vertices[1].y > triangle.vertices[2].y) {std::swap(triangle.vertices[1],triangle.vertices[2]);}

    CanvasPoint top = triangle.vertices[0];
    CanvasPoint middle = triangle.vertices[1];
    CanvasPoint bottom = triangle.vertices[2];

    for(float i = top.y; i <= middle.y; i++) {
        auto coefficient_Left = (i - top.y) / (middle.y - top.y);
        auto coefficient_Right = (i - top.y) / (bottom.y - top.y);

        auto x_left = top.x + coefficient_Left * (middle.x - top.x);
        auto x_right = top.x + coefficient_Right * (bottom.x - top.x);


        drawLine(window,CanvasPoint(x_left,i),CanvasPoint(x_right,i),colour);
    }

    for(float i = middle.y; i <= bottom.y; i++) {
        auto coefficient_Left = (i - middle.y) / (bottom.y - middle.y);
        auto coefficient_Right = (i - top.y) / (bottom.y - top.y);

        auto x_left = middle.x + coefficient_Left * (bottom.x - middle.x);
        auto x_right = top.x + coefficient_Right * (bottom.x - top.x);

        drawLine(window,CanvasPoint(x_left,i),CanvasPoint(x_right,i),colour);
    }

}
void drawTextureTriangle(DrawingWindow &window, CanvasTriangle triangle, TextureMap texture) {
    if (triangle.vertices[0].y > triangle.vertices[1].y) {std::swap(triangle.vertices[0],triangle.vertices[1]);}
    if (triangle.vertices[0].y > triangle.vertices[2].y) {std::swap(triangle.vertices[0],triangle.vertices[2]);}
    if (triangle.vertices[1].y > triangle.vertices[2].y) {std::swap(triangle.vertices[1],triangle.vertices[2]);}

    CanvasPoint top = triangle.vertices[0];
    CanvasPoint middle = triangle.vertices[1];
    CanvasPoint bottom = triangle.vertices[2];

    for(int i = top.y; i <= middle.y; i++) {
        auto coefficient_Left = (i - top.y) / (middle.y - top.y);
        auto coefficient_Right = (i - top.y) / (bottom.y - top.y);

        auto x_left = top.x + coefficient_Left * (middle.x - top.x);
        auto x_right = top.x + coefficient_Right * (bottom.x - top.x);


        //计算uv
        auto u_left= top.texturePoint.x + coefficient_Left * (middle.texturePoint.x - top.texturePoint.x);
        auto v_left = top.texturePoint.y + coefficient_Left * (middle.texturePoint.y - top.texturePoint.y);
        auto u_right = top.texturePoint.x + coefficient_Right * (bottom.texturePoint.x - top.texturePoint.x);
        auto v_right = top.texturePoint.y + coefficient_Right * (bottom.texturePoint.y - top.texturePoint.y);

        u_left /= texture.width;
        v_left /= texture.height;
        u_right /= texture.width;
        v_right /= texture.height;

        if (x_left > x_right) {
            std::swap(x_left, x_right);
            std::swap(u_left, u_right);
            std::swap(v_left, v_right);
        }

        for(int j = round(x_left); j <= round(x_right); j++) {
            float t = (j - x_left) / (x_right - x_left);

            //计算纹理内的相对坐标
            auto u = u_left + t * (u_right - u_left);
            auto v = v_left + t * (v_right - v_left);

            //计算纹理内的坐标
            int textureX = int(u * texture.width);
            int textureY = int(v * texture.height);

            //第 i 行第 j 列的像素会存储在 pixels 数组的索引为 i * width + j 处。
            uint32_t pixelColour = texture.pixels[textureY * texture.width + textureX];
            window.setPixelColour(j,i,pixelColour);


        }

    }

    for(int i = middle.y; i <= bottom.y; i++) {
        auto coefficient_Left = (i - middle.y) / (bottom.y - middle.y);
        auto coefficient_Right = (i - top.y) / (bottom.y - top.y);

        auto x_left = middle.x + coefficient_Left * (bottom.x - middle.x);
        auto x_right = top.x + coefficient_Right * (bottom.x - top.x);

        auto u_left = middle.texturePoint.x + coefficient_Left * (bottom.texturePoint.x - middle.texturePoint.x);
        auto v_left = middle.texturePoint.y + coefficient_Left * (bottom.texturePoint.y - middle.texturePoint.y);

        auto u_right = top.texturePoint.x + coefficient_Right * (bottom.texturePoint.x - top.texturePoint.x);
        auto v_right = top.texturePoint.y + coefficient_Right * (bottom.texturePoint.y - top.texturePoint.y);

        u_left /= texture.width;
        v_left /= texture.height;
        u_right /= texture.width;
        v_right /= texture.height;
        if (x_left > x_right) {
            std::swap(x_left, x_right);
            std::swap(u_left, u_right);
            std::swap(v_left, v_right);
        }

        for(int j = round(x_left); j <= round(x_right); j++) {
            float t = (j - x_left) / (x_right - x_left);

            //计算纹理内的相对坐标
            auto u = u_left + t * (u_right - u_left);
            auto v = v_left + t * (v_right - v_left);

            //计算纹理内的坐标
            int textureX = int(u * texture.width);
            int textureY = int(v * texture.height);

            //第 i 行第 j 列的像素会存储在 pixels 数组的索引为 i * width + j 处。
            uint32_t pixelColour = texture.pixels[textureY * texture.width + textureX];
            window.setPixelColour(j,i,pixelColour);
        }

    }



}

void draw(DrawingWindow &window) {
    // window.clearPixels();

    // CanvasPoint topLeft(0, 1);
    // CanvasPoint Right(WIDTH- 1, 1);
    // CanvasPoint center(WIDTH / 2, HEIGHT / 2);
    //CanvasPoint topCenter(WIDTH / 2, 0);
    // CanvasPoint bottomCenter(WIDTH / 2, HEIGHT);
    // CanvasPoint thirdWidthStart(WIDTH/3,HEIGHT / 2);
    // CanvasPoint thirdWidthEnd((WIDTH/3)*2,HEIGHT / 2);

    //Colour white(255,0,0);

    // drawLine(window, topLeft, center, black);
    // drawLine(window, topRight, center, black);
    // drawLine(window, topCenter, bottomCenter, black);
    // drawLine(window,thirdWidthStart, thirdWidthEnd, black);

    //CanvasTriangle triangleTest(topLeft,Right,center);
    //drawTriangle(window,triangleTest,white);

    //drawFilledTriangles(window,triangleTest,white);

    CanvasPoint v1(160, 10);
    v1.texturePoint = TexturePoint(195, 5);

    CanvasPoint v2(300, 230);
    v2.texturePoint = TexturePoint(395, 380);

    CanvasPoint v3(10, 150);
    v3.texturePoint = TexturePoint(65, 330);

    CanvasTriangle triangle(v1,v2,v3);
    TextureMap texture("../texture.ppm");

    drawTextureTriangle(window, triangle, texture);



}



void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
        else if (event.key.keysym.sym == SDLK_u) {
            Colour randomColour = generateRandomColour();
            CanvasTriangle randomTriangle = generateRandomTruangle();
            drawTriangle(window, randomTriangle, randomColour);
        }
        else if (event.key.keysym.sym == SDLK_f) {
            Colour randomColour = generateRandomColour();
            CanvasTriangle randomTriangle = generateRandomTruangle();
            drawFilledTriangles(window, randomTriangle, randomColour);
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}



int main(int argc, char *argv[]) {


    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        draw(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }


}