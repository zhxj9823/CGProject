# CGProject

- 张行健 3170105373 计算机科学与技术
- 沈霁 3170102491 计算机科学与技术
- 陈欢 3170102503 计算机科学与技术

## 1 Description



## 1.2 Library used

- GLFW
- GLAD
- stb_image
- irrKlang
- freetype

## 2 Design Specification

### 2.1 Game Class

First, we define a game class that contains all relevant render and gameplay code. The idea of such a game class is that it (sort of) organizes your game code while also decoupling all windowing code from the game. 

The game class hosts an initialization function, an update function, a function to process input and a render function. 

**Some common global variables to be added into the game class.**

### 2.2 Utility

Since we're creating a large application we'll frequently have to re-use several OpenGL concepts, like textures and shaders. It thus makes sense to create a more easy-to-use interface for these two items as similarly done in one of the earlier tutorials where we created a shader class.

A shader class is defined that generates a compiled shader (or generates error messages if it fails) from two or three strings (if a geometry shader is present). The shader class also contains a lot of useful utility functions to quickly set uniform values. A texture class is also defined that generates a 2D texture image (based on its properties) from a byte array and a given width and height. Again, the texture class also hosts utility functions.

### 2.3 Resource management

For this reason it is often considered a more organized approach to create a single entity designed for loading game-related resources called a resource manager. We chose to use a singleton static resource manager that is (due to its static nature) always available throughout the project hosting all loaded resources and its relevant loading functionality.

Using the resource manager we can easily load shaders into the program like:

```C++
Shader shader = ResourceManager::LoadShader("vertex.vs", "fragment.vs", nullptr, "test");
// then use it
shader.Use();
// or
ResourceManager::GetShader("test").Use();
```

**Seems that not much changes can be made to the two part above**

### 2.4 Audio

We are, however, going to make use of library for audio management called **irrKlang**.

IrrKlang is a high level 2D and 3D cross platform (Windows, Mac OS X, Linux) sound engine and audio library that plays WAV, MP3, OGG, and FLAC files to name a few. It also features several audio effects like reverb, delay and distortion that can be extensively tweaked.

Adding this to the Breakout game is extremely easy with the irrKlang library. We include the corresponding header files, create an `irrKlang::ISoundEngine`, initialize it with createIrrKlangDevice and then use the engine to load and play audio files:

```C++
#include <irrklang/irrKlang.h>

irrklang::ISoundEngine *SoundEngine = createIrrKlangDevice();
  
void Game::Init()
{
    [...]
    irrklang::SoundEngine->play2D("audio/breakout.mp3", GL_TRUE);
}
```

### 2.5 Text Render

All text rendering code is encapsulated within a class called `TextRenderer` that features the initialization of the *FreeType* library, render configuration and the actual render code.

Adding the TextRenderer to the game is easy:

```C++
TextRenderer  *Text;
  
void Game::Init()
{
    [...]
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("fonts/ocraext.TTF", 24);
}
```

Display the text may be like this:

```C++
Text->RenderText("Press ENTER to start", 250.0f, Height / 2, 1.0f);
```



## 3 Project Log

### Dec 31 2018

- Create the solution

- Add some basic part of program to the file

  - game
  - shader
  - texture
  - game_object
  - camera

