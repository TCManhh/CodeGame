#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include "Structs.h"

// --- Khai báo extern cho các biến toàn cục ---

// Textures
extern SDL_Texture* playerTextureLvl1;
extern SDL_Texture* playerTextureLvl2;
extern SDL_Texture* playerTextureLvl3;
extern SDL_Texture* playerTextureLvl4;
extern SDL_Texture* bulletTextureLvl1;
extern SDL_Texture* bulletTextureLvl2;
extern SDL_Texture* bulletTextureLvl3;
extern SDL_Texture* bulletTextureLvl4;
extern SDL_Texture* enemyTexture;
extern SDL_Texture* enemyTextureStraight;
extern SDL_Texture* enemyTextureWeave;
extern SDL_Texture* enemyTextureTank;
extern SDL_Texture* normalBulletTexture;
extern SDL_Texture* straightShooterBulletTexture;
extern SDL_Texture* tankBulletTexture;
extern SDL_Texture* weaverBulletTexture;
extern SDL_Texture* backgroundTexture;
extern SDL_Texture* gameBackgroundTexture;
extern SDL_Texture* gameOverBackgroundTexture;
extern SDL_Texture* explosionTexture;

// Game Objects & Containers
extern std::vector<Entity> enemies;
extern std::vector<Entity> bullets;
extern std::vector<Entity> enemyBullets;
extern std::vector<ExplosionEffect> activeExplosions;
extern Entity player;

// Audio
extern bool musicOn;
extern Mix_Music* bgMusic;
extern Mix_Music* gameMusic;
extern Mix_Chunk* startSound;
extern Mix_Chunk* explosionSound;

// SDL Core & Font
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern TTF_Font* font;

// Game State & Scores
extern GameState state;
extern int currentScore;
extern int highScore;

// Timing & Speed
extern Uint32 lastEnemySpawnTime;
extern float currentEnemySpeed;

// Input & Control State
extern bool running;
extern bool isPlayerDragging;
extern bool isMovingToTarget;
extern float targetMoveX;
extern float targetMoveY;
extern bool isFollowingMouse;

// Buttons (Khai báo mảng và số lượng)

extern Button buttons[];
extern const int BUTTON_COUNT; // Khai báo const int
extern Button gameOverButtons[];
extern const int GAMEOVER_BUTTON_COUNT;
extern Button pauseButton; // Nút đơn lẻ không cần COUNT
extern Button pauseMenuButtons[];
extern const int PAUSE_MENU_BUTTON_COUNT;

#endif // GLOBALS_H