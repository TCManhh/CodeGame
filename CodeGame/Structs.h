#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL.h>
#include <string>
#include <vector>

// --- Enum for Game State ---
enum GameState {
    MENU,
    GAME,
    PAUSED,
    GAME_OVER
};

// --- Enum for Enemy Type ---
enum EnemyType {
    NORMAL,
    STRAIGHT_SHOOTER,
    WEAVER,
    TANK
};

// --- Struct for Explosion Effect ---
struct ExplosionEffect {
    SDL_Rect position;
    SDL_Texture* texture;
    int frameWidth;
    int frameHeight;
    int totalFrames;
    Uint32 frameDuration;

    int currentFrame;
    Uint32 lastFrameTime;
    bool finished;
};

// --- Struct for Buttons ---
struct Button {
    SDL_Rect rect;
    std::string text;
    bool hovered;
};

// --- Struct for Game Entities (Player, Enemies, Bullets) ---
struct Entity {
    SDL_Rect rect{};
    SDL_Texture* texture = nullptr;
    float xPos = 0.0f;
    float yPos = 0.0f;
    float speedX = 0.0f;
    float speedY = 0.0f;
    int enemyStopY = 0;
    Uint32 lastShotTime = 0;
    Uint32 fireCooldown = 1500;
    int level = 1;

    // --- Các trường cập nhật/mới ---
    EnemyType type = NORMAL;
    int health = 1;
    int maxHealth = 1;
    int bulletDamage = 1;
    int collisionDamage = 1;
    int damage = 0;

    // Weaver specific
    float weaveAmplitude = 0.0f;
    float weaveFrequency = 0.0f;
    float initialXPos = 0.0f;
};


#endif