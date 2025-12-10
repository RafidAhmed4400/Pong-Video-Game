# **Pong**

A classic Pong-style game built in C++ using SDL2 and OpenGL/GLSL. This project features textured paddles and ball, real-time collision detection, and both manual and automatic paddle controls.


<br>
<br>

## **Features:**

   - Textured paddles, ball, and animated background

   - Real-time paddle and ball physics

   - Collision detection with dynamic ball angles

   - Keyboard controls for two players

   - Toggleable automatic movement for the left paddle

   - Fixed timestep game loop for consistent gameplay
 
<br>
<br>

## **Technologies Used:**

   - C++

   - SDL2

   - OpenGL

   - GLSL Shaders

   - GLM

   - stb_image

<br>
<br>

## **Controls:**

  -  **Left Paddle (Player 1):**
      - W – Move Up
      - S – Move Down

   - **Right Paddle (Player 2):**
      - Up Arrow – Move Up
      - Down Arrow – Move Down

<br>
<br>

## **Extra Controls:**
   - T – Toggle automatic movement for the left paddle

<br>
<br>

## **Game Rules:**

   - The ball bounces off the top and bottom walls.

   - The ball changes direction based on where it hits the paddle.

   - The game ends when the ball goes past the left or right edge of the screen.

<br>
<br>

## **Project Structure:**

   - /project-root
   - main.cpp
   - ShaderProgram.h / ShaderProgram.cpp
   - shaders/
   - vertex_textured.glsl
   - fragment_textured.glsl
   - assets/
   - paddle.png
   - ball.png
   - deepSea.jpg


<br>
<br>

## **How to Run:**

   - Prerequisites:
      - Install SDL2, OpenGL, and GLM.

### Build (example using g++):
   - g++ main.cpp ShaderProgram.cpp -lSDL2 -lOpenGL -ldl -o pong

### **Run:**
   - ./pong
