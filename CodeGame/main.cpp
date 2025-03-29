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
// const int ENEMY_WIDTH = 60; // <<< Có thể bỏ hoặc comment đi nếu không dùng chung nữa
// const int ENEMY_HEIGHT = 60; // <<< Có thể bỏ hoặc comment đi nếu không dùng chung nữa
// const int ENEMY_BULLET_WIDTH = 10; // <<< Có thể bỏ hoặc comment đi nếu không dùng chung nữa
// const int ENEMY_BULLET_HEIGHT = 20; // <<< Có thể bỏ hoặc comment đi nếu không dùng chung nữa

// --- THÊM MỚI: Kích thước địch theo loại ---
// Kích thước cho địch NORMAL (Ví dụ: Giữ nguyên như cũ)
const int NORMAL_ENEMY_WIDTH = 60;
const int NORMAL_ENEMY_HEIGHT = 60;
// tỉ lệ 1:2
const int NORMAL_BULLET_WIDTH = 10;
const int NORMAL_BULLET_HEIGHT = 20;

// Kích thước cho địch STRAIGHT_SHOOTER (Ví dụ: Hẹp hơn, dài hơn)
// tỉ lệ 4:3
const int STRAIGHT_SHOOTER_ENEMY_WIDTH = 60;
const int STRAIGHT_SHOOTER_ENEMY_HEIGHT = 45;
// tỉ lệ 2:1
const int STRAIGHT_SHOOTER_BULLET_WIDTH = 60; // Đạn nhỏ hơn
const int STRAIGHT_SHOOTER_BULLET_HEIGHT = 30; // Đạn dài hơn

// Kích thước cho địch WEAVER (Ví dụ: Rộng hơn, thấp hơn)
//tỉ lệ 5:4
const int WEAVER_ENEMY_WIDTH = 75;
const int WEAVER_ENEMY_HEIGHT = 60;
// tỉ lệ 4:3
const int WEAVER_BULLET_WIDTH = 34;  // Ví dụ: Đạn tròn nhỏ
const int WEAVER_BULLET_HEIGHT = 25; // Ví dụ: Đạn tròn nhỏ

// Kích thước cho địch TANK (Ví dụ: Lớn hơn về mọi mặt)
//tỉ lệ 5:4
const int TANK_ENEMY_WIDTH = 75;
const int TANK_ENEMY_HEIGHT = 60;
// tỉ lệ 4:3
const int TANK_BULLET_WIDTH = 34; // Đạn to hơn
const int TANK_BULLET_HEIGHT = 25;

// -----------------------------------------------------

// --- THÊM MỚI: Kích thước Player và Đạn theo Level ---
// Level 1
// tỉ lệ 5:7
const int PLAYER_WIDTH_LVL1 = 60;
const int PLAYER_HEIGHT_LVL1 = 84;
// tỉ lệ 1:2
const int PLAYER_BULLET_WIDTH_LVL1 = 15; // Giữ nguyên kích thước bạn đã tăng
const int PLAYER_BULLET_HEIGHT_LVL1 = 30; // Giữ nguyên kích thước bạn đã tăng

// Level 2 (Ví dụ: Tăng nhẹ)
// tỉ lệ 5:7
const int PLAYER_WIDTH_LVL2 = 60;
const int PLAYER_HEIGHT_LVL2 = 84;
// tỉ lệ 1:1
const int PLAYER_BULLET_WIDTH_LVL2 = 35;
const int PLAYER_BULLET_HEIGHT_LVL2 = 35;

// Level 3 (Ví dụ: Tăng nữa)
//tỉ lệ 3:2
const int PLAYER_WIDTH_LVL3 = 125;
const int PLAYER_HEIGHT_LVL3 = 84;
// tỉ lệ 4:3
const int PLAYER_BULLET_WIDTH_LVL3 = 54;
const int PLAYER_BULLET_HEIGHT_LVL3 = 40;

// Level 4 (Ví dụ: Lớn nhất)
// tỉ lệ 7:5
const int PLAYER_WIDTH_LVL4 = 168;
const int PLAYER_HEIGHT_LVL4 = 120;
// tỉ lệ 2:1
const int PLAYER_BULLET_WIDTH_LVL4 = 80;
const int PLAYER_BULLET_HEIGHT_LVL4 = 40;
// -----------------------------------------------------


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
    int level = 1;

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
    "Pause", // Luôn là "||"
    false
};

// --- THÊM MỚI: Nút Pause Menu ---
const int PAUSE_MENU_BUTTON_WIDTH = 280;
const int PAUSE_MENU_BUTTON_HEIGHT = 70;
const int pauseMenuButtonX = SCREEN_WIDTH / 2 - PAUSE_MENU_BUTTON_WIDTH / 2; // Căn giữa X
Button pauseMenuButtons[] = {
    {{pauseMenuButtonX, 250, PAUSE_MENU_BUTTON_WIDTH, PAUSE_MENU_BUTTON_HEIGHT}, "Continue", false},
    {{pauseMenuButtonX, 340, PAUSE_MENU_BUTTON_WIDTH, PAUSE_MENU_BUTTON_HEIGHT}, "Mute Sound", false}, // Text sẽ được cập nhật khi vào Pause
    {{pauseMenuButtonX, 430, PAUSE_MENU_BUTTON_WIDTH, PAUSE_MENU_BUTTON_HEIGHT}, "Exit to Menu", false} // <<< Chức năng chưa được thêm vào phiên bản này
};
const int PAUSE_MENU_BUTTON_COUNT = sizeof(pauseMenuButtons) / sizeof(Button);

// --- Global Variables ---
SDL_Texture* playerTextureLvl1 = nullptr;
SDL_Texture* playerTextureLvl2 = nullptr;
SDL_Texture* playerTextureLvl3 = nullptr;
SDL_Texture* playerTextureLvl4 = nullptr;
SDL_Texture* bulletTextureLvl1 = nullptr;
SDL_Texture* bulletTextureLvl2 = nullptr;
SDL_Texture* bulletTextureLvl3 = nullptr;
SDL_Texture* bulletTextureLvl4 = nullptr;
SDL_Texture* enemyTexture = nullptr;          // Giữ lại dòng này
// SDL_Texture* enemyBulletTexture = nullptr; // <<< XÓA DÒNG NÀY
// --- THÊM MỚI: Textures cho địch mới ---
SDL_Texture* enemyTextureStraight = nullptr;
SDL_Texture* enemyTextureWeave = nullptr;
SDL_Texture* enemyTextureTank = nullptr;
// <<< THÊM MỚI: Textures cho đạn địch riêng >>>
SDL_Texture* normalBulletTexture = nullptr;
SDL_Texture* straightShooterBulletTexture = nullptr;
SDL_Texture* tankBulletTexture = nullptr;
SDL_Texture* weaverBulletTexture = nullptr;
// ------------------------------------

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
void checkPlayerLevelUp();
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

// <<< THÊM MỚI: Hàm kiểm tra và xử lý tăng cấp >>>
void checkPlayerLevelUp() {
    if (player.level >= 4) return; // Đã đạt cấp tối đa

    int scoreRequiredForNextLevel = player.level * 250;

    if (currentScore >= scoreRequiredForNextLevel) {
        player.level++;
        std::cout << "Player Leveled Up to Level: " << player.level << std::endl;

        // <<< THÊM MỚI: Lưu vị trí tâm cũ >>>
        float oldCenterX = player.xPos + player.rect.w / 2.0f;
        float oldCenterY = player.yPos + player.rect.h / 2.0f;

        // --- Cập nhật texture VÀ KÍCH THƯỚC người chơi ---
        switch (player.level) {
        case 2:
            player.texture = playerTextureLvl2;
            player.rect.w = PLAYER_WIDTH_LVL2;   // <<< Cập nhật kích thước
            player.rect.h = PLAYER_HEIGHT_LVL2;  // <<< Cập nhật kích thước
            // Optional: player.bulletDamage += 1;
            break;
        case 3:
            player.texture = playerTextureLvl3;
            player.rect.w = PLAYER_WIDTH_LVL3;   // <<< Cập nhật kích thước
            player.rect.h = PLAYER_HEIGHT_LVL3;  // <<< Cập nhật kích thước
            break;
        case 4:
            player.texture = playerTextureLvl4;
            player.rect.w = PLAYER_WIDTH_LVL4;   // <<< Cập nhật kích thước
            player.rect.h = PLAYER_HEIGHT_LVL4;  // <<< Cập nhật kích thước
            break;
        }

        // <<< THÊM MỚI: Cập nhật lại xPos/yPos để giữ nguyên tâm >>>
        player.xPos = oldCenterX - player.rect.w / 2.0f;
        player.yPos = oldCenterY - player.rect.h / 2.0f;

        // <<< THÊM MỚI: Đảm bảo không ra khỏi màn hình sau khi đổi kích thước/vị trí >>>
        player.xPos = std::max(0.0f, std::min((float)SCREEN_WIDTH - player.rect.w, player.xPos));
        player.yPos = std::max(0.0f, std::min((float)SCREEN_HEIGHT - player.rect.h, player.yPos));

        // Cập nhật rect x,y từ xPos, yPos mới (quan trọng sau khi điều chỉnh vị trí)
        player.rect.x = (int)player.xPos;
        player.rect.y = (int)player.yPos;

        // Optional: Play level up sound
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

    // <<< SỬA ĐỔI: Xác định kích thước trước khi tính spawnX (nếu cần) hoặc đặt trong switch >>>
    int currentEnemyWidth = 0; // Biến tạm để lưu width
    int currentEnemyHeight = 0; // Biến tạm để lưu height

    // Đặt kích thước dựa trên type TRƯỚC KHI dùng nó trong rect
    switch (chosenType) {
    case NORMAL:
        currentEnemyWidth = NORMAL_ENEMY_WIDTH;
        currentEnemyHeight = NORMAL_ENEMY_HEIGHT;
        newEnemy.texture = enemyTexture;
        // ... (các thuộc tính khác của NORMAL)
        break;
    case STRAIGHT_SHOOTER:
        currentEnemyWidth = STRAIGHT_SHOOTER_ENEMY_WIDTH;
        currentEnemyHeight = STRAIGHT_SHOOTER_ENEMY_HEIGHT;
        newEnemy.texture = enemyTextureStraight;
        // ... (các thuộc tính khác của STRAIGHT_SHOOTER)
        break;
    case WEAVER:
        currentEnemyWidth = WEAVER_ENEMY_WIDTH;
        currentEnemyHeight = WEAVER_ENEMY_HEIGHT;
        newEnemy.texture = enemyTextureWeave;
        // ... (các thuộc tính khác của WEAVER)
        break;
    case TANK:
        currentEnemyWidth = TANK_ENEMY_WIDTH;
        currentEnemyHeight = TANK_ENEMY_HEIGHT;
        newEnemy.texture = enemyTextureTank;
        // ... (các thuộc tính khác của TANK)
        break;
    }

    // <<< SỬA ĐỔI: Tính toán spawnX cẩn thận hơn nếu width thay đổi nhiều >>>
    // Cách đơn giản: Dùng width vừa xác định
    int spawnX = rand() % (SCREEN_WIDTH - currentEnemyWidth);
    // Cách an toàn hơn nếu kích thước chênh lệch lớn (dùng max width):
    // int maxEnemyWidth = std::max({NORMAL_ENEMY_WIDTH, STRAIGHT_SHOOTER_ENEMY_WIDTH, WEAVER_ENEMY_WIDTH, TANK_ENEMY_WIDTH});
    // int spawnX = rand() % (SCREEN_WIDTH - maxEnemyWidth);


    // <<< SỬA ĐỔI: Sử dụng kích thước đã xác định khi tạo rect >>>
    newEnemy.rect = { spawnX, -currentEnemyHeight, currentEnemyWidth, currentEnemyHeight };
    newEnemy.xPos = (float)spawnX;
    newEnemy.yPos = (float)newEnemy.rect.y;
    newEnemy.speedX = 0;
    newEnemy.lastShotTime = SDL_GetTicks() + (rand() % 1000);

    // <<< DI CHUYỂN: Phần còn lại của switch để gán health, damage, speed,... vào đây >>>
    switch (chosenType) {
    case NORMAL:
        // newEnemy.texture = enemyTexture; // Đã gán ở trên
        newEnemy.health = NORMAL_INITIAL_HEALTH;
        newEnemy.maxHealth = NORMAL_INITIAL_HEALTH;
        newEnemy.bulletDamage = NORMAL_BULLET_DAMAGE;
        newEnemy.collisionDamage = NORMAL_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (1.0f + (rand() % 3) / 10.0f);
        newEnemy.enemyStopY = rand() % 200 + 150;
        newEnemy.fireCooldown = static_cast<Uint32>(1000 + (rand() % 1500));
        break;
    case STRAIGHT_SHOOTER:
        // newEnemy.texture = enemyTextureStraight; // Đã gán ở trên
        newEnemy.health = STRAIGHT_SHOOTER_INITIAL_HEALTH;
        newEnemy.maxHealth = STRAIGHT_SHOOTER_INITIAL_HEALTH;
        newEnemy.bulletDamage = STRAIGHT_SHOOTER_BULLET_DAMAGE;
        newEnemy.collisionDamage = STRAIGHT_SHOOTER_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (1.2f + (rand() % 3) / 10.0f);
        newEnemy.enemyStopY = -1;
        newEnemy.fireCooldown = static_cast<Uint32>(1200 + (rand() % 1000));
        break;
    case WEAVER:
        // newEnemy.texture = enemyTextureWeave; // Đã gán ở trên
        newEnemy.health = WEAVER_INITIAL_HEALTH;
        newEnemy.maxHealth = WEAVER_INITIAL_HEALTH;
        newEnemy.bulletDamage = WEAVER_BULLET_DAMAGE; // Sát thương đạn Weaver
        newEnemy.collisionDamage = WEAVER_COLLISION_DAMAGE;
        newEnemy.speedY = currentEnemySpeed * (0.9f + (rand() % 2) / 10.0f);
        newEnemy.enemyStopY = -1; // Không dừng lại
        // <<< SỬA ĐỔI: Cho phép bắn liên tục >>>
        // Giá trị nhỏ hơn -> bắn nhanh hơn. Ví dụ: 500ms +/- 200ms
        newEnemy.fireCooldown = static_cast<Uint32>(500 + (rand() % 401) - 200);
        newEnemy.initialXPos = newEnemy.xPos;
        // <<< SỬA ĐỔI: Tăng biên độ >>>
        newEnemy.weaveAmplitude = 150.0f + (rand() % 100); // Ví dụ: biên độ lớn hơn (150-249)
        newEnemy.weaveFrequency = 0.004f + (float)(rand() % 5) / 1000.0f; // Giữ nguyên hoặc điều chỉnh tần số nếu muốn
        break;
    case TANK:
        // newEnemy.texture = enemyTextureTank; // Đã gán ở trên
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
        // <<< SỬA ĐỔI: Cho phép WEAVER bắn dựa trên cooldown >>>
        switch (enemy.type) {
        case NORMAL: case TANK:
            canShoot = (enemy.enemyStopY != -1 && enemy.yPos >= enemy.enemyStopY && currentTime > enemy.lastShotTime + enemy.fireCooldown);
            break;
        case STRAIGHT_SHOOTER:
        case WEAVER: // Weaver giờ cũng bắn theo cooldown
            canShoot = (currentTime > enemy.lastShotTime + enemy.fireCooldown);
            break;
            // case WEAVER: // Xóa hoặc comment dòng cũ
            //     canShoot = false;
            //     break;
        }

        if (canShoot) {
            float startX = enemy.xPos + enemy.rect.w / 2.0f;
            float startY = enemy.yPos + enemy.rect.h;

            float bulletSpeedX = 0.0f;
            float bulletSpeedY = 0.0f;
            float bulletSpeedMagnitude = ENEMY_BULLET_SPEED; // Tốc độ gốc

            // <<< SỬA ĐỔI: Xác định hướng bắn cho WEAVER (ví dụ: bắn thẳng) >>>
            if (enemy.type == STRAIGHT_SHOOTER || enemy.type == WEAVER) { // Bắn thẳng xuống
                bulletSpeedX = 0.0f;
                bulletSpeedY = bulletSpeedMagnitude;
                // Optional: Điều chỉnh tốc độ đạn riêng cho Weaver nếu muốn
                // if (enemy.type == WEAVER) bulletSpeedY *= 0.8f; // Ví dụ: chậm hơn chút
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

            // <<< THÊM MỚI / SỬA ĐỔI: Xác định kích thước VÀ TEXTURE đạn dựa trên loại địch >>>
            int bulletW = 0;
            int bulletH = 0;
            switch (enemy.type) {
            case NORMAL:
                bulletW = NORMAL_BULLET_WIDTH;
                bulletH = NORMAL_BULLET_HEIGHT;
                enemyBullet.texture = normalBulletTexture;
                break;
            case STRAIGHT_SHOOTER:
                bulletW = STRAIGHT_SHOOTER_BULLET_WIDTH;
                bulletH = STRAIGHT_SHOOTER_BULLET_HEIGHT;
                enemyBullet.texture = straightShooterBulletTexture;
                break;
            case TANK:
                bulletW = TANK_BULLET_WIDTH;
                bulletH = TANK_BULLET_HEIGHT;
                enemyBullet.texture = tankBulletTexture;
                break;
                // <<< THÊM MỚI: Case cho đạn WEAVER >>>
            case WEAVER:
                bulletW = WEAVER_BULLET_WIDTH;
                bulletH = WEAVER_BULLET_HEIGHT;
                enemyBullet.texture = weaverBulletTexture; // GÁN TEXTURE ĐẠN WEAVER
                break;
            default: // Trường hợp dự phòng
                bulletW = NORMAL_BULLET_WIDTH;
                bulletH = NORMAL_BULLET_HEIGHT;
                enemyBullet.texture = normalBulletTexture;
                break;
            }

            // Kiểm tra texture SAU KHI đã gán
            if (!enemyBullet.texture) {
                std::cerr << "Warning: Enemy bullet texture is null for type " << static_cast<int>(enemy.type)
                    << " in enemyShoot! Cannot create bullet." << std::endl;
                enemy.lastShotTime = currentTime;
                continue;
            }

            // Tạo rect và các thuộc tính khác
            enemyBullet.rect = { (int)(startX - bulletW / 2.0f), (int)startY, bulletW, bulletH };
            enemyBullet.xPos = (float)enemyBullet.rect.x;
            enemyBullet.yPos = (float)startY;
            enemyBullet.speedX = bulletSpeedX;
            enemyBullet.speedY = bulletSpeedY;
            enemyBullet.damage = enemy.bulletDamage; // Lấy sát thương đã gán cho Weaver
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
                    checkPlayerLevelUp();
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

            int scoreBonus = 5; // Điểm cơ bản khi va chạm
            if (enemies[i].type == TANK) scoreBonus = 15;
            else if (enemies[i].type == WEAVER) scoreBonus = 10;
            currentScore += scoreBonus; // Chỉ cộng điểm một lần
            checkPlayerLevelUp();
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
    // SDL_DestroyTexture(playerTexture); // <<< Xóa hoặc comment
    // SDL_DestroyTexture(bulletTexture); // <<< Xóa hoặc comment

    // <<< THÊM MỚI: Giải phóng các texture cấp độ >>>
    SDL_DestroyTexture(playerTextureLvl1);
    SDL_DestroyTexture(playerTextureLvl2);
    SDL_DestroyTexture(playerTextureLvl3);
    SDL_DestroyTexture(playerTextureLvl4);
    SDL_DestroyTexture(bulletTextureLvl1);
    SDL_DestroyTexture(bulletTextureLvl2);
    SDL_DestroyTexture(bulletTextureLvl3);
    SDL_DestroyTexture(bulletTextureLvl4);

    SDL_DestroyTexture(enemyTexture);
    //SDL_DestroyTexture(enemyBulletTexture); // <<< SỬA ĐỔI: Đảm bảo giải phóng texture đạn địch
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(gameBackgroundTexture);
    SDL_DestroyTexture(gameOverBackgroundTexture);
    SDL_DestroyTexture(enemyTextureStraight);
    SDL_DestroyTexture(enemyTextureWeave);
    SDL_DestroyTexture(enemyTextureTank);

    // <<< THÊM MỚI: Giải phóng texture đạn địch riêng >>>
    SDL_DestroyTexture(normalBulletTexture);
    SDL_DestroyTexture(straightShooterBulletTexture);
    SDL_DestroyTexture(tankBulletTexture);
    SDL_DestroyTexture(weaverBulletTexture);
    // ------------------------------------------

    if (font) TTF_CloseFont(font);
    Mix_FreeMusic(bgMusic);
    Mix_FreeMusic(gameMusic);
    Mix_FreeChunk(startSound);
    Mix_FreeChunk(explosionSound);

    // <<< THÊM MỚI: Đặt các con trỏ texture cấp độ về nullptr >>>
    renderer = nullptr;
    window = nullptr;
    font = nullptr;
    playerTextureLvl1 = nullptr;
    playerTextureLvl2 = nullptr;
    playerTextureLvl3 = nullptr;
    playerTextureLvl4 = nullptr;
    bulletTextureLvl1 = nullptr;
    bulletTextureLvl2 = nullptr;
    bulletTextureLvl3 = nullptr;
    bulletTextureLvl4 = nullptr;
    enemyTexture = nullptr;
    //enemyBulletTexture = nullptr; // <<< SỬA ĐỔI
    backgroundTexture = nullptr;
    gameBackgroundTexture = nullptr;
    gameOverBackgroundTexture = nullptr;
    enemyTextureStraight = nullptr;
    enemyTextureWeave = nullptr;
    enemyTextureTank = nullptr;
    // <<< THÊM MỚI: Đặt con trỏ texture đạn địch riêng về nullptr >>>
    normalBulletTexture = nullptr;
    straightShooterBulletTexture = nullptr;
    tankBulletTexture = nullptr;
    weaverBulletTexture = nullptr;
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
    // playerTexture = loadTexture("player.png"); // <<< Xóa hoặc comment
    enemyTexture = loadTexture("enemy.png");
    // bulletTexture = loadTexture("bullet.png"); // <<< Xóa hoặc comment
    // <<< THÊM MỚI: Tải texture đạn địch riêng >>>
    // Thay thế "..." bằng tên file ảnh thực tế của bạn
    normalBulletTexture = loadTexture("normal_bullet.png");
    straightShooterBulletTexture = loadTexture("straight_shooter_bullet.png");
    tankBulletTexture = loadTexture("tank_bullet.png");
    weaverBulletTexture = loadTexture("weaver_bullet.png");
    enemyTextureStraight = loadTexture("enemy_straight.png");
    enemyTextureWeave = loadTexture("enemy_weave.png");
    enemyTextureTank = loadTexture("enemy_tank.png");
    bgMusic = Mix_LoadMUS("background.mp3");
    gameMusic = Mix_LoadMUS("game_music.mp3");
    startSound = Mix_LoadWAV("start_sound.wav");
    explosionSound = Mix_LoadWAV("explosion.wav");

    // <<< THÊM MỚI: Tải texture cho các cấp độ >>>
    playerTextureLvl1 = loadTexture("player_lvl1.png");
    playerTextureLvl2 = loadTexture("player_lvl2.png");
    playerTextureLvl3 = loadTexture("player_lvl3.png");
    playerTextureLvl4 = loadTexture("player_lvl4.png");
    bulletTextureLvl1 = loadTexture("bullet_lvl1.png");
    bulletTextureLvl2 = loadTexture("bullet_lvl2.png");
    bulletTextureLvl3 = loadTexture("bullet_lvl3.png");
    bulletTextureLvl4 = loadTexture("bullet_lvl4.png");

    // --- Check Resource Loading ---
    if (!backgroundTexture || !gameBackgroundTexture || !gameOverBackgroundTexture ||
        !playerTextureLvl1 || !playerTextureLvl2 || !playerTextureLvl3 || !playerTextureLvl4 ||
        !bulletTextureLvl1 || !bulletTextureLvl2 || !bulletTextureLvl3 || !bulletTextureLvl4 ||
        !enemyTexture || !enemyTextureStraight || !enemyTextureWeave || !enemyTextureTank ||
        // Kiểm tra texture đạn mới
        !normalBulletTexture || !straightShooterBulletTexture || !tankBulletTexture ||
        !weaverBulletTexture ||
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
    // <<< SỬA ĐỔI: Khởi tạo với kích thước Level 1 >>>
    player.rect.w = PLAYER_WIDTH_LVL1;
    player.rect.h = PLAYER_HEIGHT_LVL1;
    player.rect.x = SCREEN_WIDTH / 2 - player.rect.w / 2; // Căn giữa theo chiều rộng Lvl1
    player.rect.y = SCREEN_HEIGHT - 100 - player.rect.h; // Đặt y để đáy cách lề dưới 100px
    player.texture = playerTextureLvl1;
    player.xPos = (float)player.rect.x;
    player.yPos = (float)player.rect.y;
    player.health = PLAYER_INITIAL_HEALTH;
    player.maxHealth = PLAYER_INITIAL_HEALTH;
    player.bulletDamage = PLAYER_BULLET_DAMAGE;
    player.lastShotTime = 0;
    player.level = 1;

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
                                if (i == 0) { // Start New Game hoặc Play Again (trong GAME_OVER)
                                    currentScore = 0;
                                    enemies.clear(); bullets.clear(); enemyBullets.clear();
                                    currentEnemySpeed = INITIAL_ENEMY_SPEED;

                                    // <<< SỬA ĐỔI: Reset kích thước và vị trí về Level 1 >>>
                                    player.rect.w = PLAYER_WIDTH_LVL1;
                                    player.rect.h = PLAYER_HEIGHT_LVL1;
                                    player.rect.x = SCREEN_WIDTH / 2 - player.rect.w / 2;
                                    player.rect.y = SCREEN_HEIGHT - 100 - player.rect.h; // Đặt lại vị trí y
                                    player.xPos = (float)player.rect.x; // Đồng bộ lại xPos/yPos
                                    player.yPos = (float)player.rect.y;

                                    player.lastShotTime = 0;
                                    lastEnemySpawnTime = SDL_GetTicks();
                                    player.health = player.maxHealth;
                                    player.level = 1;                      // Reset level
                                    player.texture = playerTextureLvl1;    // Reset texture

                                    // player.bulletDamage = PLAYER_BULLET_DAMAGE; // Reset damage nếu có thay đổi

                                    Mix_HaltMusic();
                                    if (musicOn && gameMusic) { Mix_PlayMusic(gameMusic, -1); }
                                    isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                                    state = GAME;
                                    lastFrameTime = SDL_GetTicks();
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

                                    // <<< SỬA ĐỔI: Reset kích thước và vị trí về Level 1 (GIỐNG NHƯ START NEW GAME) >>>
                                    player.rect.w = PLAYER_WIDTH_LVL1;
                                    player.rect.h = PLAYER_HEIGHT_LVL1;
                                    player.rect.x = SCREEN_WIDTH / 2 - player.rect.w / 2;
                                    player.rect.y = SCREEN_HEIGHT - 100 - player.rect.h; // Đặt lại vị trí y
                                    player.xPos = (float)player.rect.x; // Đồng bộ lại xPos/yPos
                                    player.yPos = (float)player.rect.y;

                                    player.lastShotTime = 0;
                                    lastEnemySpawnTime = SDL_GetTicks();
                                    player.health = player.maxHealth;
                                    player.level = 1;                      // Reset level
                                    player.texture = playerTextureLvl1;    // Reset texture

                                    // player.bulletDamage = PLAYER_BULLET_DAMAGE; // Reset damage nếu có thay đổi

                                    isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                                    Mix_HaltMusic();
                                    if (musicOn && gameMusic) { Mix_PlayMusic(gameMusic, -1); }
                                    state = GAME;
                                    lastFrameTime = SDL_GetTicks();
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

                // <<< THÊM MỚI: Xác định kích thước đạn theo level >>>
                int bulletW = PLAYER_BULLET_WIDTH_LVL1; // Mặc định là Lvl 1
                int bulletH = PLAYER_BULLET_HEIGHT_LVL1;

                // Chọn texture VÀ cập nhật kích thước đạn dựa trên cấp độ
                switch (player.level) {
                case 1:
                    newBullet.texture = bulletTextureLvl1;
                    // bulletW, bulletH giữ nguyên giá trị Lvl1
                    break;
                case 2:
                    newBullet.texture = bulletTextureLvl2;
                    bulletW = PLAYER_BULLET_WIDTH_LVL2; // <<< Lấy kích thước Lvl 2
                    bulletH = PLAYER_BULLET_HEIGHT_LVL2;
                    break;
                case 3:
                    newBullet.texture = bulletTextureLvl3;
                    bulletW = PLAYER_BULLET_WIDTH_LVL3; // <<< Lấy kích thước Lvl 3
                    bulletH = PLAYER_BULLET_HEIGHT_LVL3;
                    break;
                case 4:
                    newBullet.texture = bulletTextureLvl4;
                    bulletW = PLAYER_BULLET_WIDTH_LVL4; // <<< Lấy kích thước Lvl 4
                    bulletH = PLAYER_BULLET_HEIGHT_LVL4;
                    break;
                default: // Trường hợp dự phòng
                    newBullet.texture = bulletTextureLvl1;
                    // bulletW, bulletH giữ nguyên giá trị Lvl1
                }

                // <<< SỬA ĐỔI: Tính toán vị trí bắn dùng kích thước đạn hiện tại (bulletW) >>>
                float bulletStartX = player.xPos + player.rect.w / 2.0f - bulletW / 2.0f; // Căn giữa đạn theo chiều rộng mới
                float bulletStartY = player.yPos; // Bắn từ đỉnh máy bay

                // <<< SỬA ĐỔI: Tạo rect với kích thước đạn mới (bulletW, bulletH) >>>
                newBullet.rect = { (int)bulletStartX, (int)bulletStartY, bulletW, bulletH };

                newBullet.xPos = bulletStartX; // Đồng bộ xPos/yPos
                newBullet.yPos = bulletStartY;
                newBullet.damage = player.bulletDamage;
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
        // --- 1. Khai báo màu ---
        SDL_Color white = { 255, 255, 255, 255 };
        SDL_Color yellow = { 255, 255, 0, 255 };
        SDL_Color gray = { 0, 0, 0, 150 };

        // --- 2. Vẽ Nền theo State ---
        if (state == MENU) {
            if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        }
        else if (state == GAME_OVER) {
            if (gameOverBackgroundTexture) SDL_RenderCopy(renderer, gameOverBackgroundTexture, NULL, NULL);
        }
        else { // GAME or PAUSED
            // <<< VẼ NỀN TRƯỚC TIÊN >>>
            if (gameBackgroundTexture) SDL_RenderCopy(renderer, gameBackgroundTexture, NULL, NULL);
        }



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
        else { // GAME or PAUSED - <<< Chỉ giữ lại khối này cho GAME/PAUSED >>>
            // <<< SAU ĐÓ VẼ CÁC ENTITY LÊN TRÊN NỀN >>>
            if (player.texture) SDL_RenderCopy(renderer, player.texture, NULL, &player.rect);
            for (auto& enemy : enemies) if (enemy.texture) SDL_RenderCopy(renderer, enemy.texture, NULL, &enemy.rect);
            for (auto& bullet : bullets) if (bullet.texture) SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);
            for (auto& bullet : enemyBullets) if (bullet.texture) SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);

            // <<< CUỐI CÙNG VẼ UI (SCORE, LEVEL, HEALTH, BUTTONS) LÊN TRÊN CÙNG >>>
            renderText("Score: " + std::to_string(currentScore), 10, 10, white);
            renderText("Level: " + std::to_string(player.level), 10, 70, white);


            int healthBarX = 10, healthBarY = 40, healthBarW = 200, healthBarH = 20;
            float healthPercent = (player.health > 0) ? (float)player.health / player.maxHealth : 0.0f;
            int currentHealthBarW = (int)(healthBarW * healthPercent);
            if (currentHealthBarW < 0) currentHealthBarW = 0;
            SDL_Rect healthBarBgRect = { healthBarX, healthBarY, healthBarW, healthBarH };
            SDL_SetRenderDrawColor(renderer, 100, 0, 0, 255); SDL_RenderFillRect(renderer, &healthBarBgRect);
            SDL_Rect currentHealthRect = { healthBarX, healthBarY, currentHealthBarW, healthBarH };
            SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255); SDL_RenderFillRect(renderer, &currentHealthRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); SDL_RenderDrawRect(renderer, &healthBarBgRect);

            renderButton(pauseButton); // Nút Pause cũng nên vẽ trên cùng

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