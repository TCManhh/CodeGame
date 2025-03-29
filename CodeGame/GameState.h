#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <vector>
#include "Player.h"   // Đảm bảo bạn có file Player.h
#include "Enemy.h"    // Đảm bảo bạn có file Enemy.h
#include "Bullet.h"   // Đảm bảo bạn có file Bullet.h
#include <SDL.h>

// Định nghĩa các biến toàn cục được sử dụng trong saveGameState()
extern int currentScore;
extern float currentEnemySpeed;
extern Uint32 lastEnemySpawnTime;
extern bool musicOn;
extern Player player;
extern std::vector<Enemy> enemies;
extern std::vector<Bullet> bullets;
extern std::vector<Bullet> enemyBullets;
extern bool canContinueGame;

#endif // GAME_STATE_H
