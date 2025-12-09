/**
Rafid Ahmed
Pong Video Game
**/
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
enum AppStatus { RUNNING, TERMINATED };
constexpr int WINDOW_WIDTH  = 640,
              WINDOW_HEIGHT = 480;
constexpr float BG_RED     = 0.5f,
                BG_GREEN   = 0.5f,
                BG_BLUE    = 0.5f,
                BG_OPACITY = 1.0f;
constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";
constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL    = 0,
                TEXTURE_BORDER     = 0;
constexpr char PADDLE_SPRITE_FILEPATH[] = "paddle.png",
               DEEP_SEA_SPRITE_FILEPATH[] = "deepSea.jpg",
               BALL_SPRITE_FILEPATH[] = "ball.png";
constexpr glm::vec3 INIT_SCALE         = glm::vec3(1.0f, 1.0f, 0.0f),
                    INIT_POS_LPADDLE   = glm::vec3(-4.9f, 0.0f, 0.0f), // middle-left
                    INIT_POS_RPADDLE   = glm::vec3(4.9f, 0.0f, 0.0f),  // middle-right
                    INIT_POS_BALL      = glm::vec3(0.0f, 3.0f, 0.0f);  // Top middle
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
float g_time_accumulator = 0.0f;
bool g_auto_move = false;
int curr_direction = 0;     //1 to move up, 2 to move down

// Paddle and ball positions
glm::vec3 LPaddlePosition = glm::vec3(-4.9, 0, 0);
glm::vec3 RPaddlePosition = glm::vec3(4.9, 0, 0);
glm::vec3 ballPosition = glm::vec3(0, 3, 0);
// Paddle and ball movements
glm::vec3 LPaddleMovement = glm::vec3(0, 0, 0);
glm::vec3 RPaddleMovement = glm::vec3(0, 0, 0);
glm::vec3 ballMovement = glm::vec3(-1.0f, -0.5f, 0.0f);
// Paddle size
glm::vec3 paddleSize = glm::vec3(0.5f, 2.0f, 1.0f);
float paddleHeight = 1.0f * paddleSize.y;
float paddleWidth = 1.0f * paddleSize.x;
float paddleSpeed = 3.0f;
// Ball size
glm::vec3 ballSize = glm::vec3(0.25f, 0.25f, 1.0f);
float ballWidth = 1.0f * ballSize.x;
float ballHeight = 1.0f * ballSize.y;
float ballSpeed = 4.0f;
SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix,
          g_lpaddle_matrix,
          g_rpaddle_matrix,
          g_deep_sea_matrix,
          g_ball_matrix,
          g_projection_matrix;
float g_previous_ticks = 0.0f;
GLuint g_lpaddle_texture_id,
       g_rpaddle_texture_id,
       g_deep_sea_texture_id,
       g_ball_texture_id;
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_image_free(image);
    return textureID;
}
void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Pong Clone",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);
    }
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    g_lpaddle_matrix     = glm::mat4(1.0f);
    g_rpaddle_matrix     = glm::mat4(1.0f);
    g_deep_sea_matrix    = glm::mat4(1.0f);
    g_ball_matrix        = glm::mat4(1.0f);
    g_view_matrix        = glm::mat4(1.0f);
    g_projection_matrix  = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    glUseProgram(g_shader_program.get_program_id());
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
    g_lpaddle_texture_id = load_texture(PADDLE_SPRITE_FILEPATH);
    g_rpaddle_texture_id = load_texture(PADDLE_SPRITE_FILEPATH);
    g_deep_sea_texture_id = load_texture(DEEP_SEA_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    // Scale and position the background
    g_deep_sea_matrix = glm::scale(g_deep_sea_matrix, glm::vec3(10.0f, 7.5f, 1.0f));
    // Left Paddle
    g_lpaddle_matrix = glm::translate(g_lpaddle_matrix, INIT_POS_LPADDLE);
    g_lpaddle_matrix = glm::scale(g_lpaddle_matrix, paddleSize);
    // Right Paddle
    g_rpaddle_matrix = glm::translate(g_rpaddle_matrix, INIT_POS_RPADDLE);
    g_rpaddle_matrix = glm::scale(g_rpaddle_matrix, paddleSize);
    // Ball
    g_ball_matrix = glm::translate(g_ball_matrix, INIT_POS_BALL);
    g_ball_matrix = glm::scale(g_ball_matrix, ballSize);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
bool checkHitTop(glm::vec3 position, float heightDifference) {
    return (position.y + heightDifference >= 3.75f);
}
bool checkHitBottom(glm::vec3 position, float heightDifference) {
    return (position.y - heightDifference <= -3.75f);
}
bool collisionDetection(bool isLeftPaddle) {
    float ballX = ballPosition.x;
    float ballY = ballPosition.y;
    float paddleX, paddleY;
    if (isLeftPaddle) {
        paddleX = LPaddlePosition.x;
        paddleY = LPaddlePosition.y;
    } else {
        paddleX = RPaddlePosition.x;
        paddleY = RPaddlePosition.y;
    }
    float xdist = fabs(paddleX - ballX) - ((ballWidth + paddleWidth) / 2.0f);
    float ydist = fabs(paddleY - ballY) - ((ballHeight + paddleHeight) / 2.0f);
    if (xdist < 0 && ydist < 0) { // Collision detected
        float hitPosition = (ballY - paddleY) / (paddleHeight / 2.0f); // Normalized hit position (-1 to 1)
        ballMovement.y = hitPosition; // Adjust y direction based on hit position
        ballMovement.x *= -1.0f; // Reverse x direction
        ballMovement = glm::normalize(ballMovement); // Normalize to maintain consistent speed
        return true;
    }
    return false;
}
void process_input()
{
    LPaddleMovement = glm::vec3(0);
    RPaddleMovement = glm::vec3(0);
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_t) {
                g_auto_move = !g_auto_move; // Toggle automatic movement
                if (g_auto_move) {
                    // Set initial movement direction based on current position
                    if (LPaddlePosition.y >= 0) {
                        curr_direction = 2; // Start moving down
                    } else {
                        curr_direction = 1; // Start moving up
                    }
                }
            }
            break;
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    // If automatic movement is off, allow manual control
    if (!g_auto_move) {
        // Left Paddle Movement (W and S keys)
        if (keys[SDL_SCANCODE_W] && !checkHitTop(LPaddlePosition, paddleHeight / 2)) {
            LPaddleMovement.y = 1.0f;
        }
        else if (keys[SDL_SCANCODE_S] && !checkHitBottom(LPaddlePosition, paddleHeight / 2)) {
            LPaddleMovement.y = -1.0f;
        }
    }
    
    // Right Paddle Movement (Up and Down arrow keys)
    if (keys[SDL_SCANCODE_UP] && !checkHitTop(RPaddlePosition, paddleHeight / 2)) {
        RPaddleMovement.y = 1.0f;
    }
    else if (keys[SDL_SCANCODE_DOWN] && !checkHitBottom(RPaddlePosition, paddleHeight / 2)) {
        RPaddleMovement.y = -1.0f;
    }
}
void update()
{
    float ticks = (float)SDL_GetTicks() / 1000;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    // ————— FIXED TIMESTEP ————— //
    delta_time += g_time_accumulator;
    if (delta_time < FIXED_TIMESTEP)
    {
        g_time_accumulator = delta_time;
        return;
    }
    while (delta_time >= FIXED_TIMESTEP)
    {
        delta_time -= FIXED_TIMESTEP;
    }
    g_time_accumulator = delta_time;

    // Left Paddle Update
    if (g_auto_move) {
        // Reverse direction if paddle hits the top or bottom
        if (LPaddlePosition.y + paddleHeight / 2 >= 3.75f) {
            curr_direction = 2; // Move down
        } else if (LPaddlePosition.y - paddleHeight / 2 <= -3.75f) {
            curr_direction = 1; // Move up
        }

        // Apply movement based on current direction
        if (curr_direction == 1) {
            LPaddleMovement.y = 1.0f; // Move up
        } else if (curr_direction == 2) {
            LPaddleMovement.y = -1.0f; // Move down
        }

        // Apply movement
        LPaddlePosition += LPaddleMovement * paddleSpeed * FIXED_TIMESTEP;
    } else {
        // Manual control (if needed)
        LPaddlePosition += LPaddleMovement * paddleSpeed * FIXED_TIMESTEP;
    }

    // Boundary checks for left paddle
    if (LPaddlePosition.y + paddleHeight / 2 > 3.75f) {
        LPaddlePosition.y = 3.75f - paddleHeight / 2; // Stop at the top
    } else if (LPaddlePosition.y - paddleHeight / 2 < -3.75f) {
        LPaddlePosition.y = -3.75f + paddleHeight / 2; // Stop at the bottom
    }

    // Update the left paddle's model matrix
    g_lpaddle_matrix = glm::mat4(1.0f);
    g_lpaddle_matrix = glm::translate(g_lpaddle_matrix, LPaddlePosition);
    g_lpaddle_matrix = glm::scale(g_lpaddle_matrix, paddleSize);

    // Right Paddle Update
    RPaddlePosition += RPaddleMovement * paddleSpeed * FIXED_TIMESTEP;

    // Boundary checks for right paddle
    if (RPaddlePosition.y + paddleHeight / 2 > 3.75f) {
        RPaddlePosition.y = 3.75f - paddleHeight / 2; // Stop at the top
    } else if (RPaddlePosition.y - paddleHeight / 2 < -3.75f) {
        RPaddlePosition.y = -3.75f + paddleHeight / 2; // Stop at the bottom
    }

    g_rpaddle_matrix = glm::mat4(1.0f);
    g_rpaddle_matrix = glm::translate(g_rpaddle_matrix, RPaddlePosition);
    g_rpaddle_matrix = glm::scale(g_rpaddle_matrix, paddleSize);

    // Ball Update
    ballPosition += ballMovement * ballSpeed * FIXED_TIMESTEP;

    // Boundary checks for ball
    if (ballPosition.y + ballHeight / 2 > 3.75f) {
        ballPosition.y = 3.75f - ballHeight / 2; // Stop at the top
        ballMovement.y *= -1.0f; // Reverse y direction
    } else if (ballPosition.y - ballHeight / 2 < -3.75f) {
        ballPosition.y = -3.75f + ballHeight / 2; // Stop at the bottom
        ballMovement.y *= -1.0f; // Reverse y direction
    }

    if (collisionDetection(true) || collisionDetection(false)) {
   }

    g_ball_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::translate(g_ball_matrix, ballPosition);
    g_ball_matrix = glm::scale(g_ball_matrix, ballSize);

    // Game End Logic
    if (ballPosition.x >= 5.0f || ballPosition.x <= -5.0f) {
        g_app_status = TERMINATED;
    }
}
void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texture_coordinates[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    draw_object(g_deep_sea_matrix, g_deep_sea_texture_id);
    draw_object(g_lpaddle_matrix, g_lpaddle_texture_id);
    draw_object(g_rpaddle_matrix, g_rpaddle_texture_id);
    draw_object(g_ball_matrix, g_ball_texture_id);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    SDL_GL_SwapWindow(g_display_window);
}
void shutdown() { SDL_Quit(); }
int main(int argc, char* argv[])
{
    initialise();
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    shutdown();
    return 0;
}
