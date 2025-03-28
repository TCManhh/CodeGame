#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <string>     // Cho std::string và std::to_string
#include <fstream>    // Cho việc đọc/ghi file (lưu điểm)
#include <sstream>    // Có thể dùng để định dạng chuỗi (tùy chọn)
#include <limits>     // Dùng khi đọc file điểm

// Screen dimensions
const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

// Timing
const int ENEMY_SPAWN_INTERVAL = 1000;
const Uint32 PLAYER_AUTO_FIRE_COOLDOWN = 150;

// --- Game Parameters / Speeds (Values adjusted for deltaTime) ---
const float PLAYER_SPEED = 25000.0f; // <<< Tốc độ di chuyển của người chơi
const float PLAYER_BULLET_SPEED = 6000.0f;
const float ENEMY_BULLET_SPEED = 4000.0f;
const float INITIAL_ENEMY_SPEED = 1000.0f;
const float ENEMY_SPEED_INCREMENT = 100.0f;

// --- Kích thước gốc ---
const int PLAYER_WIDTH = 50;
const int PLAYER_HEIGHT = 50;
const int ENEMY_WIDTH = 50;
const int ENEMY_HEIGHT = 50;
const int PLAYER_BULLET_WIDTH = 10;
const int PLAYER_BULLET_HEIGHT = 20;
const int ENEMY_BULLET_WIDTH = 10;
const int ENEMY_BULLET_HEIGHT = 20;


// --- THÊM MỚI: Health & Damage Constants ---
const int PLAYER_INITIAL_HEALTH = 10;
const int NORMAL_INITIAL_HEALTH = 6;
const int STRAIGHT_SHOOTER_INITIAL_HEALTH = 4;
const int WEAVER_INITIAL_HEALTH = 8;
const int TANK_INITIAL_HEALTH = 14;

const int PLAYER_BULLET_DAMAGE = 2;
const int NORMAL_BULLET_DAMAGE = 1;
const int STRAIGHT_SHOOTER_BULLET_DAMAGE = 2;
const int WEAVER_BULLET_DAMAGE = 1; // Sát thương đạn (nếu Weaver bắn sau này)
const int TANK_BULLET_DAMAGE = 4;

const int NORMAL_COLLISION_DAMAGE = 2;           // Sát thương va chạm địch -> người chơi
const int STRAIGHT_SHOOTER_COLLISION_DAMAGE = 4;
const int WEAVER_COLLISION_DAMAGE = 2;
const int TANK_COLLISION_DAMAGE = 8;
// ------------------------------------------

Uint32 lastEnemySpawnTime = 0;
float currentEnemySpeed = INITIAL_ENEMY_SPEED;

// Gần đầu file
enum GameState { MENU, GAME, PAUSED, GAME_OVER };
GameState state = MENU;

// --- THÊM MỚI: Định nghĩa loại địch ---
enum EnemyType {
    NORMAL,           // Loại cũ
    STRAIGHT_SHOOTER, // Bắn thẳng, đi thẳng
    WEAVER,           // Bay lượn sóng
    TANK              // Trâu bò, chịu nhiều đòn
};

// --- Structs ---
struct Button {
    SDL_Rect rect;
    std::string text;
    bool hovered;
};

// --- SỬA ĐỔI: Struct Entity ---
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

    // --- Các trường cập nhật/mới ---
    EnemyType type = NORMAL;
    int health = 1;           // Máu hiện tại
    int maxHealth = 1;        // Máu tối đa <<< THÊM
    int bulletDamage = 1;     // Sát thương đạn của entity này <<< THÊM
    int collisionDamage = 1;  // Sát thương va chạm của entity này <<< THÊM
    int damage = 0;           // Sát thương CỦA ĐẠN (nếu là đạn) <<< THÊM/SỬA ĐỔI MỤC ĐÍCH

    // Weaver specific
    float weaveAmplitude = 0.0f;
    float weaveFrequency = 0.0f;
    float initialXPos = 0.0f;
};

// --- Nút Menu (Đã căn giữa X) ---
const int centeredButtonX = SCREEN_WIDTH / 2 - 280 / 2; // = 400
Button buttons[] = {
    {{centeredButtonX, 200, 280, 70}, "Start New Game", false},
    {{centeredButtonX, 290, 280, 70}, "Continue Game", false}, // <<< Chức năng chưa được thêm vào phiên bản này
    {{centeredButtonX, 380, 280, 70}, "Music: On", false},
    {{centeredButtonX, 470, 280, 70}, "Exit Game", false}
};
const int BUTTON_COUNT = sizeof(buttons) / sizeof(Button);

// --- Nút Game Over (Đã căn giữa X) ---
Button gameOverButtons[] = {
    {{centeredButtonX, 350, 280, 70}, "Play Again", false},
    {{centeredButtonX, 440, 280, 70}, "Main Menu", false}
};
const int GAMEOVER_BUTTON_COUNT = sizeof(gameOverButtons) / sizeof(Button);

// --- Nút Pause (Thêm mới) ---
const int PAUSE_BUTTON_WIDTH = 80;
const int PAUSE_BUTTON_HEIGHT = 40;
Button pauseButton = {
    {SCREEN_WIDTH - PAUSE_BUTTON_WIDTH - 10, 10, PAUSE_BUTTON_WIDTH, PAUSE_BUTTON_HEIGHT},
    "||", // Luôn là "||"
    false
};

// --- THÊM MỚI: Nút Pause Menu ---
const int PAUSE_MENU_BUTTON_WIDTH = 280;
const int PAUSE_MENU_BUTTON_HEIGHT = 70;
const int pauseMenuButtonX = SCREEN_WIDTH / 2 - PAUSE_MENU_BUTTON_WIDTH / 2; // Căn giữa X
Button pauseMenuButtons[] = {
    {{pauseMenuButtonX, 250, PAUSE_MENU_BUTTON_WIDTH, PAUSE_MENU_BUTTON_HEIGHT}, "Continue", false},
    {{pauseMenuButtonX, 340, PAUSE_MENU_BUTTON_WIDTH, PAUSE_MENU_BUTTON_HEIGHT}, "Mute Sound", false}, // Text sẽ được cập nhật khi vào Pause
    {{pauseMenuButtonX, 430, PAUSE_MENU_BUTTON_WIDTH, PAUSE_MENU_BUTTON_HEIGHT}, "Save & Exit to Menu", false} // <<< Chức năng chưa được thêm vào phiên bản này
};
const int PAUSE_MENU_BUTTON_COUNT = sizeof(pauseMenuButtons) / sizeof(Button);

// --- Global Variables ---
SDL_Texture* playerTexture = nullptr;
SDL_Texture* enemyTexture = nullptr;
SDL_Texture* bulletTexture = nullptr; // <<< Sửa: Chỉ cần một texture đạn
SDL_Texture* enemyBulletTexture = nullptr; // <<< Thêm: Texture đạn địch (nếu muốn khác)
// --- THÊM MỚI: Textures cho địch mới ---
SDL_Texture* enemyTextureStraight = nullptr;
SDL_Texture* enemyTextureWeave = nullptr;
SDL_Texture* enemyTextureTank = nullptr;
// ------------------------------------
std::vector<Entity> enemies;
std::vector<Entity> bullets;
std::vector<Entity> enemyBullets;
Entity player;

bool musicOn = true;
Mix_Music* bgMusic = nullptr;
Mix_Music* gameMusic = nullptr;
Mix_Chunk* startSound = nullptr;
Mix_Chunk* explosionSound = nullptr;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* gameBackgroundTexture = nullptr;
SDL_Texture* gameOverBackgroundTexture = nullptr;
TTF_Font* font = nullptr;

int currentScore = 0;
int highScore = 0;
bool running = true;
bool isPlayerDragging = false;
bool isMovingToTarget = false;
float targetMoveX = 0.0f;
float targetMoveY = 0.0f;
bool isFollowingMouse = false;

// --- Function Declarations ---
SDL_Texture* loadTexture(const char* path);
void saveHighScore();
void loadHighScore();
void renderText(const std::string& text, int x, int y, SDL_Color color);
void renderButton(Button& button);
void spawnEnemy();
void enemyShoot();
void updateGame(float deltaTime);
void cleanup();
// Các hàm saveGameState, loadGameState, resetGame, fileExists không có trong phiên bản này

// --- Function Definitions ---

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cerr << "Failed to load image: " << path << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        std::cerr << "Failed to create texture from " << path << " - " << SDL_GetError() << std::endl;
    }
    return texture;
}

void saveHighScore() {
    std::ofstream file("highscore.txt");
    if (file.is_open()) {
        file << highScore;
        file.close();
        std::cout << "Saved high score: " << highScore << std::endl;
    }
    else {
        std::cerr << "Error: Could not open highscore.txt for saving!" << std::endl;
    }
}

void loadHighScore() {
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        if (!(file >> highScore)) {
            std::cerr << "Warning: Could not read highscore from file or file is corrupted. Resetting to 0." << std::endl;
            highScore = 0;
        }
        else {
            if (highScore < 0) {
                std::cerr << "Warning: Invalid negative highscore found. Resetting to 0." << std::endl;
                highScore = 0;
            }
        }
        file.close();
        std::cout << "Loaded high score: " << highScore << std::endl;
    }
    else {
        std::cout << "highscore.txt not found. Starting with high score 0." << std::endl;
        highScore = 0;
    }
}

void renderText(const std::string& text, int x, int y, SDL_Color color) {
    if (!font) {
        std::cerr << "Font not loaded in renderText!\n";
        return;
    }
    if (text.empty()) {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "Failed to render text surface: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create text texture: " << SDL_GetError() << std::endl;
    }
    else {
        SDL_Rect dstRect = { x, y, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, NULL, &dstRect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void renderButton(Button& button) {
    if (!font) {
        std::cerr << "Font not loaded in renderButton!\n";
        return;
    }

    SDL_Color color = button.hovered ? SDL_Color{ 150, 150, 150, 255 } : SDL_Color{ 100, 100, 100, 255 };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &button.rect);

    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, button.text.c_str(), textColor);
    if (!textSurface) {
        std::cerr << "Failed to render button text surface: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "CreateTextureFromSurface Error (button): " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect textRect = { button.rect.x + (button.rect.w - textSurface->w) / 2,
                          button.rect.y + (button.rect.h - textSurface->h) / 2,
                          textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
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

    int spawnX = rand() % (SCREEN_WIDTH - ENEMY_WIDTH);
    newEnemy.rect = { spawnX, -ENEMY_HEIGHT, ENEMY_WIDTH, ENEMY_HEIGHT };
    newEnemy.xPos = (float)spawnX;
    newEnemy.yPos = (float)newEnemy.rect.y;
    newEnemy.speedX = 0;
    newEnemy.lastShotTime = SDL_GetTicks() + (rand() % 1000);

    switch (chosenType) {
    case NORMAL:
        newEnemy.texture = enemyTexture;
        newEnemy.health = NORMAL_INITIAL_HEALTH;
        newEnemy.maxHealth = NORMAL_INITIAL_HEALTH;
        newEnemy.bulletDamage = NORMAL_BULLET_DAMAGE;
        newEnemy.collisionDamage = NORMAL_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (1.0f + (rand() % 3) / 10.0f);
        newEnemy.enemyStopY = rand() % 200 + 150;
        newEnemy.fireCooldown = static_cast<Uint32>(1000 + (rand() % 1500));
        break;
    case STRAIGHT_SHOOTER:
        newEnemy.texture = enemyTextureStraight;
        newEnemy.health = STRAIGHT_SHOOTER_INITIAL_HEALTH;
        newEnemy.maxHealth = STRAIGHT_SHOOTER_INITIAL_HEALTH;
        newEnemy.bulletDamage = STRAIGHT_SHOOTER_BULLET_DAMAGE;
        newEnemy.collisionDamage = STRAIGHT_SHOOTER_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (1.2f + (rand() % 3) / 10.0f);
        newEnemy.enemyStopY = -1;
        newEnemy.fireCooldown = static_cast<Uint32>(1200 + (rand() % 1000));
        break;
    case WEAVER:
        newEnemy.texture = enemyTextureWeave;
        newEnemy.health = WEAVER_INITIAL_HEALTH;
        newEnemy.maxHealth = WEAVER_INITIAL_HEALTH;
        newEnemy.bulletDamage = WEAVER_BULLET_DAMAGE;
        newEnemy.collisionDamage = WEAVER_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (0.9f + (rand() % 2) / 10.0f);
        newEnemy.enemyStopY = -1;
        newEnemy.fireCooldown = UINT32_MAX; // Không bắn
        newEnemy.initialXPos = newEnemy.xPos;
        newEnemy.weaveAmplitude = 40.0f + (rand() % 40);
        newEnemy.weaveFrequency = 0.004f + (float)(rand() % 5) / 1000.0f;
        break;
    case TANK:
        newEnemy.texture = enemyTextureTank;
        newEnemy.health = TANK_INITIAL_HEALTH;
        newEnemy.maxHealth = TANK_INITIAL_HEALTH;
        newEnemy.bulletDamage = TANK_BULLET_DAMAGE;
        newEnemy.collisionDamage = TANK_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (0.7f + (rand() % 2) / 10.0f);
        newEnemy.enemyStopY = rand() % 150 + 100;
        newEnemy.fireCooldown = static_cast<Uint32>(1800 + (rand() % 2000));
        break;
    }
    enemies.push_back(newEnemy);
}

void enemyShoot() {
    Uint32 currentTime = SDL_GetTicks();
    for (auto& enemy : enemies) {
        bool canShoot = false;
        switch (enemy.type) {
        case NORMAL: case TANK:
            canShoot = (enemy.enemyStopY != -1 && enemy.yPos >= enemy.enemyStopY && currentTime > enemy.lastShotTime + enemy.fireCooldown);
            break;
        case STRAIGHT_SHOOTER:
            canShoot = (currentTime > enemy.lastShotTime + enemy.fireCooldown);
            break;
        case WEAVER:
            canShoot = false;
            break;
        }

        if (canShoot) {
            float startX = enemy.xPos + enemy.rect.w / 2.0f;
            float startY = enemy.yPos + enemy.rect.h;

            float bulletSpeedX = 0.0f;
            float bulletSpeedY = 0.0f;
            float bulletSpeedMagnitude = ENEMY_BULLET_SPEED * (enemy.type == TANK ? 0.8f : 1.0f);

            if (enemy.type == STRAIGHT_SHOOTER) {
                bulletSpeedX = 0.0f;
                bulletSpeedY = bulletSpeedMagnitude;
            }
            else { // NORMAL và TANK bắn về phía player
                float targetX = player.xPos + player.rect.w / 2.0f;
                float targetY = player.yPos + player.rect.h / 2.0f;
                float dirX = targetX - startX;
                float dirY = targetY - startY;
                float length = std::sqrt(dirX * dirX + dirY * dirY);
                if (length > 0.0001f) {
                    float invLength = 1.0f / length;
                    bulletSpeedX = dirX * invLength * bulletSpeedMagnitude;
                    bulletSpeedY = dirY * invLength * bulletSpeedMagnitude;
                }
                else {
                    bulletSpeedX = 0.0f; bulletSpeedY = bulletSpeedMagnitude;
                }
            }

            Entity enemyBullet;
            enemyBullet.texture = enemyBulletTexture ? enemyBulletTexture : bulletTexture;
            enemyBullet.rect = { (int)(startX - ENEMY_BULLET_WIDTH / 2.0f), (int)startY, ENEMY_BULLET_WIDTH, ENEMY_BULLET_HEIGHT };
            enemyBullet.xPos = (float)enemyBullet.rect.x;
            enemyBullet.yPos = (float)startY;
            enemyBullet.speedX = bulletSpeedX;
            enemyBullet.speedY = bulletSpeedY;
            enemyBullet.damage = enemy.bulletDamage;
            enemyBullets.push_back(enemyBullet);

            enemy.lastShotTime = currentTime;
        }
    }
}

void updateGame(float deltaTime) {
    // --- 1. Cập nhật & Xóa Đạn Người Chơi ---
    for (int i = (int)bullets.size() - 1; i >= 0; --i) {
        bullets[i].yPos -= PLAYER_BULLET_SPEED * deltaTime;
        bullets[i].rect.y = (int)bullets[i].yPos;
        bullets[i].xPos = (float)bullets[i].rect.x;

        if (bullets[i].rect.y + bullets[i].rect.h < 0) {
            bullets.erase(bullets.begin() + i);
        }
    }

    // --- 2. Cập nhật vị trí Kẻ Địch & Xóa ---
    for (int i = (int)enemies.size() - 1; i >= 0; --i) {
        if (i >= (int)enemies.size()) continue;
        Entity& enemy = enemies[i];

        bool stopped = (enemy.enemyStopY != -1 && enemy.yPos >= enemy.enemyStopY);
        if (!stopped) {
            enemy.yPos += enemy.speedY * deltaTime;
            if (enemy.enemyStopY != -1 && enemy.yPos > enemy.enemyStopY) {
                enemy.yPos = (float)enemy.enemyStopY;
            }
        }

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
        bool bulletHit = false;
        for (int j = (int)enemies.size() - 1; j >= 0; --j) {
            if (j >= (int)enemies.size()) continue;
            if (SDL_HasIntersection(&bullets[i].rect, &enemies[j].rect)) {
                enemies[j].health -= player.bulletDamage;
                bulletHit = true;
                bullets.erase(bullets.begin() + i);

                if (enemies[j].health <= 0) {
                    int scoreBonus = 10;
                    if (enemies[j].type == TANK) scoreBonus = 30;
                    else if (enemies[j].type == WEAVER) scoreBonus = 15;
                    currentScore += scoreBonus;
                    if (musicOn && explosionSound) {
                        Mix_PlayChannel(-1, explosionSound, 0);
                    }
                    enemies.erase(enemies.begin() + j);
                }
                else {
                    // TODO: Add hit effect?
                }
                break; // Đạn đã trúng, thoát vòng lặp địch
            }
        }
    }

    // --- 4. Va chạm: Kẻ Địch <-> Người Chơi ---
    for (int i = (int)enemies.size() - 1; i >= 0; --i) {
        if (i >= (int)enemies.size()) continue;
        if (SDL_HasIntersection(&enemies[i].rect, &player.rect)) {
            player.health -= enemies[i].collisionDamage;

            int scoreBonus = 5;
            if (enemies[i].type == TANK) scoreBonus = 15;
            else if (enemies[i].type == WEAVER) scoreBonus = 10;
            currentScore += scoreBonus;
            if (musicOn && explosionSound) {
                Mix_PlayChannel(-1, explosionSound, 0);
            }
            enemies.erase(enemies.begin() + i);

            if (player.health <= 0) {
                std::cout << "Player destroyed by collision! Game Over! Final Score: " << currentScore << std::endl;
                if (currentScore > highScore) { highScore = currentScore; saveHighScore(); }
                isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                state = GAME_OVER;
                return;
            }
        }
    }

    // --- 5. Cập nhật, Va chạm & Xóa Đạn Địch ---
    for (int i = (int)enemyBullets.size() - 1; i >= 0; --i) {
        if (i >= (int)enemyBullets.size()) continue;

        enemyBullets[i].xPos += enemyBullets[i].speedX * deltaTime;
        enemyBullets[i].yPos += enemyBullets[i].speedY * deltaTime;
        enemyBullets[i].rect.x = (int)enemyBullets[i].xPos;
        enemyBullets[i].rect.y = (int)enemyBullets[i].yPos;

        if (SDL_HasIntersection(&enemyBullets[i].rect, &player.rect)) {
            player.health -= enemyBullets[i].damage;
            enemyBullets.erase(enemyBullets.begin() + i);

            if (player.health <= 0) {
                std::cout << "Player destroyed by bullet! Game Over! Final Score: " << currentScore << std::endl;
                if (currentScore > highScore) { highScore = currentScore; saveHighScore(); }
                isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                state = GAME_OVER;
                return;
            }
        }
        else if (enemyBullets[i].rect.x + enemyBullets[i].rect.w < 0 || enemyBullets[i].rect.x > SCREEN_WIDTH ||
            enemyBullets[i].rect.y + enemyBullets[i].rect.h < 0 || enemyBullets[i].rect.y > SCREEN_HEIGHT) {
            enemyBullets.erase(enemyBullets.begin() + i);
        }
    }
}

void cleanup() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(gameBackgroundTexture);
    SDL_DestroyTexture(gameOverBackgroundTexture);
    SDL_DestroyTexture(enemyTextureStraight);
    SDL_DestroyTexture(enemyTextureWeave);
    SDL_DestroyTexture(enemyTextureTank);
    if (font) TTF_CloseFont(font);
    Mix_FreeMusic(bgMusic);
    Mix_FreeMusic(gameMusic);
    Mix_FreeChunk(startSound);
    Mix_FreeChunk(explosionSound);

    renderer = nullptr;
    window = nullptr;
    font = nullptr;
    playerTexture = nullptr;
    enemyTexture = nullptr;
    bulletTexture = nullptr;
    backgroundTexture = nullptr;
    gameBackgroundTexture = nullptr;
    gameOverBackgroundTexture = nullptr;
    enemyTextureStraight = nullptr;
    enemyTextureWeave = nullptr;
    enemyTextureTank = nullptr;
    bgMusic = nullptr;
    gameMusic = nullptr;
    startSound = nullptr;
    explosionSound = nullptr;
}

// --- Main Function ---
int main(int argc, char* argv[]) {
    // --- Initialize SDL and Subsystems ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    srand((unsigned int)time(0));
    loadHighScore();

    // --- Create Window, Renderer, Font ---
    window = SDL_CreateWindow("Airplane Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit();
        return -1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit();
        return -1;
    }
    font = TTF_OpenFont("arial.ttf", 28);
    if (!font) {
        std::cerr << "Failed to load font: arial.ttf - " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
        Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit();
        return -1;
    }

    // --- Load Resources ---
    backgroundTexture = loadTexture("background.png");
    gameBackgroundTexture = loadTexture("game_background.png");
    gameOverBackgroundTexture = loadTexture("gameover_background.png");
    playerTexture = loadTexture("player.png");
    enemyTexture = loadTexture("enemy.png");
    bulletTexture = loadTexture("bullet.png");
    enemyTextureStraight = loadTexture("enemy_straight.png");
    enemyTextureWeave = loadTexture("enemy_weave.png");
    enemyTextureTank = loadTexture("enemy_tank.png");
    bgMusic = Mix_LoadMUS("background.mp3");
    gameMusic = Mix_LoadMUS("game_music.mp3");
    startSound = Mix_LoadWAV("start_sound.wav");
    explosionSound = Mix_LoadWAV("explosion.wav");

    // --- Check Resource Loading ---
    if (!backgroundTexture || !gameBackgroundTexture || !gameOverBackgroundTexture ||
        !playerTexture || !enemyTexture || !bulletTexture ||
        !enemyTextureStraight || !enemyTextureWeave || !enemyTextureTank ||
        !bgMusic || !gameMusic || !explosionSound) {
        std::cerr << "Failed to load one or more resources!" << std::endl;
        cleanup();
        Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit();
        return -1;
    }

    // --- PHÁT NHẠC MENU BAN ĐẦU ---
    if (musicOn && bgMusic) {
        Mix_PlayMusic(bgMusic, -1);
    }

    // --- Initialize Player ---
    player.rect = { SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT - 100, PLAYER_WIDTH, PLAYER_HEIGHT };
    player.texture = playerTexture;
    player.xPos = (float)player.rect.x;
    player.yPos = (float)player.rect.y;
    player.health = PLAYER_INITIAL_HEALTH;
    player.maxHealth = PLAYER_INITIAL_HEALTH;
    player.bulletDamage = PLAYER_BULLET_DAMAGE;
    player.lastShotTime = 0;

    // --- Game Loop Variables ---
    SDL_Event event;
    Uint32 lastFrameTime = SDL_GetTicks();
    running = true;

    // --- Main Game Loop ---
    while (running) {
        // --- Delta Time ---
        Uint32 currentFrameTime = SDL_GetTicks();
        Uint32 frameTicks = SDL_TICKS_PASSED(currentFrameTime, lastFrameTime);
        float deltaTime = 0.0f;
        if (state != PAUSED) {
            deltaTime = frameTicks / 1000.0f;
            lastFrameTime = currentFrameTime;
            if (deltaTime > 0.1f) { deltaTime = 0.1f; }
        }
        else {
            deltaTime = 0.0f;
        }

        // --- Input ---
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_Point mousePointCheckHover = { mouseX, mouseY };

        // Xử lý hàng đợi sự kiện
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            // --- Xử lý nhấn chuột ---
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_Point mousePoint = { event.button.x, event.button.y };

                    if (state == MENU) {
                        for (int i = 0; i < BUTTON_COUNT; i++) {
                            if (buttons[i].hovered) {
                                if (i == 0) { // Start New Game
                                    currentScore = 0;
                                    enemies.clear(); bullets.clear(); enemyBullets.clear();
                                    currentEnemySpeed = INITIAL_ENEMY_SPEED;
                                    player.rect = { SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT - 100, PLAYER_WIDTH, PLAYER_HEIGHT };
                                    player.xPos = (float)player.rect.x; player.yPos = (float)player.rect.y;
                                    player.lastShotTime = 0;
                                    lastEnemySpawnTime = SDL_GetTicks();
                                    player.health = player.maxHealth; // <<< Sửa lỗi thiếu maxHealth
                                    Mix_HaltMusic();
                                    if (musicOn && gameMusic) {
                                        Mix_PlayMusic(gameMusic, -1);
                                    }
                                    isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                                    state = GAME;
                                    lastFrameTime = SDL_GetTicks(); // <<< Reset time khi bắt đầu game mới
                                }
                                else if (i == 1) { // Continue
                                    // Chức năng chưa thêm vào phiên bản này
                                    std::cout << "Continue button clicked (Not implemented in this version)" << std::endl;
                                }
                                else if (i == 2) { // Music On/Off
                                    musicOn = !musicOn;
                                    buttons[i].text = musicOn ? "Music: On" : "Music: Off";
                                    pauseMenuButtons[1].text = musicOn ? "Mute Sound" : "Unmute Sound";
                                    if (musicOn) {
                                        Mix_Resume(-1);
                                        if (Mix_PausedMusic()) Mix_ResumeMusic();
                                        else if (!Mix_PlayingMusic()) {
                                            if (state == MENU && bgMusic) Mix_PlayMusic(bgMusic, -1);
                                            else if ((state == GAME || state == PAUSED) && gameMusic) Mix_PlayMusic(gameMusic, -1);
                                        }
                                    }
                                    else {
                                        if (Mix_PlayingMusic()) Mix_PauseMusic();
                                        Mix_Pause(-1);
                                    }
                                }
                                else if (i == 3) { // Exit Game
                                    running = false;
                                }
                                break;
                            }
                        }
                    }
                    else if (state == GAME_OVER) {
                        for (int i = 0; i < GAMEOVER_BUTTON_COUNT; i++) {
                            if (gameOverButtons[i].hovered) {
                                if (i == 0) { // Play Again
                                    currentScore = 0;
                                    enemies.clear(); bullets.clear(); enemyBullets.clear();
                                    currentEnemySpeed = INITIAL_ENEMY_SPEED;
                                    player.rect = { SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT - 100, PLAYER_WIDTH, PLAYER_HEIGHT };
                                    player.xPos = (float)player.rect.x; player.yPos = (float)player.rect.y;
                                    player.lastShotTime = 0;
                                    lastEnemySpawnTime = SDL_GetTicks();
                                    player.health = player.maxHealth; // <<< Sửa lỗi thiếu maxHealth
                                    isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                                    Mix_HaltMusic();
                                    if (musicOn && gameMusic) {
                                        Mix_PlayMusic(gameMusic, -1);
                                    }
                                    state = GAME;
                                    lastFrameTime = SDL_GetTicks(); // <<< Reset time khi chơi lại
                                }
                                else if (i == 1) { // Main Menu
                                    isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                                    state = MENU;
                                    Mix_HaltMusic();
                                    if (musicOn && bgMusic) {
                                        Mix_PlayMusic(bgMusic, -1);
                                    }
                                }
                                break;
                            }
                        }
                    }
                    else if (state == GAME) {
                        if (SDL_PointInRect(&mousePoint, &pauseButton.rect)) {
                            state = PAUSED;
                            pauseMenuButtons[1].text = musicOn ? "Mute Sound" : "Unmute Sound";
                        }
                        else {
                            isFollowingMouse = false;
                            if (SDL_PointInRect(&mousePoint, &player.rect)) {
                                isPlayerDragging = true;
                                isMovingToTarget = false;
                            }
                            else {
                                isMovingToTarget = true;
                                isPlayerDragging = false;
                                targetMoveX = (float)mousePoint.x - player.rect.w / 2.0f;
                                targetMoveY = (float)mousePoint.y - player.rect.h / 2.0f;
                                targetMoveX = std::max(0.0f, std::min((float)SCREEN_WIDTH - player.rect.w, targetMoveX));
                                targetMoveY = std::max(0.0f, std::min((float)SCREEN_HEIGHT - player.rect.h, targetMoveY));
                            }
                        }
                    }
                    else if (state == PAUSED) {
                        for (int i = 0; i < PAUSE_MENU_BUTTON_COUNT; ++i) {
                            if (pauseMenuButtons[i].hovered) {
                                if (i == 0) { // Continue
                                    state = GAME;
                                    lastFrameTime = SDL_GetTicks();
                                }
                                else if (i == 1) { // Mute/Unmute Sound
                                    musicOn = !musicOn;
                                    pauseMenuButtons[i].text = musicOn ? "Mute Sound" : "Unmute Sound";
                                    buttons[2].text = musicOn ? "Music: On" : "Music: Off";
                                    if (musicOn) {
                                        Mix_Resume(-1);
                                        if (Mix_PausedMusic()) Mix_ResumeMusic();
                                        else if (!Mix_PlayingMusic() && gameMusic) Mix_PlayMusic(gameMusic, -1);
                                    }
                                    else {
                                        if (Mix_PlayingMusic()) Mix_PauseMusic();
                                        Mix_Pause(-1);
                                    }
                                }
                                else if (i == 2) { // Save & Exit to Menu
                                    // Chức năng chưa thêm vào phiên bản này
                                    std::cout << "Save & Exit clicked (Not implemented in this version)" << std::endl;
                                    // Vẫn thoát ra menu như cũ
                                    if (currentScore > highScore) {
                                        highScore = currentScore;
                                        saveHighScore();
                                    }
                                    Mix_HaltMusic();
                                    isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                                    state = MENU;
                                    if (musicOn && bgMusic) {
                                        Mix_PlayMusic(bgMusic, -1);
                                    }
                                    buttons[2].text = musicOn ? "Music: On" : "Music: Off";
                                }
                                break;
                            }
                        }
                        // Optional click pause icon to resume
                        // if (pauseButton.hovered && SDL_PointInRect(&mousePoint, &pauseButton.rect)) {
                        //    state = GAME;
                        //    lastFrameTime = SDL_GetTicks();
                        // }
                    }
                }
            }
            // --- Xử lý nhả chuột ---
            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (state == GAME) {
                        if (isPlayerDragging) {
                            isPlayerDragging = false;
                            isFollowingMouse = false;
                        }
                    }
                }
            }
        } // end poll event

        // --- Update ---
        if (state == GAME) {
            // Cập nhật vị trí người chơi
            if (isPlayerDragging) {
                player.xPos = (float)mouseX - player.rect.w / 2.0f;
                player.yPos = (float)mouseY - player.rect.h / 2.0f;
            }
            else if (isMovingToTarget) {
                float dirX = targetMoveX - player.xPos;
                float dirY = targetMoveY - player.yPos;
                float dist = std::sqrt(dirX * dirX + dirY * dirY);
                bool reachedTarget = false;
                if (dist < 2.0f) {
                    reachedTarget = true;
                }
                else {
                    float moveAmount = PLAYER_SPEED * deltaTime;
                    if (moveAmount >= dist) {
                        reachedTarget = true;
                    }
                    else {
                        player.xPos += (dirX / dist) * moveAmount;
                        player.yPos += (dirY / dist) * moveAmount;
                    }
                }
                if (reachedTarget) {
                    player.xPos = targetMoveX;
                    player.yPos = targetMoveY;
                    isMovingToTarget = false;
                    isFollowingMouse = true;
                }
            }
            else if (isFollowingMouse) {
                player.xPos = (float)mouseX - player.rect.w / 2.0f;
                player.yPos = (float)mouseY - player.rect.h / 2.0f;
            }

            player.xPos = std::max(0.0f, std::min((float)SCREEN_WIDTH - player.rect.w, player.xPos));
            player.yPos = std::max(0.0f, std::min((float)SCREEN_HEIGHT - player.rect.h, player.yPos));
            player.rect.x = (int)player.xPos;
            player.rect.y = (int)player.yPos;

            // Bắn tự động
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime > player.lastShotTime + PLAYER_AUTO_FIRE_COOLDOWN) {
                Entity newBullet;
                newBullet.rect = { player.rect.x + player.rect.w / 2 - PLAYER_BULLET_WIDTH / 2, player.rect.y, PLAYER_BULLET_WIDTH, PLAYER_BULLET_HEIGHT };
                newBullet.texture = bulletTexture;
                newBullet.xPos = (float)newBullet.rect.x;
                newBullet.yPos = (float)player.rect.y;
                // Gán damage cho đạn player
                newBullet.damage = player.bulletDamage; // <<< Thêm dòng này
                bullets.push_back(newBullet);
                player.lastShotTime = currentTime;
            }

            // Cập nhật logic game khác
            if (currentTime > lastEnemySpawnTime + ENEMY_SPAWN_INTERVAL) {
                spawnEnemy();
                lastEnemySpawnTime = currentTime;
            }
            enemyShoot();
            updateGame(deltaTime); // Gọi hàm update chính
        }
        else {
            isPlayerDragging = false;
            isMovingToTarget = false;
            isFollowingMouse = false;
        }

        // --- Cập nhật trạng thái hover của nút ---
        if (state == MENU) {
            for (int i = 0; i < BUTTON_COUNT; i++) {
                buttons[i].hovered = SDL_PointInRect(&mousePointCheckHover, &buttons[i].rect);
                // Nút Continue chưa làm nên không cần xử lý disable/hover đặc biệt
            }
            for (int i = 0; i < GAMEOVER_BUTTON_COUNT; ++i) gameOverButtons[i].hovered = false;
            for (int i = 0; i < PAUSE_MENU_BUTTON_COUNT; ++i) pauseMenuButtons[i].hovered = false;
            pauseButton.hovered = false;
        }
        else if (state == GAME_OVER) {
            for (int i = 0; i < GAMEOVER_BUTTON_COUNT; i++) {
                gameOverButtons[i].hovered = SDL_PointInRect(&mousePointCheckHover, &gameOverButtons[i].rect);
            }
            for (int i = 0; i < BUTTON_COUNT; ++i) buttons[i].hovered = false;
            for (int i = 0; i < PAUSE_MENU_BUTTON_COUNT; ++i) pauseMenuButtons[i].hovered = false;
            pauseButton.hovered = false;
        }
        else if (state == GAME) {
            pauseButton.hovered = SDL_PointInRect(&mousePointCheckHover, &pauseButton.rect);
            for (int i = 0; i < BUTTON_COUNT; ++i) buttons[i].hovered = false;
            for (int i = 0; i < GAMEOVER_BUTTON_COUNT; ++i) gameOverButtons[i].hovered = false;
            for (int i = 0; i < PAUSE_MENU_BUTTON_COUNT; ++i) pauseMenuButtons[i].hovered = false;
        }
        else if (state == PAUSED) {
            pauseButton.hovered = SDL_PointInRect(&mousePointCheckHover, &pauseButton.rect);
            for (int i = 0; i < PAUSE_MENU_BUTTON_COUNT; ++i) {
                pauseMenuButtons[i].hovered = SDL_PointInRect(&mousePointCheckHover, &pauseMenuButtons[i].rect);
            }
            for (int i = 0; i < BUTTON_COUNT; ++i) buttons[i].hovered = false;
            for (int i = 0; i < GAMEOVER_BUTTON_COUNT; ++i) gameOverButtons[i].hovered = false;
        }

        // --- Render ---
        SDL_RenderClear(renderer);

        // --- 1. Vẽ Nền theo State ---
        if (state == MENU) {
            if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        }
        else if (state == GAME_OVER) {
            if (gameOverBackgroundTexture) SDL_RenderCopy(renderer, gameOverBackgroundTexture, NULL, NULL);
        }
        else { // GAME or PAUSED
            if (gameBackgroundTexture) SDL_RenderCopy(renderer, gameBackgroundTexture, NULL, NULL);
        }

        // --- 2. Khai báo màu ---
        SDL_Color white = { 255, 255, 255, 255 };
        SDL_Color yellow = { 255, 255, 0, 255 };
        SDL_Color gray = { 0, 0, 0, 150 };

        // --- 3. Vẽ Nội dung chồng lên Nền theo State ---
        if (state == MENU) {
            for (int i = 0; i < BUTTON_COUNT; i++) renderButton(buttons[i]);
            renderText("High Score: " + std::to_string(highScore), 10, 10, white);
        }
        else if (state == GAME_OVER) {
            int textW = 0, textH = 0;
            std::string gameOverMsg = "Game Over!";
            if (font) TTF_SizeText(font, gameOverMsg.c_str(), &textW, &textH);
            renderText(gameOverMsg, SCREEN_WIDTH / 2 - textW / 2, 150, yellow);

            std::string finalScoreText = "Your Score: " + std::to_string(currentScore);
            if (font) TTF_SizeText(font, finalScoreText.c_str(), &textW, &textH);
            renderText(finalScoreText, SCREEN_WIDTH / 2 - textW / 2, 220, white);

            std::string hsText = "High Score: " + std::to_string(highScore);
            if (font) TTF_SizeText(font, hsText.c_str(), &textW, &textH);
            renderText(hsText, SCREEN_WIDTH / 2 - textW / 2, 260, white);

            for (int i = 0; i < GAMEOVER_BUTTON_COUNT; i++) renderButton(gameOverButtons[i]);
        }
        else { // GAME or PAUSED
            if (player.texture) SDL_RenderCopy(renderer, player.texture, NULL, &player.rect);
            for (auto& enemy : enemies) if (enemy.texture) SDL_RenderCopy(renderer, enemy.texture, NULL, &enemy.rect);
            for (auto& bullet : bullets) if (bullet.texture) SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);
            for (auto& bullet : enemyBullets) if (bullet.texture) SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);

            renderText("Score: " + std::to_string(currentScore), 10, 10, white);

            int healthBarX = 10, healthBarY = 40, healthBarW = 200, healthBarH = 20;
            float healthPercent = (player.health > 0) ? (float)player.health / player.maxHealth : 0.0f;
            int currentHealthBarW = (int)(healthBarW * healthPercent);
            if (currentHealthBarW < 0) currentHealthBarW = 0;
            SDL_Rect healthBarBgRect = { healthBarX, healthBarY, healthBarW, healthBarH };
            SDL_SetRenderDrawColor(renderer, 100, 0, 0, 255); SDL_RenderFillRect(renderer, &healthBarBgRect);
            SDL_Rect currentHealthRect = { healthBarX, healthBarY, currentHealthBarW, healthBarH };
            SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255); SDL_RenderFillRect(renderer, &currentHealthRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); SDL_RenderDrawRect(renderer, &healthBarBgRect);

            renderButton(pauseButton);

            if (state == PAUSED) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, gray.r, gray.g, gray.b, gray.a);
                SDL_Rect pauseOverlayRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
                SDL_RenderFillRect(renderer, &pauseOverlayRect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

                for (int i = 0; i < PAUSE_MENU_BUTTON_COUNT; ++i) {
                    renderButton(pauseMenuButtons[i]);
                }
            }
        }

        // --- 4. Hiển thị mọi thứ lên màn hình ---
        SDL_RenderPresent(renderer);
    } // --- Kết thúc while(running) ---

    // --- Cleanup ---
    cleanup();
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
} // <<< Kết thúc main