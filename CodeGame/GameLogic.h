#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <SDL_stdinc.h>
void checkPlayerLevelUp();
void spawnEnemy();
void enemyShoot();
void updateGame(float deltaTime);
void updateExplosions(float deltaTime);

#endif