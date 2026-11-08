#include "raylib.h"
#include <iostream>
#include "raymath.h"
#include "rlgl.h"
#include <array>
#include <cmath>


class Window{
public:
    Window(const int& screenW, const int& screenH, const char* title, 
        const int& fps = 60) {
            SetTraceLogLevel(LOG_NONE);
            InitWindow(screenW, screenH, title);
            SetTargetFPS(fps);
    };

    ~Window() noexcept {
        CloseWindow();
    }
};


class Cube{
    const float part_size = 0.95f;
    const float sticker_thick = 0.05f;
    
    struct CubePart{
        Vector3 local_pos;
        Matrix transform;

        // Порядок: 0-UP, 1-DOWN, 2-LEFT, 3-RIGHT, 4-FRONT, 5-BACK
        std::array<bool, 6> has_sticker;
    };
    
    std::array<CubePart, 27> parts;

public:

    const Color CUBE_PLASTIC = Color{ 21, 22, 26, 255 };
    const Color CUBE_WIRES = Color{ 58, 64, 78, 255 };

    const Color COLOR_UP = Color{ 240, 242, 245, 255 };
    const Color COLOR_DOWN = Color{ 242, 211, 56, 255 };
    const Color COLOR_LEFT = Color{ 242, 153, 74, 255 };
    const Color COLOR_RIGHT = Color{ 235, 87, 87, 255 };
    const Color COLOR_BACK = Color{ 53, 132, 227, 255 };
    const Color COLOR_FRONT = Color{ 46, 182, 125, 255 };

    Cube(){
        int index = 0;
        for (int x = -1; x <= 1; ++x){
            for (int y = -1; y <= 1; ++y){
                for (int z = -1; z <= 1; ++z){
                    parts[index].local_pos = 
                        Vector3{(float)x, (float)y, (float)z};
                    parts[index].transform = MatrixIdentity();

                    parts[index].has_sticker[0] = (y == 1);  // UP
                    parts[index].has_sticker[1] = (y == -1); // DOWN
                    parts[index].has_sticker[2] = (x == -1); // LEFT
                    parts[index].has_sticker[3] = (x == 1);  // RIGHT
                    parts[index].has_sticker[4] = (z == 1);  // FRONT
                    parts[index].has_sticker[5] = (z == -1); // BACK

                    ++index;
                }
            }
        }
    }

    void render() {
        for (const auto& part : parts) {
            rlPushMatrix(); 
            rlTranslatef(part.local_pos.x, part.local_pos.y, part.local_pos.z);
            rlMultMatrixf(MatrixToFloat(part.transform));
            {
                Vector3 origin = {0.0f, 0.0f, 0.0f};

                DrawCube(origin, part_size, part_size, 
                    part_size, CUBE_PLASTIC);
                DrawCubeWires(origin, part_size, part_size, 
                    part_size, CUBE_WIRES);
                
                if (part.has_sticker[4])
                    DrawCube(Vector3{ 0.0f, 0.0f, part_size / 2 }, 
                        part_size * 0.8f, part_size * 0.8f, 
                            sticker_thick, COLOR_FRONT);
                if (part.has_sticker[5])
                    DrawCube(Vector3{ 0.0f, 0.0f, -part_size / 2 }, 
                        part_size * 0.8f, part_size * 0.8f, 
                            sticker_thick, COLOR_BACK);
                if (part.has_sticker[0])
                    DrawCube(Vector3{ 0.0f, part_size / 2, 0.0f }, 
                        part_size * 0.8f, 
                            sticker_thick, part_size * 0.8f, COLOR_UP);
                if (part.has_sticker[1])
                    DrawCube(Vector3{ 0.0f, -part_size / 2, 0.0f }, 
                        part_size * 0.8f, 
                            sticker_thick, part_size * 0.8f, COLOR_DOWN);
                if (part.has_sticker[3])
                    DrawCube(Vector3{ part_size / 2, 0.0f, 0.0f }, 
                        sticker_thick, part_size * 0.8f, 
                            part_size * 0.8f, COLOR_RIGHT);
                if (part.has_sticker[2])
                    DrawCube(Vector3{ -part_size / 2, 0.0f, 0.0f }, 
                        sticker_thick, part_size * 0.8f, 
                            part_size * 0.8f, COLOR_LEFT);  

            }
            rlPopMatrix();
        }
    }

    void rotateLayer(char layer, bool clockwise) {
        float angle = clockwise ? (PI / 2.0f) : -(PI / 2.0f);

        Matrix rotX = MatrixRotateX(angle);
        Matrix rotY = MatrixRotateY(angle);
        Matrix rotZ = MatrixRotateZ(angle);

        for(auto& part : parts){
            float cx = std::round(part.local_pos.x);
            float cy = std::round(part.local_pos.y);
            float cz = std::round(part.local_pos.z);

            bool shouldRotate = false;
            Matrix currentRot = MatrixIdentity();

            if (layer == 'F' && cz == 1){
                shouldRotate = true; currentRot = rotZ; 
            }
            else if (layer == 'B' && cz == -1){
                shouldRotate = true; currentRot = rotZ; 
            }
            else if (layer == 'R' && cx == 1){
                shouldRotate = true; currentRot = rotX; 
            }
            else if (layer == 'L' && cx == -1){
                shouldRotate = true; currentRot = rotX; 
            }
            else if (layer == 'U' && cy == 1){
                shouldRotate = true; currentRot = rotY; 
            }
            else if (layer == 'D' && cy == -1){
                shouldRotate = true; currentRot = rotY; 
            }

            if (shouldRotate) {
                part.transform = MatrixMultiply(part.transform, currentRot);
                part.local_pos = Vector3Transform(part.local_pos, currentRot);
            }
        }
    }
};


int main(int argc, char const *argv[]){
    int screenW = 1200;
    int screenH = 800;

    for (int i = 1; i < argc; ++i){
        std::string arg = argv[i];
        if (arg == "--width" && i + 1 < argc) {
            screenW = std::stoi(argv[i + 1]);
            ++i;
        } else if (arg == "--height" && i + 1 < argc) {
            screenH = std::stoi(argv[i + 1]);
            ++i;
        }
    }

    std::cout << "Запуск игрового окна " + std::to_string(screenW) + 
        "x" + std::to_string(screenH) << std::endl;
    
    Window window(screenW, screenH, "Rubik's Cube 3D", 120);

    Camera3D camera = {0};
    camera.position = Vector3{6.0f, 6.0f, 6.0f};
    camera.target = Vector3{0.0f, 0.0f, 0.0f};
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Matrix cubeRotation = MatrixIdentity();
    const float sensitivity = 0.005f;
    Vector2 targetRotationSpeed = { 0.0f, 0.0f };
    Vector2 currentRotationSpeed = { 0.0f, 0.0f };
    const float friction = 0.93f;

    Cube cube;
    
    while (!WindowShouldClose()){
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mouseDelta = GetMouseDelta();
            targetRotationSpeed.x = mouseDelta.x * sensitivity;
            targetRotationSpeed.y = mouseDelta.y * sensitivity;
        } else {
            targetRotationSpeed.x *= friction;
            targetRotationSpeed.y *= friction;
        }

        currentRotationSpeed.x = Lerp(currentRotationSpeed.x, targetRotationSpeed.x, 0.2f);
        currentRotationSpeed.y = Lerp(currentRotationSpeed.y, targetRotationSpeed.y, 0.2f);
        if (Vector2Length(currentRotationSpeed) > 0.0001f) {
            Matrix rotationY = MatrixRotateY(currentRotationSpeed.x);
            Matrix rotationX = MatrixRotateX(currentRotationSpeed.y);
            cubeRotation = MatrixMultiply(cubeRotation, rotationY);
            cubeRotation = MatrixMultiply(cubeRotation, rotationX);
        }

        if (IsKeyPressed(KEY_F)) cube.rotateLayer('F', true);  // F
        if (IsKeyPressed(KEY_G)) cube.rotateLayer('F', false); // F'
        if (IsKeyPressed(KEY_R)) cube.rotateLayer('R', true);  // R
        if (IsKeyPressed(KEY_E)) cube.rotateLayer('R', false); // R'
        if (IsKeyPressed(KEY_U)) cube.rotateLayer('U', true);  // U
        if (IsKeyPressed(KEY_I)) cube.rotateLayer('U', false); // U'
        if (IsKeyPressed(KEY_B)) cube.rotateLayer('B', true);  // B
        if (IsKeyPressed(KEY_V)) cube.rotateLayer('B', false); // B'
        if (IsKeyPressed(KEY_L)) cube.rotateLayer('L', true);  // L
        if (IsKeyPressed(KEY_K)) cube.rotateLayer('L', false); // L'
        if (IsKeyPressed(KEY_D)) cube.rotateLayer('D', true);  // D
        if (IsKeyPressed(KEY_S)) cube.rotateLayer('D', false); // D'

        BeginDrawing();
        ClearBackground(Color{30, 32, 40, 255});

        BeginMode3D(camera);
        {
            rlPushMatrix(); 
            rlMultMatrixf(MatrixToFloat(cubeRotation)); 
            cube.render();
            rlPopMatrix();
        }
        EndMode3D();
        int startX = 20;
        int startY = 20;
        int spacing = 28;
        int fontSize = 20;

        DrawRectangle(startX - 15, startY - 10, 170, 200, 
            Color{ 15, 16, 20, 180 });
        DrawText("CUBE CONTROLS:", startX, startY, 16, GRAY);
        DrawText("U / I : UP",    startX, startY + spacing, 
            fontSize, cube.COLOR_UP);
        DrawText("D / S : DOWN",   startX, startY + spacing * 2, 
            fontSize, cube.COLOR_DOWN);
        DrawText("F / G : FRONT", startX, startY + spacing * 3, 
            fontSize, cube.COLOR_FRONT);
        DrawText("B / V : BACK",    startX, startY + spacing * 4, 
            fontSize, cube.COLOR_BACK);
        DrawText("R / E : RIGHT", startX, startY + spacing * 5, 
            fontSize, cube.COLOR_RIGHT);
        DrawText("L / K : LEFT",   startX, startY + spacing * 6, 
            fontSize, cube.COLOR_LEFT);
        DrawFPS(startX, startY + spacing * 7);

        EndDrawing();
    }

    return 0;
}
