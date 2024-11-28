#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <ModelTriangle.h>
#include <TextureMap.h>
#include <map>


using namespace std;
using namespace glm;
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

CanvasTriangle generateRandomTriangle() {
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



void drawFilledTrianglesWithDepth(DrawingWindow &window, CanvasTriangle triangle, Colour colour, float depth_buff[WIDTH][HEIGHT]) {

    if (triangle.vertices[0].y > triangle.vertices[1].y) {std::swap(triangle.vertices[0],triangle.vertices[1]);}
    if (triangle.vertices[0].y > triangle.vertices[2].y) {std::swap(triangle.vertices[0],triangle.vertices[2]);}
    if (triangle.vertices[1].y > triangle.vertices[2].y) {std::swap(triangle.vertices[1],triangle.vertices[2]);}

    CanvasPoint top = triangle.vertices[0];
    CanvasPoint middle = triangle.vertices[1];
    CanvasPoint bottom = triangle.vertices[2];

    for (int y = (int)ceil(top.y); y <= (int)floor(middle.y); y++) {
        auto coefficient_Left = (y - top.y) / (middle.y - top.y);
        auto coefficient_Right = (y - top.y) / (bottom.y - top.y);

        auto x_left = top.x + coefficient_Left * (middle.x - top.x);
        auto x_right = top.x + coefficient_Right * (bottom.x - top.x);

        auto z_start = top.depth + coefficient_Left * (middle.depth - top.depth);
        auto z_end = top.depth + coefficient_Right * (bottom.depth - top.depth);

        if (x_left > x_right) {
            std::swap(x_left, x_right);
            std::swap(z_start, z_end);
        }

        for (int x = (int)ceil(x_left); x <= (int)floor(x_right); x++) {
            auto coefficient = (x - x_left) / (x_right - x_left);
            auto z = z_start + coefficient * (z_end - z_start);
            auto inverse_z = 1.0 / z;

            if (inverse_z > depth_buff[x][y]) {
                depth_buff[x][y] = inverse_z;
                uint32_t pixelColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
                window.setPixelColour(x, y, pixelColour);
            }

        }

    }

    for (int y = (int)ceil(middle.y); y <= (int)floor(bottom.y); y++) {
        auto coefficient_Left = (y - middle.y) / (bottom.y - middle.y);
        auto coefficient_Right = (y - top.y) / (bottom.y - top.y);

        auto x_left = middle.x + coefficient_Left * (bottom.x - middle.x);
        auto x_right = top.x + coefficient_Right * (bottom.x - top.x);

        auto z_start = middle.depth + coefficient_Left * (bottom.depth - middle.depth);
        auto z_end = top.depth + coefficient_Right * (bottom.depth - top.depth);

        if (x_left > x_right) {
            std::swap(x_left, x_right);
            std::swap(z_start, z_end);
        }

        for (int x = (int)ceil(x_left); x <= (int)floor(x_right); x++) {
            auto coefficient = (x - x_left) / (x_right - x_left);
            auto z = z_start + coefficient * (z_end - z_start);



            if (z > depth_buff[x][y]) {
                    depth_buff[x][y] = z;
                    uint32_t pixelColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
                    window.setPixelColour(x, y, pixelColour);
            }

        }

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


        //扫描线与屏幕三角形左右边界的交点在纹理图像中的对应位置
        auto u_left= top.texturePoint.x + coefficient_Left * (middle.texturePoint.x - top.texturePoint.x);
        auto v_left = top.texturePoint.y + coefficient_Left * (middle.texturePoint.y - top.texturePoint.y);
        auto u_right = top.texturePoint.x + coefficient_Right * (bottom.texturePoint.x - top.texturePoint.x);
        auto v_right = top.texturePoint.y + coefficient_Right * (bottom.texturePoint.y - top.texturePoint.y);



        if (x_left > x_right) {
            std::swap(x_left, x_right);
            std::swap(u_left, u_right);
            std::swap(v_left, v_right);
        }

        for(int j = round(x_left); j <= round(x_right); j++) {
            float t = (j - x_left) / (x_right - x_left);

            //计算纹理内的坐标
            int u = u_left + t * (u_right - u_left);
            int v = v_left + t * (v_right - v_left);


            //第 i 行第 j 列的像素会存储在 pixels 数组的索引为 i * width + j 处。
            uint32_t pixelColour = texture.pixels[v * texture.width + u];
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


        if (x_left > x_right) {
            std::swap(x_left, x_right);
            std::swap(u_left, u_right);
            std::swap(v_left, v_right);
        }

        for(int j = round(x_left); j <= round(x_right); j++) {
            float t = (j - x_left) / (x_right - x_left);

            //计算纹理内的坐标
            int u = u_left + t * (u_right - u_left);
            int v = v_left + t * (v_right - v_left);


            //第 i 行第 j 列的像素会存储在 pixels 数组的索引为 i * width + j 处。
            uint32_t pixelColour = texture.pixels[v * texture.width + u];
            window.setPixelColour(j,i,pixelColour);
        }

    }
}

vector<ModelTriangle> readObjFile(string fileName, float scaleFactor) {
    // .mtl 文件
    ifstream mtlFile(fileName + ".mtl");
    string colorData;
    map<string, glm::vec3> color;
    std::vector<std::string> parts;
    string colorName;
    glm::vec3 colorNum;

    if (!mtlFile.is_open()) {
        cout << "Error opening .mtl file: " << fileName << endl;
    } else {
        while (getline(mtlFile, colorData)) {
            parts = split(colorData, ' ');
            if (parts[0] == "newmtl") {
                colorName = parts[1];

            }
            if (parts[0] == "Kd") {
                colorNum = glm::vec3(stof(parts[1]), stof(parts[2]), stof(parts[3]));
                color[colorName] = colorNum;
            }
        }
    }
    mtlFile.close();

    // .obj 文件
    ifstream objFile(fileName + ".obj");
    std::vector<glm::vec3> vertices;
    vector<ModelTriangle> modelTriangles;
    string lineData;
    std::vector<std::string> tokens;
    Colour colour;

    if (!objFile.is_open()) {
        std::cerr << "Error opening .obj file: " << fileName << std::endl;
    } else {
        while (getline(objFile, lineData)) {
            tokens = split(lineData, ' ');

            if (tokens[0] == "usemtl") {
                if (color.find(tokens[1]) != color.end()) {
                    glm::vec3 colorVec = color[tokens[1]];
                    colour = Colour(static_cast<int>(colorVec.r * 255), static_cast<int>(colorVec.g * 255), static_cast<int>(colorVec.b * 255));
                } else {
                    std::cerr << "Color " << tokens[1] << " not found!" << std::endl;
                }
            }

            if (tokens[0] == "v") {
                glm::vec3 vertex;
                vertex.x = stof(tokens[1]) * scaleFactor;
                vertex.y = stof(tokens[2]) * scaleFactor;
                vertex.z = stof(tokens[3]) * scaleFactor;
                vertices.push_back(vertex);

            }

            if (tokens[0] == "f") {
                std::vector<int> vertexIndices;
                for (int i = 1; i <= 3; i++) {
                    std::vector<std::string> indices = split(tokens[i], '/');
                    int vIndex = stoi(indices[0]) - 1;
                    vertexIndices.push_back(vIndex);

                }

                ModelTriangle triangle(vertices[vertexIndices[0]],
                                       vertices[vertexIndices[1]],
                                       vertices[vertexIndices[2]],
                                       colour);
                modelTriangles.push_back(triangle);

            }
        }
    }

    objFile.close();
    return modelTriangles;
}

CanvasPoint projectVertexOntoCanvasPoint(vec3 cameraPosition, float focalLength, vec3 vertexPosition) {

    int image_plane_scaling = 160;
    vec3 relativePosition = cameraPosition - vertexPosition;

    auto inverse_z = 1.0 / relativePosition.z;

    float u = -focalLength * (relativePosition.x / relativePosition.z) * image_plane_scaling + WIDTH / 2;
    float v = focalLength * (relativePosition.y / relativePosition.z) * image_plane_scaling + HEIGHT / 2;


    return CanvasPoint(u, v,inverse_z);

}

void draw(DrawingWindow &window) {
    window.clearPixels();

    vector<ModelTriangle> triangles = readObjFile("../models/cornell-box", 0.35);
    vec3 cameraPosition(0.0, 0.0, 4.0);
    float focalLength = 2.0;

    float depth_buff[WIDTH][HEIGHT];
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            depth_buff[x][y] = 0;
        }
    }


        for (const auto& triangle : triangles) {
            CanvasPoint v1 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[0]);
            CanvasPoint v2 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[1]);
            CanvasPoint v3 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[2]);
            CanvasTriangle canvasTriangle(v1, v2, v3);


            Colour white(255, 255, 255);
            //drawTriangle(window, canvasTriangle, white);
            drawFilledTrianglesWithDepth(window, canvasTriangle, triangle.colour, depth_buff);
        }
}





void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
        else if (event.key.keysym.sym == SDLK_u) {
            Colour randomColour = generateRandomColour();
            CanvasTriangle randomTriangle = generateRandomTriangle();
            drawTriangle(window, randomTriangle, randomColour);
        }
        else if (event.key.keysym.sym == SDLK_f) {
            Colour randomColour = generateRandomColour();
            CanvasTriangle randomTriangle = generateRandomTriangle();
            drawFilledTriangles(window, randomTriangle, randomColour);
        }
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


}