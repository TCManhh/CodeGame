#include "GameLogic.h"

#include "Globals.h"
#include "Structs.h"
#include "Constants.h"
#include "Utils.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// --- Function Definitions (Phần triển khai hàm) ---

void checkPlayerLevelUp() {
    if (player.level >= 4) return; // Đã đạt cấp tối đa

    // Tính điểm cần dựa trên level HIỆN TẠI trước khi tăng
    int scoreRequiredForNextLevel = player.level * 250;

    if (currentScore >= scoreRequiredForNextLevel) {
        player.level++;
        std::cout << "Player Leveled Up to Level: " << player.level << std::endl;

        // Lưu vị trí tâm cũ
        float oldCenterX = player.xPos + player.rect.w / 2.0f;
        float oldCenterY = player.yPos + player.rect.h / 2.0f;

        // Cập nhật texture VÀ KÍCH THƯỚC người chơi
        switch (player.level) {
        case 2:
            player.texture = playerTextureLvl2;
            player.rect.w = PLAYER_WIDTH_LVL2;
            player.rect.h = PLAYER_HEIGHT_LVL2;
            // Optional: player.bulletDamage += 1;
            break;
        case 3:
            player.texture = playerTextureLvl3;
            player.rect.w = PLAYER_WIDTH_LVL3;
            player.rect.h = PLAYER_HEIGHT_LVL3;
            break;
        case 4:
            player.texture = playerTextureLvl4;
            player.rect.w = PLAYER_WIDTH_LVL4;
            player.rect.h = PLAYER_HEIGHT_LVL4;
            break;
        }

        // Cập nhật lại xPos/yPos để giữ nguyên tâm
        player.xPos = oldCenterX - player.rect.w / 2.0f;
        player.yPos = oldCenterY - player.rect.h / 2.0f;

        // Đảm bảo không ra khỏi màn hình sau khi đổi kích thước/vị trí
        player.xPos = std::max(0.0f, std::min((float)SCREEN_WIDTH - player.rect.w, player.xPos));
        player.yPos = std::max(0.0f, std::min((float)SCREEN_HEIGHT - player.rect.h, player.yPos));

        // Cập nhật rect x,y từ xPos, yPos mới
        player.rect.x = (int)player.xPos;
        player.rect.y = (int)player.yPos;
    }
}

void spawnEnemy() {
    Entity newEnemy;

    int typeRoll = rand() % 100;
    EnemyType chosenType;
    if (typeRoll < 45) chosenType = NORMAL;
    else if (typeRoll < 65) chosenType = STRAIGHT_SHOOTER;
    else if (typeRoll < 85) chosenType = WEAVER;
    else chosenType = TANK;

    newEnemy.type = chosenType;

    int currentEnemyWidth = 0;
    int currentEnemyHeight = 0;

    // Xác định kích thước và texture dựa trên loại
    switch (chosenType) {
    case NORMAL:
        currentEnemyWidth = NORMAL_ENEMY_WIDTH;
        currentEnemyHeight = NORMAL_ENEMY_HEIGHT;
        newEnemy.texture = enemyTexture;
        break;
    case STRAIGHT_SHOOTER:
        currentEnemyWidth = STRAIGHT_SHOOTER_ENEMY_WIDTH;
        currentEnemyHeight = STRAIGHT_SHOOTER_ENEMY_HEIGHT;
        newEnemy.texture = enemyTextureStraight;
        break;
    case WEAVER:
        currentEnemyWidth = WEAVER_ENEMY_WIDTH;
        currentEnemyHeight = WEAVER_ENEMY_HEIGHT;
        newEnemy.texture = enemyTextureWeave;
        break;
    case TANK:
        currentEnemyWidth = TANK_ENEMY_WIDTH;
        currentEnemyHeight = TANK_ENEMY_HEIGHT;
        newEnemy.texture = enemyTextureTank;
        break;
    }

    // Tính vị trí spawn
    int spawnX = rand() % (SCREEN_WIDTH - currentEnemyWidth);

    // Thiết lập các thuộc tính ban đầu
    newEnemy.rect = { spawnX, -currentEnemyHeight, currentEnemyWidth, currentEnemyHeight };
    newEnemy.xPos = (float)spawnX;
    newEnemy.yPos = (float)newEnemy.rect.y;
    newEnemy.speedX = 0;
    newEnemy.lastShotTime = SDL_GetTicks() + (rand() % 1000); // Randomize initial shot slightly

    // Thiết lập các thuộc tính đặc trưng cho từng loại
    switch (chosenType) {
    case NORMAL:
        newEnemy.health = NORMAL_INITIAL_HEALTH;
        newEnemy.maxHealth = NORMAL_INITIAL_HEALTH;
        newEnemy.bulletDamage = NORMAL_BULLET_DAMAGE;
        newEnemy.collisionDamage = NORMAL_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (1.0f + (rand() % 3) / 10.0f);
        newEnemy.enemyStopY = rand() % 200 + 150;
        newEnemy.fireCooldown = static_cast<Uint32>(1000 + (rand() % 1500));
        break;
    case STRAIGHT_SHOOTER:
        newEnemy.health = STRAIGHT_SHOOTER_INITIAL_HEALTH;
        newEnemy.maxHealth = STRAIGHT_SHOOTER_INITIAL_HEALTH;
        newEnemy.bulletDamage = STRAIGHT_SHOOTER_BULLET_DAMAGE;
        newEnemy.collisionDamage = STRAIGHT_SHOOTER_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (1.2f + (rand() % 3) / 10.0f);
        newEnemy.enemyStopY = -1; // Doesn't stop based on Y
        newEnemy.fireCooldown = static_cast<Uint32>(1200 + (rand() % 1000));
        break;
    case WEAVER:
        newEnemy.health = WEAVER_INITIAL_HEALTH;
        newEnemy.maxHealth = WEAVER_INITIAL_HEALTH;
        newEnemy.bulletDamage = WEAVER_BULLET_DAMAGE;
        newEnemy.collisionDamage = WEAVER_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (0.9f + (rand() % 2) / 10.0f);
        newEnemy.enemyStopY = -1; // Doesn't stop based on Y
        newEnemy.fireCooldown = static_cast<Uint32>(500 + (rand() % 401) - 200);
        newEnemy.initialXPos = newEnemy.xPos;
        newEnemy.weaveAmplitude = 150.0f + (rand() % 100);
        newEnemy.weaveFrequency = 0.004f + (float)(rand() % 5) / 1000.0f;
        break;
    case TANK:
        newEnemy.health = TANK_INITIAL_HEALTH;
        newEnemy.maxHealth = TANK_INITIAL_HEALTH;
        newEnemy.bulletDamage = TANK_BULLET_DAMAGE;
        newEnemy.collisionDamage = TANK_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (0.7f + (rand() % 2) / 10.0f);
        newEnemy.enemyStopY = rand() % 150 + 100;
        newEnemy.fireCooldown = static_cast<Uint32>(1800 + (rand() % 2000));
        break;
    }

    enemies.push_back(newEnemy); // Thêm địch vào danh sách
}


void enemyShoot() {
    Uint32 currentTime = SDL_GetTicks();
    for (auto& enemy : enemies) { // Dùng tham chiếu để sửa đổi lastShotTime
        bool canShoot = false;

        // Xác định xem địch có thể bắn không
        switch (enemy.type) {
        case NORMAL: case TANK:
            canShoot = (enemy.enemyStopY != -1 && enemy.yPos >= enemy.enemyStopY && currentTime > enemy.lastShotTime + enemy.fireCooldown);
            break;
        case STRAIGHT_SHOOTER:
        case WEAVER:
            canShoot = (currentTime > enemy.lastShotTime + enemy.fireCooldown);
            break;
        }

        if (canShoot) {
            float startX = enemy.xPos + enemy.rect.w / 2.0f;
            float startY = enemy.yPos + enemy.rect.h; // Bắn từ dưới

            float bulletSpeedX = 0.0f;
            float bulletSpeedY = 0.0f;
            float bulletSpeedMagnitude = ENEMY_BULLET_SPEED;

            // Xác định hướng đạn
            if (enemy.type == STRAIGHT_SHOOTER || enemy.type == WEAVER) { // Bắn thẳng xuống
                bulletSpeedX = 0.0f;
                bulletSpeedY = bulletSpeedMagnitude;
            }
            else { // Bắn về phía player
                float targetX = player.xPos + player.rect.w / 2.0f;
                float targetY = player.yPos + player.rect.h / 2.0f;
                float dirX = targetX - startX;
                float dirY = targetY - startY;
                float length = std::sqrt(dirX * dirX + dirY * dirY);

                if (length > 0.0001f) { // Tránh chia cho 0
                    float invLength = 1.0f / length;
                    bulletSpeedX = dirX * invLength * bulletSpeedMagnitude;
                    bulletSpeedY = dirY * invLength * bulletSpeedMagnitude;
                }
                else {
                    bulletSpeedX = 0.0f; // Bắn thẳng xuống nếu quá gần
                    bulletSpeedY = bulletSpeedMagnitude;
                }
            }

            Entity enemyBullet;
            int bulletW = 0;
            int bulletH = 0;

            // Xác định texture và kích thước đạn
            switch (enemy.type) {
            case NORMAL:
                bulletW = NORMAL_BULLET_WIDTH; bulletH = NORMAL_BULLET_HEIGHT;
                enemyBullet.texture = normalBulletTexture; break;
            case STRAIGHT_SHOOTER:
                bulletW = STRAIGHT_SHOOTER_BULLET_WIDTH; bulletH = STRAIGHT_SHOOTER_BULLET_HEIGHT;
                enemyBullet.texture = straightShooterBulletTexture; break;
            case TANK:
                bulletW = TANK_BULLET_WIDTH; bulletH = TANK_BULLET_HEIGHT;
                enemyBullet.texture = tankBulletTexture; break;
            case WEAVER:
                bulletW = WEAVER_BULLET_WIDTH; bulletH = WEAVER_BULLET_HEIGHT;
                enemyBullet.texture = weaverBulletTexture; break;
            default:
                bulletW = NORMAL_BULLET_WIDTH; bulletH = NORMAL_BULLET_HEIGHT;
                enemyBullet.texture = normalBulletTexture; break;
            }

            // Kiểm tra texture trước khi tạo đạn
            if (!enemyBullet.texture) {
                std::cerr << "Warning: Enemy bullet texture is null for type " << static_cast<int>(enemy.type) << " in enemyShoot! Cannot create bullet." << std::endl;
                enemy.lastShotTime = currentTime; // Vẫn reset cooldown
                continue; // Bỏ qua địch này
            }

            // Tạo đạn
            enemyBullet.rect = { (int)(startX - bulletW / 2.0f), (int)startY, bulletW, bulletH };
            enemyBullet.xPos = (float)enemyBullet.rect.x;
            enemyBullet.yPos = (float)startY;
            enemyBullet.speedX = bulletSpeedX;
            enemyBullet.speedY = bulletSpeedY;
            enemyBullet.damage = enemy.bulletDamage; // Damage đạn theo damage của địch
            enemyBullet.type = enemy.type; // Gán type (tùy chọn)

            enemyBullets.push_back(enemyBullet); // Thêm đạn vào danh sách
            enemy.lastShotTime = currentTime; // Reset cooldown
        }
    }
}


void updateGame(float deltaTime) {
    // --- 1. Cập nhật & Xóa đạn người chơi ---
    for (int i = (int)bullets.size() - 1; i >= 0; --i) {
        bullets[i].yPos -= PLAYER_BULLET_SPEED * deltaTime;
        bullets[i].rect.y = (int)bullets[i].yPos;

        if (bullets[i].rect.y + bullets[i].rect.h < 0) { // Ra khỏi màn hình (trên)
            bullets.erase(bullets.begin() + i);
        }
    }

    // --- 2. Cập nhật vị trí địch & Xóa nếu ra khỏi màn hình ---
    for (int i = (int)enemies.size() - 1; i >= 0; --i) {
        if (i >= (int)enemies.size()) continue; // Kiểm tra sau khi xóa
        Entity& enemy = enemies[i];

        bool stopped = (enemy.enemyStopY != -1 && enemy.yPos >= enemy.enemyStopY);
        if (!stopped) {
            enemy.yPos += enemy.speedY * deltaTime;
            if (enemy.enemyStopY != -1 && enemy.yPos > enemy.enemyStopY) {
                enemy.yPos = (float)enemy.enemyStopY; // Snap vào vị trí dừng
            }
        }

        // Cập nhật vị trí X (cho Weaver hoặc loại khác nếu có speedX)
        if (enemy.type == WEAVER) {
            enemy.xPos = enemy.initialXPos + enemy.weaveAmplitude * sin(enemy.yPos * enemy.weaveFrequency);
            enemy.xPos = std::max(0.0f, std::min((float)SCREEN_WIDTH - enemy.rect.w, enemy.xPos));
        }
        else {
            enemy.xPos += enemy.speedX * deltaTime;
            enemy.xPos = std::max(0.0f, std::min((float)SCREEN_WIDTH - enemy.rect.w, enemy.xPos));
        }

        enemy.rect.x = (int)enemy.xPos;
        enemy.rect.y = (int)enemy.yPos;

        if (enemy.rect.y > SCREEN_HEIGHT) {
            enemies.erase(enemies.begin() + i);
        }
    }

    // --- 3. Va chạm: Đạn Người Chơi <-> Kẻ Địch ---
    for (int i = (int)bullets.size() - 1; i >= 0; --i) {
        if (i >= (int)bullets.size()) continue;

        for (int j = (int)enemies.size() - 1; j >= 0; --j) {
            if (j >= (int)enemies.size()) continue;

            if (SDL_HasIntersection(&bullets[i].rect, &enemies[j].rect)) {
                enemies[j].health -= player.bulletDamage;
                bullets.erase(bullets.begin() + i);

                if (enemies[j].health <= 0) { // Nếu địch hết máu
                    // --- Kích hoạt nổ ---
                    ExplosionEffect newExplosion;
                    newExplosion.texture = explosionTexture;
                    newExplosion.frameWidth = EXPLOSION_FRAME_WIDTH;
                    newExplosion.frameHeight = EXPLOSION_FRAME_HEIGHT;
                    newExplosion.totalFrames = EXPLOSION_TOTAL_FRAMES;
                    newExplosion.frameDuration = EXPLOSION_FRAME_DURATION;
                    newExplosion.currentFrame = 0;
                    newExplosion.lastFrameTime = SDL_GetTicks();
                    newExplosion.finished = false;
                    float centerX = enemies[j].xPos + enemies[j].rect.w / 2.0f;
                    float centerY = enemies[j].yPos + enemies[j].rect.h / 2.0f;
                    newExplosion.position = {
                        (int)(centerX - newExplosion.frameWidth / 2.0f),
                        (int)(centerY - newExplosion.frameHeight / 2.0f),
                        newExplosion.frameWidth, newExplosion.frameHeight };
                    activeExplosions.push_back(newExplosion);
                    // --- Kết thúc nổ ---

                    // Cộng điểm
                    int scoreBonus = 10;
                    if (enemies[j].type == TANK) scoreBonus = 30;
                    else if (enemies[j].type == WEAVER) scoreBonus = 15;
                    currentScore += scoreBonus;

                    checkPlayerLevelUp(); // Kiểm tra lên cấp

                    if (musicOn && explosionSound) { // Phát âm thanh nổ
                        Mix_PlayChannel(-1, explosionSound, 0);
                    }

                    enemies.erase(enemies.begin() + j);
                }
                goto next_bullet;
            }
        }
    next_bullet:;
    }

    // --- 4. Va chạm: Kẻ Địch <-> Người Chơi ---
    for (int i = (int)enemies.size() - 1; i >= 0; --i) {
        if (i >= (int)enemies.size()) continue;

        if (SDL_HasIntersection(&enemies[i].rect, &player.rect)) {
            player.health -= enemies[i].collisionDamage;

            // --- Kích hoạt nổ cho địch ---
            ExplosionEffect newExplosion;
            // Gán các thuộc tính cho hiệu ứng nổ từ hằng số và trạng thái ban đầu
            newExplosion.texture = explosionTexture;
            newExplosion.frameWidth = EXPLOSION_FRAME_WIDTH;
            newExplosion.frameHeight = EXPLOSION_FRAME_HEIGHT;
            newExplosion.totalFrames = EXPLOSION_TOTAL_FRAMES;
            newExplosion.frameDuration = EXPLOSION_FRAME_DURATION;
            newExplosion.currentFrame = 0;
            newExplosion.lastFrameTime = SDL_GetTicks();
            newExplosion.finished = false;

            // Tính toán vị trí tâm của kẻ địch vừa va chạm (dùng index i)
            float centerX = enemies[i].xPos + enemies[i].rect.w / 2.0f;
            float centerY = enemies[i].yPos + enemies[i].rect.h / 2.0f;

            // Đặt vị trí vẽ vụ nổ (căn giữa tại tâm địch)
            newExplosion.position = {
                (int)(centerX - newExplosion.frameWidth / 2.0f),
                (int)(centerY - newExplosion.frameHeight / 2.0f),
                newExplosion.frameWidth,
                newExplosion.frameHeight
            };

            // Thêm vụ nổ vào danh sách các vụ nổ đang hoạt động
            activeExplosions.push_back(newExplosion);
            // --- Kết thúc nổ ---

            if (musicOn && explosionSound) { // Phát âm thanh
                Mix_PlayChannel(-1, explosionSound, 0);
            }

            enemies.erase(enemies.begin() + i);

            if (player.health <= 0) { 
                std::cout << "Player destroyed by collision! Game Over! Final Score: " << currentScore << std::endl;
                if (currentScore > highScore) {
                    highScore = currentScore;
                    saveHighScore();
                }
                isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                state = GAME_OVER;
                return;
            }
        }
    }

    // --- 5. Cập nhật, Va chạm & Xóa đạn địch ---
    for (int i = (int)enemyBullets.size() - 1; i >= 0; --i) {
        if (i >= (int)enemyBullets.size()) continue;

        // Cập nhật vị trí đạn địch
        enemyBullets[i].xPos += enemyBullets[i].speedX * deltaTime;
        enemyBullets[i].yPos += enemyBullets[i].speedY * deltaTime;
        enemyBullets[i].rect.x = (int)enemyBullets[i].xPos;
        enemyBullets[i].rect.y = (int)enemyBullets[i].yPos;

        // Kiểm tra va chạm với player
        if (SDL_HasIntersection(&enemyBullets[i].rect, &player.rect)) {
            player.health -= enemyBullets[i].damage;
            enemyBullets.erase(enemyBullets.begin() + i);

            if (player.health <= 0) { // Kiểm tra nếu player hết máu
                std::cout << "Player destroyed by bullet! Game Over! Final Score: " << currentScore << std::endl;
                if (currentScore > highScore) {
                    highScore = currentScore;
                    saveHighScore();
                }
                isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                state = GAME_OVER;
                return;
            }
        }
        // Xóa đạn địch nếu ra khỏi màn hình
        else if (enemyBullets[i].rect.x + enemyBullets[i].rect.w < 0 ||
            enemyBullets[i].rect.x > SCREEN_WIDTH ||
            enemyBullets[i].rect.y + enemyBullets[i].rect.h < 0 ||
            enemyBullets[i].rect.y > SCREEN_HEIGHT)
        {
            enemyBullets.erase(enemyBullets.begin() + i);
        }
    }
}

void updateExplosions(float deltaTime) {
    Uint32 currentTime = SDL_GetTicks();

    for (int i = (int)activeExplosions.size() - 1; i >= 0; --i) {
        ExplosionEffect& explosion = activeExplosions[i];

        if (explosion.finished) {
            continue;
        }

        if (currentTime > explosion.lastFrameTime + explosion.frameDuration) {
            explosion.currentFrame++;
            explosion.lastFrameTime = currentTime;

            if (explosion.currentFrame >= explosion.totalFrames) {
                explosion.finished = true;
            }
        }
    }

    // Xóa các vụ nổ đã hoàn thành
    activeExplosions.erase(
        std::remove_if(
            activeExplosions.begin(),
            activeExplosions.end(),
            [](const ExplosionEffect& e) { return e.finished; }
        ),
        activeExplosions.end()
    );
}