#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SDL_stdinc.h>

// --- Constants for Explosion Effect ---
const int EXPLOSION_FRAME_WIDTH = 192;
const int EXPLOSION_FRAME_HEIGHT = 192;
const int EXPLOSION_TOTAL_FRAMES = 20;
const Uint32 EXPLOSION_FRAME_DURATION = 75;

// Screen dimensions
const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

// Timing
const int ENEMY_SPAWN_INTERVAL = 1000;
const Uint32 PLAYER_AUTO_FIRE_COOLDOWN = 150;

// --- Game Parameters / Speeds ---
const float PLAYER_SPEED = 25000.0f;
const float PLAYER_BULLET_SPEED = 6000.0f;
const float ENEMY_BULLET_SPEED = 4000.0f;
const float INITIAL_ENEMY_SPEED = 1000.0f;
const float ENEMY_SPEED_INCREMENT = 100.0f;

// --- Kích thước địch theo loại ---
const int NORMAL_ENEMY_WIDTH = 60;
const int NORMAL_ENEMY_HEIGHT = 60;
const int NORMAL_BULLET_WIDTH = 10;
const int NORMAL_BULLET_HEIGHT = 20;

const int STRAIGHT_SHOOTER_ENEMY_WIDTH = 60;
const int STRAIGHT_SHOOTER_ENEMY_HEIGHT = 45;
const int STRAIGHT_SHOOTER_BULLET_WIDTH = 60;
const int STRAIGHT_SHOOTER_BULLET_HEIGHT = 30;

const int WEAVER_ENEMY_WIDTH = 75;
const int WEAVER_ENEMY_HEIGHT = 60;
const int WEAVER_BULLET_WIDTH = 34;
const int WEAVER_BULLET_HEIGHT = 25;

const int TANK_ENEMY_WIDTH = 75;
const int TANK_ENEMY_HEIGHT = 60;
const int TANK_BULLET_WIDTH = 34;
const int TANK_BULLET_HEIGHT = 25;

// --- Kích thước Player và Đạn theo Level ---
const int PLAYER_WIDTH_LVL1 = 60;
const int PLAYER_HEIGHT_LVL1 = 84;
const int PLAYER_BULLET_WIDTH_LVL1 = 15;
const int PLAYER_BULLET_HEIGHT_LVL1 = 30;

const int PLAYER_WIDTH_LVL2 = 60;
const int PLAYER_HEIGHT_LVL2 = 84;
const int PLAYER_BULLET_WIDTH_LVL2 = 35;
const int PLAYER_BULLET_HEIGHT_LVL2 = 35;

const int PLAYER_WIDTH_LVL3 = 125;
const int PLAYER_HEIGHT_LVL3 = 84;
const int PLAYER_BULLET_WIDTH_LVL3 = 54;
const int PLAYER_BULLET_HEIGHT_LVL3 = 40;

const int PLAYER_WIDTH_LVL4 = 168;
const int PLAYER_HEIGHT_LVL4 = 120;
const int PLAYER_BULLET_WIDTH_LVL4 = 80;
const int PLAYER_BULLET_HEIGHT_LVL4 = 40;

// --- Health & Damage Constants ---
const int PLAYER_INITIAL_HEALTH = 10;
const int NORMAL_INITIAL_HEALTH = 6;
const int STRAIGHT_SHOOTER_INITIAL_HEALTH = 4;
const int WEAVER_INITIAL_HEALTH = 8;
const int TANK_INITIAL_HEALTH = 14;

const int PLAYER_BULLET_DAMAGE = 2;
const int NORMAL_BULLET_DAMAGE = 1;
const int STRAIGHT_SHOOTER_BULLET_DAMAGE = 2;
const int WEAVER_BULLET_DAMAGE = 1;
const int TANK_BULLET_DAMAGE = 4;

const int NORMAL_COLLISION_DAMAGE = 2;
const int STRAIGHT_SHOOTER_COLLISION_DAMAGE = 4;
const int WEAVER_COLLISION_DAMAGE = 2;
const int TANK_COLLISION_DAMAGE = 8;

// --- Button Dimensions (Nên đặt ở đây nếu không phụ thuộc vào biến khác) ---
const int centeredButtonX = SCREEN_WIDTH / 2 - 280 / 2;
const int PAUSE_BUTTON_WIDTH = 80;
const int PAUSE_BUTTON_HEIGHT = 40;
const int PAUSE_MENU_BUTTON_WIDTH = 280;
const int PAUSE_MENU_BUTTON_HEIGHT = 70;
const int pauseMenuButtonX = SCREEN_WIDTH / 2 - PAUSE_MENU_BUTTON_WIDTH / 2;

#endif // CONSTANTS_H