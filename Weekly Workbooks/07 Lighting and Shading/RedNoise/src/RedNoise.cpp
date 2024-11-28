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
#include <RayTriangleIntersection.h>




using namespace std;
using namespace glm;

#define WIDTH 640
#define HEIGHT 480
vec3 cameraPosition(0.0, 0.0, 4.0);
glm::mat3 cameraOrientation = glm::mat3(1);
bool isOrbit = false;
bool isWireframe = false;
bool isRasterised = false;
bool isRayTraced = false;
// vec3 box_light(0.0, 0.6,0.8);
vec3 box_light(0.0, 0.6,0.5);
 vec3 Spherlight(-0.5, 0.7f,1.0f);
// vec3 Spherlight(-0.5, 0.3f,2.0f);
// vec3 light(0.5, 0.8, 1.6);
float focalLength = 2.0;


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
        window.setPixelColour(round(x), round(y), pixelColour);
    }
}

void drawTriangle(DrawingWindow &window, CanvasTriangle triangleV, Colour colour) {
    drawLine(window, triangleV.vertices[0], triangleV.vertices[1], colour);
    drawLine(window, triangleV.vertices[1], triangleV.vertices[2], colour);
    drawLine(window, triangleV.vertices[2], triangleV.vertices[0], colour);
}

void drawFilledTriangles(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
    //排序
    if (triangle.vertices[0].y > triangle.vertices[1].y) { std::swap(triangle.vertices[0], triangle.vertices[1]); }
    if (triangle.vertices[0].y > triangle.vertices[2].y) { std::swap(triangle.vertices[0], triangle.vertices[2]); }
    if (triangle.vertices[1].y > triangle.vertices[2].y) { std::swap(triangle.vertices[1], triangle.vertices[2]); }

    CanvasPoint top = triangle.vertices[0];
    CanvasPoint middle = triangle.vertices[1];
    CanvasPoint bottom = triangle.vertices[2];

    for (float i = top.y; i <= middle.y; i++) {
        auto coefficient_Left = (i - top.y) / (middle.y - top.y);
        auto coefficient_Right = (i - top.y) / (bottom.y - top.y);

        auto x_left = top.x + coefficient_Left * (middle.x - top.x);
        auto x_right = top.x + coefficient_Right * (bottom.x - top.x);


        drawLine(window, CanvasPoint(x_left, i), CanvasPoint(x_right, i), colour);
    }

    for (float i = middle.y; i <= bottom.y; i++) {
        auto coefficient_Left = (i - middle.y) / (bottom.y - middle.y);
        auto coefficient_Right = (i - top.y) / (bottom.y - top.y);

        auto x_left = middle.x + coefficient_Left * (bottom.x - middle.x);
        auto x_right = top.x + coefficient_Right * (bottom.x - top.x);

        drawLine(window, CanvasPoint(x_left, i), CanvasPoint(x_right, i), colour);
    }
}

void drawFilledTrianglesWithDepth(DrawingWindow &window, CanvasTriangle triangle, Colour colour,
                                  float depth_buff[WIDTH][HEIGHT]) {
    if (triangle.vertices[0].y > triangle.vertices[1].y) { std::swap(triangle.vertices[0], triangle.vertices[1]); }
    if (triangle.vertices[0].y > triangle.vertices[2].y) { std::swap(triangle.vertices[0], triangle.vertices[2]); }
    if (triangle.vertices[1].y > triangle.vertices[2].y) { std::swap(triangle.vertices[1], triangle.vertices[2]); }

    CanvasPoint top = triangle.vertices[0];
    CanvasPoint middle = triangle.vertices[1];
    CanvasPoint bottom = triangle.vertices[2];

    for (int y = (int) ceil(top.y); y <= (int) floor(middle.y); y++) {
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

        for (int x = (int) ceil(x_left); x <= (int) floor(x_right); x++) {
            auto coefficient = (x - x_left) / (x_right - x_left);
            auto z = z_start + coefficient * (z_end - z_start);


            if (x > 0 && x < WIDTH && y > 0 && y < HEIGHT) {
                if (z > depth_buff[x][y]) {
                    depth_buff[x][y] = z;
                    uint32_t pixelColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
                    window.setPixelColour(x, y, pixelColour);
                }
            }
        }
    }

    for (int y = (int) ceil(middle.y); y <= (int) floor(bottom.y); y++) {
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

        for (int x = (int) ceil(x_left); x <= (int) floor(x_right); x++) {
            auto coefficient = (x - x_left) / (x_right - x_left);
            auto z = z_start + coefficient * (z_end - z_start);

            if (x > 0 && x < WIDTH && y > 0 && y < HEIGHT) {
                if (z > depth_buff[x][y]) {
                    depth_buff[x][y] = z;
                    uint32_t pixelColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
                    window.setPixelColour(x, y, pixelColour);
                }
            }
        }
    }
}

void drawTextureTriangle(DrawingWindow &window, CanvasTriangle triangle, TextureMap texture) {
    if (triangle.vertices[0].y > triangle.vertices[1].y) { std::swap(triangle.vertices[0], triangle.vertices[1]); }
    if (triangle.vertices[0].y > triangle.vertices[2].y) { std::swap(triangle.vertices[0], triangle.vertices[2]); }
    if (triangle.vertices[1].y > triangle.vertices[2].y) { std::swap(triangle.vertices[1], triangle.vertices[2]); }

    CanvasPoint top = triangle.vertices[0];
    CanvasPoint middle = triangle.vertices[1];
    CanvasPoint bottom = triangle.vertices[2];

    for (int i = top.y; i <= middle.y; i++) {
        auto coefficient_Left = (i - top.y) / (middle.y - top.y);
        auto coefficient_Right = (i - top.y) / (bottom.y - top.y);

        auto x_left = top.x + coefficient_Left * (middle.x - top.x);
        auto x_right = top.x + coefficient_Right * (bottom.x - top.x);


        //扫描线与屏幕三角形左右边界的交点在纹理图像中的对应位置
        auto u_left = top.texturePoint.x + coefficient_Left * (middle.texturePoint.x - top.texturePoint.x);
        auto v_left = top.texturePoint.y + coefficient_Left * (middle.texturePoint.y - top.texturePoint.y);
        auto u_right = top.texturePoint.x + coefficient_Right * (bottom.texturePoint.x - top.texturePoint.x);
        auto v_right = top.texturePoint.y + coefficient_Right * (bottom.texturePoint.y - top.texturePoint.y);


        if (x_left > x_right) {
            std::swap(x_left, x_right);
            std::swap(u_left, u_right);
            std::swap(v_left, v_right);
        }

        for (int j = round(x_left); j <= round(x_right); j++) {
            float t = (j - x_left) / (x_right - x_left);

            //计算纹理内的坐标
            int u = u_left + t * (u_right - u_left);
            int v = v_left + t * (v_right - v_left);


            //第 i 行第 j 列的像素会存储在 pixels 数组的索引为 i * width + j 处。
            uint32_t pixelColour = texture.pixels[v * texture.width + u];
            window.setPixelColour(j, i, pixelColour);
        }
    }

    for (int i = middle.y; i <= bottom.y; i++) {
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

        for (int j = round(x_left); j <= round(x_right); j++) {
            float t = (j - x_left) / (x_right - x_left);

            //计算纹理内的坐标
            int u = u_left + t * (u_right - u_left);
            int v = v_left + t * (v_right - v_left);


            //第 i 行第 j 列的像素会存储在 pixels 数组的索引为 i * width + j 处。
            uint32_t pixelColour = texture.pixels[v * texture.width + u];
            window.setPixelColour(j, i, pixelColour);
        }
    }
}

vec3 getNormal(ModelTriangle triangle) {
    vec3 edge1 = triangle.vertices[1] - triangle.vertices[0];
    vec3 edge2 = triangle.vertices[2] - triangle.vertices[0];
    vec3 normal = normalize(cross(edge1, edge2));

    vec3 rayDirection = triangle.vertices[0] - cameraPosition;

    bool front_face = dot(rayDirection, normal) < 0;
    normal = front_face ? normal : - normal;


    return normal;
}

vector<ModelTriangle> readObjFile(string fileName, float scaleFactor) {
    // .mtl 文件
    ifstream mtlFile("../models/textured-cornell-box.mtl");
    // ifstream mtlFile(fileName + ".mtl");
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
                    colour = Colour(static_cast<int>(colorVec.r * 255), static_cast<int>(colorVec.g * 255),
                                    static_cast<int>(colorVec.b * 255));
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
                                       colour
                                       );

                triangle.normal = getNormal(triangle);

                modelTriangles.push_back(triangle);
            }

        }
    }

    objFile.close();
    return modelTriangles;
}

CanvasPoint projectVertexOntoCanvasPoint(vec3 cameraPosition, float focalLength, vec3 vertexPosition) {
    int image_plane_scaling = 160;
    vec3 cameraToVertex = - (vertexPosition - cameraPosition);

    vec3 adjustedVector = cameraToVertex * cameraOrientation;


    float u = -focalLength * (adjustedVector.x / adjustedVector.z) * image_plane_scaling + WIDTH / 2;
    float v = focalLength * (adjustedVector.y / adjustedVector.z) * image_plane_scaling + HEIGHT / 2;

    float inverse_z = 1.0 / adjustedVector.z;

    return CanvasPoint(u, v, inverse_z);
}

glm::mat3 rotationMatrix(string axis, float theta) {
    if (axis == "x") {
        return glm::mat3(
            1.0, 0.0, 0.0,
            0.0, cos(theta), sin(theta),
            0.0, -sin(theta), cos(theta)
        );
    }
    if (axis == "y") {
        return glm::mat3(
            cos(theta), 0.0, -sin(theta),
            0.0, 1.0, 0.0,
            sin(theta), 0.0f, cos(theta)
        );
    }
    if (axis == "z") {
        return glm::mat3(
            cos(theta), sin(theta), 0.0,
            -sin(theta), cos(theta), 0.0,
            0.0, 1.0, 0.0
        );
    }
    return mat3(1);
}

void lookAt() {
    glm::vec3 forward = glm::normalize(- (vec3(0, 0, 0) - cameraPosition));
    glm::vec3 vertical = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(vertical, forward));
    glm::vec3 up = glm::normalize(glm::cross(forward, right));


    cameraOrientation = glm::mat3(
        right,
        up,
        forward
    );
}

RayTriangleIntersection getClosestValidIntersection(vec3 startPosition,vec3 rayDirection,const vector<ModelTriangle> &triangles, int thisTriangeIndex = -1) {
    RayTriangleIntersection closestIntersection;
    closestIntersection.distanceFromCamera = std::numeric_limits<float>::infinity();

    for(int i = 0; i < triangles.size(); i++) {
        if(i == thisTriangeIndex) { continue;}
        glm::vec3 e0 = triangles[i].vertices[1] - triangles[i].vertices[0];
        glm::vec3 e1 = triangles[i].vertices[2] - triangles[i].vertices[0];
        glm::vec3 SPVector = startPosition - triangles[i].vertices[0];
        glm::mat3 DEMatrix(-rayDirection, e0, e1);
        glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

        float t = possibleSolution.x;
        float u = possibleSolution.y;
        float v = possibleSolution.z;

        bool valid = false;
        if ((u >= 0.0) && (v >= 0.0) && (u + v <= 1.0) && t > 0) {
            valid = true;
        }
        vec3 intersectionPoint = startPosition + t * rayDirection;

        if(valid && t < closestIntersection.distanceFromCamera) {
            closestIntersection = RayTriangleIntersection(intersectionPoint,t,triangles[i],i);
        }

    }
    return closestIntersection;

}

bool isShadow(RayTriangleIntersection ray, vec3 lightPosition,const vector<ModelTriangle> &triangles) {

    vec3 shadowRayDirection = glm::normalize(lightPosition - ray.intersectionPoint);
    glm::vec3 shadowRayOrigin = ray.intersectionPoint;
    RayTriangleIntersection shadow_ray_intersection = getClosestValidIntersection(shadowRayOrigin, shadowRayDirection, triangles,ray.triangleIndex);
    float distanceToLight = glm::length(lightPosition - ray.intersectionPoint);

    if(shadow_ray_intersection.distanceFromCamera < distanceToLight) {
        return true;
    }
    return false;

}

float getSpecular(vec3 lightDirection, vec3 normal,int pow,vec3 viewDirection) {
    //R_i = R_i - 2 N (R_i · N)
    vec3 direction_r = lightDirection - 2.0f * normal * glm::dot(lightDirection, normal);
    //(VR)^n
    float specular = glm::dot(viewDirection,direction_r);
    specular = glm::max(0.0f, specular);
    float pow_spec = glm::pow(specular, pow);
    return pow_spec;

}

float getBrightness(RayTriangleIntersection intersection, vec3 rayDirection, vec3 normal,vec3 light) {


    vec3 vectorToLight = normalize(light - intersection.intersectionPoint);


    float angle = dot(normal, vectorToLight);
    float adjuctangle = glm::max(angle, 0.0f);

    float distanceToLight = glm::length(light - intersection.intersectionPoint);
    float proximityBrightness = 15.0f / (5.0f * M_PI * distanceToLight * distanceToLight);
    // proximityBrightness = std::min(std::max(proximityBrightness, 0.01f), 1.0f);



    // vec3 viewDirection = glm::normalize(cameraPosition - intersection.intersectionPoint);
    float specular = getSpecular(vectorToLight, normal,128,rayDirection);


    float brightness = proximityBrightness * adjuctangle + specular;

    brightness += 0.1f;

    float minimum_ambient = 0.2f;

    brightness = std::max(brightness, minimum_ambient);

    return glm::clamp(brightness, 0.0f, 1.0f);
}

void drawRaytrace(DrawingWindow &window,vector<ModelTriangle> triangles,vec3 light) {
    window.clearPixels();
    auto focalLength = 4.0;


    // vector<ModelTriangle> triangles = readObjFile("../models/cornell-box", 0.35);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            float imagePlaneX = (x - WIDTH / 2)/ 160.0;
            float imagePlaneY = (y - HEIGHT / 2)/ 160.0;

            glm::vec3 rayDirection = glm::normalize(vec3(imagePlaneX,-imagePlaneY,-focalLength));
            RayTriangleIntersection intersection = getClosestValidIntersection(cameraPosition, rayDirection, triangles);
            float brightness;
            //brightness = getBrightness(intersection);

            if(isShadow(intersection,box_light,triangles)) {
                brightness = 0.2f;
            }
            else {
               brightness = getBrightness(intersection,rayDirection,intersection.intersectedTriangle.normal,light);

            }
            Colour colour = intersection.intersectedTriangle.colour;
            uint32_t pixelColour = (255 << 24) + (int(colour.red*brightness) << 16) + (int(colour.green*brightness)<< 8) + int(colour.blue*brightness);
            window.setPixelColour(x, y, pixelColour);
        }
    }


}

void drawWireframe(DrawingWindow &window,vector<ModelTriangle> triangles) {
    window.clearPixels();

    // vector<ModelTriangle> triangles = readObjFile("../models/cornell-box", 0.35);


    if (isOrbit == true) {
        float angel = glm::radians(0.1f);
        mat3 rotationMatrixY = rotationMatrix("y", angel);
        cameraPosition = rotationMatrixY * cameraPosition;
        lookAt();
    }


    float focalLength = 2.0;

    float depth_buff[WIDTH][HEIGHT];
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            depth_buff[x][y] = 0;
        }
    }

    for (const auto &triangle: triangles) {
        CanvasPoint v1 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[0]);
        CanvasPoint v2 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[1]);
        CanvasPoint v3 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[2]);
        CanvasTriangle canvasTriangle(v1, v2, v3);


        Colour white(255, 0, 0);
        //drawTriangle(window, canvasTriangle, white);
        drawTriangle(window, canvasTriangle, triangle.colour);
    }
}

void drawRasterisedScene(DrawingWindow &window,vector<ModelTriangle> triangles) {
    window.clearPixels();
    auto focalLength = 4.0;

    //vector<ModelTriangle> triangles = readObjFile("../models/cornell-box", 0.35);

    if (isOrbit == true) {
        float angel = glm::radians(0.1f);
        mat3 rotationMatrixY = rotationMatrix("y", angel);
        cameraPosition = rotationMatrixY * cameraPosition;
        lookAt();
    }

    float depth_buff[WIDTH][HEIGHT];
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            depth_buff[x][y] = 0;
        }
    }

    for (const auto &triangle: triangles) {
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
    float angle = glm::radians(1.0);
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) {
            cameraPosition.x -= 0.1;
        } else if (event.key.keysym.sym == SDLK_RIGHT) {
            cameraPosition.x += 0.1;
        } else if (event.key.keysym.sym == SDLK_UP) {
            cameraPosition.y += 0.1;
        } else if (event.key.keysym.sym == SDLK_DOWN) {
            cameraPosition.y -= 0.1;
        } else if (event.key.keysym.sym == SDLK_w) {
            cameraPosition.z -= 0.1;
        } else if (event.key.keysym.sym == SDLK_s) {
            cameraPosition.z += 0.1;
        } else if (event.key.keysym.sym == SDLK_a) {
            glm::mat3 rotationMatrixX = rotationMatrix("x", angle);
            cameraOrientation = rotationMatrixX * cameraOrientation;
        } else if (event.key.keysym.sym == SDLK_d) {
            glm::mat3 rotationMatrixY = rotationMatrix("y", angle);
            cameraOrientation = rotationMatrixY * cameraOrientation;
        } else if (event.key.keysym.sym == SDLK_o) {
            isOrbit = !isOrbit;
        }
        else if (event.key.keysym.sym == SDLK_l) {
            isRasterised = false;
            isRayTraced = false;
            isWireframe = true;
        }
        else if (event.key.keysym.sym == SDLK_k) {
            isRasterised = true;
            isRayTraced = false;
            isWireframe = false;
        }
        else if (event.key.keysym.sym == SDLK_j) {
            isRasterised = false;
            isRayTraced = true;
            isWireframe = false;
        }
        if (event.key.keysym.sym == SDLK_1) {
            box_light.y += 0.1f; // Move light up
        } else if (event.key.keysym.sym == SDLK_2) {
            box_light.y -= 0.1f; // Move light down
        }
        if (event.key.keysym.sym == SDLK_z) {
            cameraPosition.z += 0.1f;
            focalLength += 0.2f;
        }
        if (event.key.keysym.sym == SDLK_y) {
            cameraPosition.z -= 0.1f;
            focalLength -= 0.1f;
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

std::vector<ModelTriangle> readSphere(float scaleFactor) {
    // .obj 文件
    std::string fileName = "../models/sphere.obj";
    std::ifstream objFile( fileName);
    std::vector<glm::vec3> vertices;
    std::vector<ModelTriangle> modelTriangles;
    std::string lineData;
    std::vector<std::string> tokens;
    Colour colour(255,0,0);


    if (!objFile.is_open()) {
        std::cerr << "Error opening .obj file: " << fileName << std::endl;
    } else {
        while (getline(objFile, lineData)) {
            tokens = split(lineData, ' ');


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
                                       colour
                                       );
                triangle.normal = getNormal(triangle);


                modelTriangles.push_back(triangle);
            }
        }
    }

    objFile.close();
    return modelTriangles;
}

vec3 reflect(vec3 ray, vec3 normal) {
    return ray - 2.0f * normal * dot(ray,normal);
}

glm::vec3 getVertexNormal(glm::vec3 vertex, vector<ModelTriangle> modelTriangles) {
    vec3 normals(0.0f,0.0f,0.0f);
    float numOfTriangles = 0.0f;
    for(const auto &triangle: modelTriangles) {
        if(triangle.vertices[0] == vertex || triangle.vertices[1] == vertex || triangle.vertices[2] == vertex) {
            normals += triangle.normal;
            numOfTriangles++;
        }
    }

        glm::vec3 vertexNormal = normals / static_cast<float>(numOfTriangles);
    return glm::normalize(vertexNormal);

}

glm::vec3 getNormalWeight(float x, float y, const ModelTriangle triangle) {

    glm::vec3 v0 = triangle.vertices[0];
    glm::vec3 v1 = triangle.vertices[1];
    glm::vec3 v2 = triangle.vertices[2];

    float t = (v2.x - v1.x) * (v0.y - v2.y)+ (v1.y - v2.y) * (v0.x - v2.x);
    float a = ((v1.y - v2.y) * (x - v2.x) + (v2.x - v1.x) * (y - v2.y)) / t;
    float b = ((v2.y - v0.y) * (x - v2.x) + (v0.x - v2.x) * (y - v2.y)) / t;
    float c = 1.0f - a - b;
    return vec3(a,b,c);


}

void gouraudShading(DrawingWindow &window, vector<ModelTriangle> triangles,vec3 light) {
    auto focalLength = 4.0;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            float imagePlaneX = (x - WIDTH / 2)/ 160.0;
            float imagePlaneY = (y - HEIGHT / 2)/ 160.0;

            glm::vec3 rayDirection = glm::normalize(vec3(imagePlaneX,-imagePlaneY,-focalLength));
            RayTriangleIntersection intersection = getClosestValidIntersection(cameraPosition, rayDirection, triangles);

                vec3 v0 = intersection.intersectedTriangle.vertices[0];
                vec3 v1 = intersection.intersectedTriangle.vertices[1];
                vec3 v2 = intersection.intersectedTriangle.vertices[2];

                glm::vec3 v0Normal = getVertexNormal(v0, triangles);
                glm::vec3 v1Normal = getVertexNormal(v1, triangles);
                glm::vec3 v2Normal = getVertexNormal(v2, triangles);


                float brightness0 = getBrightness(intersection,rayDirection,v0Normal,light);
                float brightness1 = getBrightness(intersection,rayDirection,v1Normal,light);
                float brightness2 = getBrightness(intersection,rayDirection,v2Normal,light);


                glm::vec3 weights = getNormalWeight(intersection.intersectionPoint.x, intersection.intersectionPoint.y, intersection.intersectedTriangle);
                float brightness = weights.x * brightness0 + weights.y * brightness1 + weights.z * brightness2;

                    Colour colour = intersection.intersectedTriangle.colour;
                    uint32_t pixelColour = (255 << 24) +
                                   (int(colour.red * brightness) << 16) +
                                   (int(colour.green * brightness) << 8) +
                                   int(colour.blue * brightness);
                    window.setPixelColour(x, y, pixelColour);


        }
    }
}

vec3 getInterpolatedNormal(vec3 v0Normal, vec3 v1Normal, vec3 v2Normal, vec3 weight) {
    return glm::normalize(weight.x * v0Normal + weight.y * v1Normal + weight.z * v2Normal);
}

float Proximity(const glm::vec3& lightPosition, const glm::vec3& intersectionPoint) {
    float distance = glm::length(lightPosition - intersectionPoint);
    return 20.0f / static_cast<float>(4 * M_PI * distance * distance);
}

float Incidence(const glm::vec3& lightPosition, const glm::vec3& normal,RayTriangleIntersection intersection) {
    glm::vec3 lightDirection = glm::normalize(lightPosition - intersection.intersectionPoint);
    return glm::dot(glm::normalize(normal), lightDirection);
}

float Specular(const glm::vec3& lightPosition,const glm::vec3& normal, float specularExponent,RayTriangleIntersection intersection) {
    glm::vec3 light_r = glm::normalize(lightPosition - intersection.intersectionPoint);
    glm::vec3 view_r = glm::normalize(cameraPosition-intersection.intersectionPoint);
    glm::vec3 reflectedRay = reflect(-light_r, normal);
    float reflectionAngle = glm::dot(glm::normalize(view_r), glm::normalize(reflectedRay));
    reflectionAngle=glm::max(0.0f,reflectionAngle);
    float SpecularLighting = pow(reflectionAngle, specularExponent);
    return SpecularLighting;
}

void phongShading(DrawingWindow &window, vector<ModelTriangle> triangles, vec3 &lightPosition, vec3 &cameraPosition, float pow) {
    auto focalLength = 4.0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            float imagePlaneX = (x - WIDTH / 2)/ 160.0;
            float imagePlaneY = (y - HEIGHT / 2)/ 160.0;
            glm::vec3 rayDirection = glm::normalize(vec3(imagePlaneX,-imagePlaneY,-focalLength));
            RayTriangleIntersection intersection = getClosestValidIntersection(cameraPosition, rayDirection, triangles);
            vec3 v0 = intersection.intersectedTriangle.vertices[0];
            vec3 v1 = intersection.intersectedTriangle.vertices[1];
            vec3 v2 = intersection.intersectedTriangle.vertices[2];

            glm::vec3 v0Normal = getVertexNormal(v0, triangles);
            glm::vec3 v1Normal = getVertexNormal(v1, triangles);
            glm::vec3 v2Normal = getVertexNormal(v2, triangles);

            glm::vec3 weights = getNormalWeight(intersection.intersectionPoint.x, intersection.intersectionPoint.y, intersection.intersectedTriangle);
            vec3 interpolatedNormal = getInterpolatedNormal(v0Normal,v1Normal,v2Normal,weights);

            auto ProximityLighting = Proximity(lightPosition,intersection.intersectionPoint);
            auto IncidenceLighting = Incidence(lightPosition,interpolatedNormal,intersection);
            auto specularLighting = Specular(lightPosition,interpolatedNormal,pow,intersection);

            Colour colour = intersection.intersectedTriangle.colour;
            colour.red = static_cast<uint32_t>(std::min(colour.red * ProximityLighting*IncidenceLighting + specularLighting * 255, 255.0f));
            colour.green = static_cast<uint32_t>(std::min(colour.green * ProximityLighting*IncidenceLighting + specularLighting * 255, 255.0f));
            colour.blue = static_cast<uint32_t>(std::min(colour.blue * ProximityLighting*IncidenceLighting + specularLighting * 255, 255.0f));

            int32_t pixelColour = (255 << 24) +(int(colour.red ) << 16) +(int(colour.green) << 8) +int(colour.blue);
            window.setPixelColour(x, y, pixelColour);
        }
    }
}

vector<ModelTriangle> readTextureObjFile(string fileName, float scaleFactor) {
    // .mtl 文件
    ifstream mtlFile("../models/textured-cornell-box.mtl");
    // ifstream mtlFile(fileName + ".mtl");
    string colorData;
    map<string, glm::vec3> color;
    std::vector<std::string> parts;
    string colorName;
    glm::vec3 colorNum;
    string materialName;

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
    vector<TexturePoint> texturePoints;
    bool isCobbles = false;

    if (!objFile.is_open()) {
        std::cerr << "Error opening .obj file: " << fileName << std::endl;
    } else {
        while (getline(objFile, lineData)) {
            tokens = split(lineData, ' ');

            if (tokens[0] == "usemtl") {
                materialName = tokens[1];
                if (color.find(materialName) != color.end()) {
                    glm::vec3 colorVec = color[tokens[1]];
                    colour = Colour(static_cast<int>(colorVec.r * 255), static_cast<int>(colorVec.g * 255),
                                    static_cast<int>(colorVec.b * 255));
                } else {
                    std::cerr << "Color " << tokens[1] << " not found!" << std::endl;
                }
                if (materialName == "Cobbles") {
                    isCobbles = true;
                } else {
                    isCobbles = false;
                }
            }

            if (tokens[0] == "v") {
                glm::vec3 vertex;
                vertex.x = stof(tokens[1]) * scaleFactor;
                vertex.y = stof(tokens[2]) * scaleFactor;
                vertex.z = stof(tokens[3]) * scaleFactor;
                vertices.push_back(vertex);
            }

            if (tokens[0] == "vt"){
                TextureMap texture("../models//texture.ppm");
                TexturePoint TP;
                TP.x = stof(tokens[1]) * texture.width; ;
                TP.y = stof(tokens[2]) * texture.height;
                texturePoints.push_back(TP);
                // cout << TP.x << " " << TP.y << endl;


            }

            if (tokens[0] == "f") {
                std::vector<int> vertexIndices;
                std::vector<int> textureIndices;
                for (int i = 1; i <= 3; i++) {
                    std::vector<std::string> indices = split(tokens[i], '/');
                    int vIndex = stoi(indices[0]) - 1;
                    vertexIndices.push_back(vIndex);
                    // cout << indices[0] << " " << indices[1] << " " << indices[2] << endl;
                    if (indices.size() > 1 && !indices[1].empty()) {
                        try {
                            int tIndex = stoi(indices[1]) - 1;
                            textureIndices.push_back(tIndex);
                            // cout << tIndex << endl;
                        } catch (const std::invalid_argument& e) {
                            cerr << "Error: Invalid texture index in face definition: " << tokens[i] << endl;
                        } catch (const std::out_of_range& e) {
                            cerr << "Error: Texture index out of range in face definition: " << tokens[i] << endl;
                        }
                    }

                }
                // cout << textureIndices.size() << " /"<< endl;

                ModelTriangle triangle(vertices[vertexIndices[0]],
                                       vertices[vertexIndices[1]],
                                       vertices[vertexIndices[2]],
                                       colour
                                       );
                if(isCobbles) {
                    triangle.texturePoints[0] = texturePoints[textureIndices[0]];
                    triangle.texturePoints[1] = texturePoints[textureIndices[1]];
                    triangle.texturePoints[2] = texturePoints[textureIndices[2]];
                }




                triangle.normal = getNormal(triangle);





                modelTriangles.push_back(triangle);
            }

        }
    }

    objFile.close();
    return modelTriangles;
}

glm::vec3 getCanvasWeight(int x, int y,CanvasTriangle triangle) {

    CanvasPoint v0 = triangle[0];
    CanvasPoint v1 = triangle[1];
    CanvasPoint v2 = triangle[2];

    float t = (v2.x - v1.x) * (v0.y - v2.y)+ (v1.y - v2.y) * (v0.x - v2.x);
    float a = ((v1.y - v2.y) * (x - v2.x) + (v2.x - v1.x) * (y - v2.y)) / t;
    float b = ((v2.y - v0.y) * (x - v2.x) + (v0.x - v2.x) * (y - v2.y)) / t;
    float c = 1.0f - a - b;
    return vec3(a,b,c);


}

void goodTureture(DrawingWindow &window,CanvasTriangle triangle, TextureMap texture, float depth_buff[WIDTH][HEIGHT]) {
    if (triangle[0].y > triangle[1].y) std::swap(triangle[0], triangle[1]);
    if (triangle[0].y > triangle[2].y) std::swap(triangle[0], triangle[2]);
    if (triangle[1].y > triangle[2].y) std::swap(triangle[1], triangle[2]);
    CanvasPoint v0 = triangle[0];
    CanvasPoint v1 = triangle[1];
    CanvasPoint v2 = triangle[2];
    int left_x = min({v0.x, v1.x, v2.x});
    int right_x = max({v0.x, v1.x, v2.x});

    for (int x = left_x; x <= right_x; x++) {
        for (int y = v0.y; y <= v2.y; y++) {
            if (x > 0 && x <= window.width && y > 0 && y <= window.height) {
                vec3 weight = getCanvasWeight(x, y, triangle);
                float a = weight.x;
                float b = weight.y;
                float c = weight.z;
                if (a >= 0 && b >= 0 && c >= 0) {
                    float depth = a * v0.depth + b * v1.depth + c * v2.depth;
                    if (depth > depth_buff[x][y]) {
                        depth_buff[x][y] = depth;
                        float tex_u = a * v0.texturePoint.x + b * v1.texturePoint.x + c * v2.texturePoint.x;
                        float tex_v = a * v0.texturePoint.y + b * v1.texturePoint.y + c * v2.texturePoint.y;
                        int tex_u_int = static_cast<int>(tex_u);
                        int tex_v_int = static_cast<int>(tex_v);
                        uint32_t colour = texture.pixels[tex_v_int * texture.width + tex_u_int];
                        // cout << colour << endl;
                        window.setPixelColour(x, y, colour);

                    }
                }
            }
        }
    }
}

void drawRasterisedSceneWithTexture(DrawingWindow &window,vector<ModelTriangle> triangles) {
    window.clearPixels();

    //vector<ModelTriangle> triangles = readObjFile("../models/cornell-box", 0.35);

    if (isOrbit == true) {
        float angel = glm::radians(0.1f);
        mat3 rotationMatrixY = rotationMatrix("y", angel);
        cameraPosition = rotationMatrixY * cameraPosition;
        lookAt();
    }


    float focalLength = 2.0;

    float depth_buff[WIDTH][HEIGHT];
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            depth_buff[x][y] = 0;
        }
    }
    TextureMap texture("../models/texture.ppm");
    for (const auto &triangle: triangles) {
        CanvasPoint v1 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[0]);
        CanvasPoint v2 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[1]);
        CanvasPoint v3 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[2]);
        CanvasTriangle canvasTriangle(v1, v2, v3);


        Colour white(255, 255, 255);
        //drawTriangle(window, canvasTriangle, white);


        if (triangle.texturePoints[0].x > 0 && triangle.texturePoints[1].x > 0 && triangle.texturePoints[2].x > 0 ) {

            canvasTriangle.v0().texturePoint = triangle.texturePoints[0];
            canvasTriangle.v1().texturePoint = triangle.texturePoints[1];
            canvasTriangle.v2().texturePoint = triangle.texturePoints[2];

            // drawTextureTriangle(window,canvasTriangle,texture);
            goodTureture(window,canvasTriangle,texture,depth_buff);

        }else {
            drawFilledTrianglesWithDepth(window, canvasTriangle, triangle.colour, depth_buff);
        }

    }
}

vector<vec3> getMultLight(vec3 centerLight, int numberOfLight, float radius){
    vector<vec3> lights;

    lights.push_back(centerLight);
    for (int i = 0; i < numberOfLight; i++) {
        float angle = (360.0f / numberOfLight) * i;
        float x = centerLight.x + radius * cos(angle * M_PI / 180.0f);
        float z = centerLight.z + radius * sin(angle * M_PI / 180.0f);
        lights.push_back(vec3(x, centerLight.y, z));
        // cout << vec3(x, centerLight.y, z).x << vec3(x, centerLight.y, z).y << vec3(x, centerLight.y, z).z << endl;
    }
    return lights;

}

void drawRayTracedSceneWithSoftShadow(DrawingWindow &window, vector<ModelTriangle> triangles,vec3 lightCenter) {
    window.clearPixels();
    auto focalLength = 2.0;

    vector<vec3> lights = getMultLight(lightCenter, 64, 0.5);



    // vector<ModelTriangle> triangles = readObjFile("../models/cornell-box", 0.35);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            float imagePlaneX = (x - WIDTH / 2)/ 160.0;
            float imagePlaneY = (y - HEIGHT / 2)/ 160.0;

            glm::vec3 rayDirection = glm::normalize(vec3(imagePlaneX,-imagePlaneY,-focalLength));
            RayTriangleIntersection intersection = getClosestValidIntersection(cameraPosition, rayDirection, triangles);
            float num_shdowPoint = 0;

            for(const vec3 &light : lights) {
                if (isShadow(intersection,light,triangles)) {
                    num_shdowPoint +=1.0f;
                }
            }

            float shadowIntensity = 1.0 - num_shdowPoint / lights.size();
            float brightness = getBrightness(intersection,rayDirection,intersection.intersectedTriangle.normal,lightCenter) * shadowIntensity;


            Colour colour = intersection.intersectedTriangle.colour;
            uint32_t pixelColour = (255 << 24) + (int(colour.red*brightness) << 16) + (int(colour.green*brightness)<< 8) + int(colour.blue*brightness);
            window.setPixelColour(x, y, pixelColour);
        }
    }

}

vector<ModelTriangle> readMirrorObjFile(string fileName, float scaleFactor) {
    // .mtl 文件
    ifstream mtlFile("../models/materials.mtl");
    // ifstream mtlFile(fileName + ".mtl");
    string colorData;
    map<string, glm::vec3> color;
    std::vector<std::string> parts;
    string colorName;
    glm::vec3 colorNum;
    string materialName;

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
    vector<TexturePoint> texturePoints;
    bool isCobbles = false;
    bool isMirror = false;
    bool isGlass = false;

    if (!objFile.is_open()) {
        std::cerr << "Error opening .obj file: " << fileName << std::endl;
    } else {
        while (getline(objFile, lineData)) {
            tokens = split(lineData, ' ');

            if (tokens[0] == "usemtl") {
                materialName = tokens[1];
                if (color.find(materialName) != color.end()) {
                    glm::vec3 colorVec = color[tokens[1]];
                    colour = Colour(static_cast<int>(colorVec.r * 255), static_cast<int>(colorVec.g * 255),
                                    static_cast<int>(colorVec.b * 255));
                } else {
                    std::cerr << "Color " << tokens[1] << " not found!" << std::endl;
                }
                if (materialName == "Cobbles") {
                    isCobbles = true;
                } else {
                    isCobbles = false;
                }
                isMirror = (materialName == "Mirror");
                isGlass = (materialName == "Glass");
                // cout << "isMirror: " << isMirror << endl;

            }

            if (tokens[0] == "v") {
                glm::vec3 vertex;
                vertex.x = stof(tokens[1]) * scaleFactor;
                vertex.y = stof(tokens[2]) * scaleFactor;
                vertex.z = stof(tokens[3]) * scaleFactor;
                vertices.push_back(vertex);
            }

            if (tokens[0] == "vt"){
                TextureMap texture("../models//texture.ppm");
                TexturePoint TP;
                TP.x = stof(tokens[1]) * texture.width; ;
                TP.y = stof(tokens[2]) * texture.height;
                texturePoints.push_back(TP);
                // cout << TP.x << " " << TP.y << endl;


            }

            if (tokens[0] == "f") {
                std::vector<int> vertexIndices;
                std::vector<int> textureIndices;
                for (int i = 1; i <= 3; i++) {
                    std::vector<std::string> indices = split(tokens[i], '/');
                    int vIndex = stoi(indices[0]) - 1;
                    vertexIndices.push_back(vIndex);
                    // cout << indices[0] << " " << indices[1] << " " << indices[2] << endl;
                    if (indices.size() > 1 && !indices[1].empty()) {
                        try {
                            int tIndex = stoi(indices[1]) - 1;
                            textureIndices.push_back(tIndex);
                            // cout << tIndex << endl;
                        } catch (const std::invalid_argument& e) {
                            cerr << "Error: Invalid texture index in face definition: " << tokens[i] << endl;
                        } catch (const std::out_of_range& e) {
                            cerr << "Error: Texture index out of range in face definition: " << tokens[i] << endl;
                        }
                    }

                }


                ModelTriangle triangle(vertices[vertexIndices[0]],
                                       vertices[vertexIndices[1]],
                                       vertices[vertexIndices[2]],
                                       colour
                                       );
                if(isCobbles) {
                    triangle.texturePoints[0] = texturePoints[textureIndices[0]];
                    triangle.texturePoints[1] = texturePoints[textureIndices[1]];
                    triangle.texturePoints[2] = texturePoints[textureIndices[2]];
                }
                if(isMirror) {
                    triangle.isMirror = isMirror;
                }
                if(isGlass) {
                    triangle.isGlass = isGlass;
                }


                // cout <<triangle.isMirror << endl;


                triangle.normal = getNormal(triangle);





                modelTriangles.push_back(triangle);
            }

        }
    }

    objFile.close();
    return modelTriangles;
}

Colour RayWithReflection(vec3 rayOrigin, vec3 rayDirection, vector<ModelTriangle> &triangles, vector<vec3> &lights, vec3 lightCenter, int depth) {
    if (depth <= 0) {
        return Colour(0, 0, 0);
    }

    RayTriangleIntersection intersection = getClosestValidIntersection(rayOrigin, rayDirection, triangles);

    if (intersection.distanceFromCamera == std::numeric_limits<float>::infinity()) {
        return Colour(0, 0, 0);
    }

    Colour colour = intersection.intersectedTriangle.colour;


    if (intersection.intersectedTriangle.isMirror) {

        vec3 reflectedDirection = reflect(rayDirection, intersection.intersectedTriangle.normal);
        vec3 origin = intersection.intersectionPoint + 0.001f * intersection.intersectedTriangle.normal;

        Colour reflectionColour = RayWithReflection(origin, reflectedDirection, triangles, lights, lightCenter, depth - 1);


        return reflectionColour;
    }


    float num_ShadowPoints = 0.0f;
    for (const vec3 &light : lights) {
        if (isShadow(intersection, light, triangles)) {
            num_ShadowPoints += 1.0f;
        }
    }

    float shadowIntensity = 1.0f - num_ShadowPoints / lights.size();
    float brightness = getBrightness(intersection, rayDirection, intersection.intersectedTriangle.normal, lightCenter) * shadowIntensity;


    colour.red = int(colour.red * brightness);
    colour.green = int(colour.green * brightness);
    colour.blue = int(colour.blue * brightness);


    return colour;
}

void drawRayTracedSceneWithSoftShadowandMirror(DrawingWindow &window, vector<ModelTriangle> triangles, vec3 lightCenter, int maxDepth = 5) {
    window.clearPixels();
    auto focalLength = 4.0;
    vector<vec3> lights = getMultLight(lightCenter, 64, 0.5);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            float imagePlaneX = (x - WIDTH / 2) / 160.0;
            float imagePlaneY = (y - HEIGHT / 2) / 160.0;

            glm::vec3 rayDirection = glm::normalize(vec3(imagePlaneX, -imagePlaneY, -focalLength));
            Colour color = RayWithReflection(cameraPosition, rayDirection, triangles, lights, lightCenter, maxDepth);

            uint32_t pixelColourValue = (255 << 24) + (int(color.red) << 16) + (int(color.green) << 8) + int(color.blue);
            window.setPixelColour(x, y, pixelColourValue);
        }
    }
}

vec2 calculateFresnelEffect(const glm::vec3& rayDirection, const glm::vec3& normal, float eta) {
    // formula https://www.cnblogs.com/starfallen/p/4019350.html
    float cosThetaI = glm::clamp(glm::dot(-rayDirection, normal), 0.0f, 1.0f);
    float sinThetaT = eta * sqrt(glm::max(0.0f, 1.0f - cosThetaI * cosThetaI));
    if (sinThetaT >= 1.0f) {
        return vec2(1.0f,0.0f);
    }
    float cosThetaT = sqrt(glm::max(0.0f, 1.0f - sinThetaT * sinThetaT));
    float rParallel = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);
    float rPerpendicular = (cosThetaI - eta * cosThetaT) / (cosThetaI + eta * cosThetaT);
    float k_r = (rParallel * rParallel + rPerpendicular * rPerpendicular) / 2.0f;
    return vec2(k_r,1-k_r);
}


vec3 refract(vec3 incident,vec3 normal, float eta) {
    auto cos_theta = std::fmin(dot(-incident, normal), 1.0f);
    vec3 r_out_perpendicular = eta * (incident + cos_theta*normal);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0f - glm::length(r_out_perpendicular))) * normal;
    return r_out_perpendicular + r_out_parallel;

}
Colour mixColoursWithFresnelEffect(const glm::vec2 &fresnelEffect, const Colour &refractionColour, const Colour &reflectionColour) {
    int red = static_cast<int>(fresnelEffect.y * refractionColour.red + fresnelEffect.x * reflectionColour.red);
    int green = static_cast<int>(fresnelEffect.y * refractionColour.green + fresnelEffect.x * reflectionColour.green);
    int blue = static_cast<int>(fresnelEffect.y * refractionColour.blue + fresnelEffect.x * reflectionColour.blue);

    return Colour(red, green, blue);
}


Colour RayWithGlass(vec3 rayOrigin, vec3 rayDirection, vector<ModelTriangle>& triangles, vector<vec3>& lights, vec3 lightCenter, int depth) {
    if (depth <= 0) {
        return Colour(0, 0, 0);
    }

    RayTriangleIntersection intersection = getClosestValidIntersection(rayOrigin, rayDirection, triangles);

    if (intersection.distanceFromCamera == std::numeric_limits<float>::infinity()) {
        return Colour(0, 0, 0);
    }

    Colour colour = intersection.intersectedTriangle.colour;

    if (intersection.intersectedTriangle.isGlass) {
        //water 1/1.333
        float eta = 1.0f / 1.333f;
        vec3 normal = intersection.intersectedTriangle.normal;

        if (glm::dot(rayDirection, normal) > 0.0f) {
            normal = -normal;
            eta = 1.333f / 1.0f;
        }

        // refract
        vec3 refractedDirection = refract(rayDirection, normal, eta);
        Colour refractionColour(0, 0, 0);
        if (!isnan(refractedDirection.x)) {
            vec3 refractedOrigin = intersection.intersectionPoint + refractedDirection * 0.001f;
            refractionColour = RayWithGlass(refractedOrigin, refractedDirection, triangles, lights, lightCenter, depth - 1);
        }

        //reflect
        vec3 reflectedDirection = reflect(rayDirection, normal);
        vec3 reflectedOrigin = intersection.intersectionPoint + normal * 0.001f;
        Colour reflectionColour = RayWithGlass(reflectedOrigin, reflectedDirection, triangles, lights, lightCenter, depth - 1);


        vec2 fresnelEffect = calculateFresnelEffect(rayDirection, normal, eta);


        return mixColoursWithFresnelEffect(fresnelEffect, refractionColour, reflectionColour);

    }

    float num_ShadowPoints = 0.0f;
    for (const vec3& light : lights) {
        if (isShadow(intersection, light, triangles)) {
            num_ShadowPoints += 1.0f;
        }
    }

    float shadowIntensity = 1.0f - num_ShadowPoints / lights.size();
    float brightness = getBrightness(intersection, rayDirection, intersection.intersectedTriangle.normal, lightCenter) * shadowIntensity;

    colour.red = int(colour.red * brightness);
    colour.green = int(colour.green * brightness);
    colour.blue = int(colour.blue * brightness);

    return colour;
}

void drawRayTracedSceneWithSoftShadowAndGlass(DrawingWindow& window, vector<ModelTriangle> triangles, vec3 lightCenter, int maxDepth = 5) {
    window.clearPixels();
    auto focalLength = 4.0;
    vector<vec3> lights = getMultLight(lightCenter, 64, 0.5);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            float imagePlaneX = (x - WIDTH / 2) / 160.0;
            float imagePlaneY = (y - HEIGHT / 2) / 160.0;

            glm::vec3 rayDirection = glm::normalize(vec3(imagePlaneX, -imagePlaneY, -focalLength));
            Colour color = RayWithGlass(cameraPosition, rayDirection, triangles, lights, lightCenter, maxDepth);

            uint32_t pixelColourValue = (255 << 24) + (int(color.red) << 16) + (int(color.green) << 8) + int(color.blue);
            window.setPixelColour(x, y, pixelColourValue);
        }
    }
}




int main(int argc, char *argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;
    std::vector<ModelTriangle> sphere = readSphere(0.35);
    std::vector<ModelTriangle> Tcornbox = readTextureObjFile("../models/textured-cornell-box",0.35);
    std::vector<ModelTriangle> cornbox = readTextureObjFile("../models/cornell-box",0.35);
    std::vector<ModelTriangle> Mirrorcornbox = readMirrorObjFile("../models/cornell-box_mirror",0.35);
    std::vector<ModelTriangle> box = readMirrorObjFile("../models/box",0.35);
    TextureMap texture("../models//texture.ppm");
    // vector<vec3> lights = getMultLight(vec3(0.0, 0.4f,0.8f), 4);

    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        // if(isWireframe) {
        //     drawWireframe(window,cornbox);
        // }else if (isRasterised) {
        // drawRasterisedScene(window,cornbox);
        // }else if(isRayTraced) {
        //     drawRaytrace(window,cornbox);
        // }

        // drawRasterisedScene(window,cornbox);
        // drawWireframe(window,cornbox);
        // drawRaytrace(window,cornbox,box_light);

        // drawRaytrace(window,sphere,Spherlight);
         // gouraudShading(window, sphere,Spherlight);
        // phongShading(window, sphere,Spherlight);


        // drawRasterisedSceneWithTexture(window, Tcornbox);
        // drawRayTracedSceneWithSoftShadow(window,cornbox,box_light);


        // drawRayTracedSceneWithSoftShadowandMirror(window,Mirrorcornbox,box_light,6);
        drawRayTracedSceneWithSoftShadowAndGlass(window, Mirrorcornbox, box_light, 6);


        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");

        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }

}
