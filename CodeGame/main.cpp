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
#include <string>    // Cho std::string và std::to_string
#include <fstream>  // Cho việc đọc/ghi file (lưu điểm)
#include <sstream>  // Có thể dùng để định dạng chuỗi (tùy chọn)
#include <limits>   // Dùng khi đọc file điểm

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
// -------------------------------------------------------------

Uint32 lastEnemySpawnTime = 0;
float currentEnemySpeed = INITIAL_ENEMY_SPEED;

// Gần đầu file
enum GameState { MENU, GAME, PAUSED, GAME_OVER };
GameState state = MENU;

// --- Structs ---
struct Button {
    SDL_Rect rect;
    std::string text;
    bool hovered;
};

struct Entity {
    SDL_Rect rect{};
    SDL_Texture* texture = nullptr;
    float xPos = 0.0f;
    float yPos = 0.0f;
    float speedX = 0.0f;
    float speedY = 0.0f;
    int enemyStopY = 0;
    Uint32 lastShotTime = 0;
    Uint32 fireCooldown = 1500; // Cooldown bắn mặc định cho địch
    // bool toBeDeleted = false; // Không cần nữa vì xóa trực tiếp
};
// --- Nút Menu (Đã căn giữa X) ---
const int centeredButtonX = SCREEN_WIDTH / 2 - 280 / 2; // = 400
Button buttons[] = {
    {{centeredButtonX, 200, 280, 70}, "Start New Game", false},
    {{centeredButtonX, 290, 280, 70}, "Continue Game", false},
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
    {SCREEN_WIDTH - PAUSE_BUTTON_WIDTH - 10, 10, PAUSE_BUTTON_WIDTH, PAUSE_BUTTON_HEIGHT}, // Vị trí góc trên phải, có lề 10px
    "Pause",
    false
};

// --- Global Variables ---
SDL_Texture* playerTexture = nullptr;
SDL_Texture* enemyTexture = nullptr;
SDL_Texture* bulletTexture = nullptr; // <<< Sửa: Chỉ cần một texture đạn
SDL_Texture* enemyBulletTexture = nullptr; // <<< Thêm: Texture đạn địch (nếu muốn khác)
std::vector<Entity> enemies;
std::vector<Entity> bullets;
std::vector<Entity> enemyBullets;
Entity player;

bool musicOn = true;
Mix_Music* bgMusic = nullptr;
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
bool running = true; // <<< Global trở lại (nếu cần, hoặc để trong main)
bool isPlayerDragging = false;
bool isMovingToTarget = false; // <<< THÊM MỚI: Cờ báo đang tự động di chuyển
float targetMoveX = 0.0f;      // <<< THÊM MỚI: Tọa độ X đích
float targetMoveY = 0.0f;      // <<< THÊM MỚI: Tọa độ Y đích
bool isFollowingMouse = false;



// --- Function Declarations ---
SDL_Texture* loadTexture(const char* path); // <<< Bỏ renderer
void saveHighScore();
void loadHighScore();
void renderText(const std::string& text, int x, int y, SDL_Color color); // <<< Bỏ renderer, font
void renderButton(Button& button); // <<< Bỏ renderer, font
void spawnEnemy();
void enemyShoot();
void updateGame(float deltaTime); // <<< Dùng phiên bản gốc đã sửa
void cleanup(); // <<< Bỏ renderer, font
// void resetGame(); // <<< Bỏ đi
// void handleInput(...); // <<< Bỏ đi
// void update(...); // <<< Bỏ đi
// void render(...); // <<< Bỏ đi

// --- Function Definitions ---

SDL_Texture* loadTexture(const char* path) { // <<< Bỏ renderer
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cerr << "Failed to load image: " << path << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); // <<< Dùng renderer global
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
    if (!font) { std::cerr << "Font not loaded in renderText!\n"; return; }
    if (text.empty()) { return; }

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
    if (!font) { std::cerr << "Font not loaded in renderButton!\n"; return; }

    SDL_Color color = button.hovered ? SDL_Color{ 150, 150, 150, 255 } : SDL_Color{ 100, 100, 100, 255 };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &button.rect);

    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, button.text.c_str(), textColor);
    if (!textSurface) { std::cerr << "Failed to render button text surface: " << TTF_GetError() << std::endl; return; }

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
    currentEnemySpeed += ENEMY_SPEED_INCREMENT; // Tăng tốc độ mỗi lần spawn (có thể điều chỉnh)
    int stopY = rand() % 200 + 100; // Vị trí dừng ngẫu nhiên trên màn hình

    Entity newEnemy;
    newEnemy.rect = { rand() % (SCREEN_WIDTH - ENEMY_WIDTH), -ENEMY_HEIGHT, ENEMY_WIDTH, ENEMY_HEIGHT };
    newEnemy.texture = enemyTexture;
    newEnemy.xPos = (float)newEnemy.rect.x;
    newEnemy.yPos = (float)newEnemy.rect.y;
    newEnemy.speedY = currentEnemySpeed; // Dùng tốc độ hiện tại
    newEnemy.speedX = 0; // Địch chỉ di chuyển xuống
    newEnemy.enemyStopY = stopY;
    newEnemy.lastShotTime = SDL_GetTicks() + (rand() % 1000); // Thêm độ trễ ban đầu ngẫu nhiên trước khi bắn
    newEnemy.fireCooldown = static_cast<Uint32>(1000 + (rand() % 2000)); // Cooldown bắn ngẫu nhiên

    enemies.push_back(newEnemy);
}

void enemyShoot() {
    Uint32 currentTime = SDL_GetTicks();
    for (auto& enemy : enemies) {
        // Chỉ bắn khi đã dừng và đủ cooldown
        if (enemy.yPos >= enemy.enemyStopY && currentTime > enemy.lastShotTime + enemy.fireCooldown) {
            float startX = enemy.xPos + enemy.rect.w / 2.0f;
            float startY = enemy.yPos + enemy.rect.h;
            float targetX = player.xPos + player.rect.w / 2.0f; // Dùng xPos/yPos để có vị trí chính xác hơn
            float targetY = player.yPos + player.rect.h / 2.0f;
            float dirX = targetX - startX;
            float dirY = targetY - startY;
            float length = std::sqrt(dirX * dirX + dirY * dirY);

            float bulletSpeedX = 0.0f;
            float bulletSpeedY = ENEMY_BULLET_SPEED; // Mặc định bắn thẳng xuống nếu không tính được

            if (length > 0.0001f) { // Tránh chia cho 0
                float invLength = 1.0f / length;
                dirX *= invLength;
                dirY *= invLength;
                bulletSpeedX = dirX * ENEMY_BULLET_SPEED;
                bulletSpeedY = dirY * ENEMY_BULLET_SPEED;
            }

            Entity enemyBullet;
            // Đặt texture đạn địch (có thể dùng chung bulletTexture nếu muốn)
            enemyBullet.texture = enemyBulletTexture ? enemyBulletTexture : bulletTexture;
            enemyBullet.rect = { (int)(startX - ENEMY_BULLET_WIDTH / 2.0f), (int)startY, ENEMY_BULLET_WIDTH, ENEMY_BULLET_HEIGHT };
            enemyBullet.xPos = (float)enemyBullet.rect.x;
            enemyBullet.yPos = (float)startY;
            enemyBullet.speedX = bulletSpeedX;
            enemyBullet.speedY = bulletSpeedY;

            enemyBullets.push_back(enemyBullet);
            enemy.lastShotTime = currentTime;
            enemy.fireCooldown = static_cast<Uint32>(1000 + (rand() % 2500)); // Random cooldown mới
        }
    }
}

// <<< Phiên bản updateGame gốc đã sửa lỗi (từ prompt #1) >>>
void updateGame(float deltaTime) {

    // --- Cập nhật & Xóa Đạn Người Chơi (Ngoài màn hình) ---
    for (int i = bullets.size() - 1; i >= 0; --i) { // Duyệt ngược
        bullets[i].yPos -= PLAYER_BULLET_SPEED * deltaTime;
        bullets[i].rect.y = (int)bullets[i].yPos;
        bullets[i].xPos = (float)bullets[i].rect.x; // Giữ xPos đồng bộ

        if (bullets[i].rect.y + bullets[i].rect.h < 0) { // Ngoài cạnh trên
            bullets.erase(bullets.begin() + i); // Xóa ngay
        }
    }

    // --- Cập nhật & Xóa Kẻ Địch (Ngoài màn hình) ---
    for (int i = enemies.size() - 1; i >= 0; --i) { // Duyệt ngược
        // Cập nhật vị trí
        if (enemies[i].yPos < enemies[i].enemyStopY) {
            enemies[i].yPos += enemies[i].speedY * deltaTime;
            if (enemies[i].yPos > enemies[i].enemyStopY) {
                enemies[i].yPos = (float)enemies[i].enemyStopY;
            }
            enemies[i].rect.y = (int)enemies[i].yPos;
            enemies[i].xPos = (float)enemies[i].rect.x; // Giữ xPos đồng bộ
        }

        // Xóa nếu ra khỏi cạnh dưới
        if (enemies[i].rect.y > SCREEN_HEIGHT) {
            enemies.erase(enemies.begin() + i); // Xóa ngay
        }
    }

    // --- Va chạm: Đạn Người Chơi <-> Kẻ Địch (Xóa tức thì) ---
    for (int i = bullets.size() - 1; i >= 0; --i) { // Duyệt ngược đạn
        bool bulletRemoved = false; // Đánh dấu nếu đạn đã bị xóa trong vòng lặp trong
        for (int j = enemies.size() - 1; j >= 0; --j) { // Duyệt ngược địch
            // Kiểm tra va chạm
            if (SDL_HasIntersection(&bullets[i].rect, &enemies[j].rect)) {
                enemies.erase(enemies.begin() + j); // Xóa địch ngay
                bullets.erase(bullets.begin() + i); // Xóa đạn ngay
                currentScore += 10;

                // Phát âm thanh nổ
                if (explosionSound) {
                    Mix_PlayChannel(-1, explosionSound, 0);
                }

                bulletRemoved = true; // Đánh dấu đạn đã xóa
                break; // Thoát vòng lặp địch, vì đạn này đã trúng và bị xóa
            }
        }
        // if (bulletRemoved) continue; // Không cần thiết khi duyệt ngược
    }

    // --- Cập nhật, Va chạm & Xóa Đạn Địch ---
    bool playerIsHit = false;
    for (int i = enemyBullets.size() - 1; i >= 0; --i) { // Duyệt ngược đạn địch
        // Cập nhật vị trí đạn địch TRƯỚC
        enemyBullets[i].xPos += enemyBullets[i].speedX * deltaTime;
        enemyBullets[i].yPos += enemyBullets[i].speedY * deltaTime;
        enemyBullets[i].rect.x = (int)enemyBullets[i].xPos;
        enemyBullets[i].rect.y = (int)enemyBullets[i].yPos;

        // Kiểm tra va chạm với người chơi
        if (!playerIsHit && SDL_HasIntersection(&enemyBullets[i].rect, &player.rect)) {
            playerIsHit = true;
            // Không break vội, vẫn cần kiểm tra off-screen cho các viên đạn khác
        }

        // Kiểm tra và xóa nếu ra ngoài màn hình
        if (enemyBullets[i].rect.x + enemyBullets[i].rect.w < 0 || enemyBullets[i].rect.x > SCREEN_WIDTH ||
            enemyBullets[i].rect.y + enemyBullets[i].rect.h < 0 || enemyBullets[i].rect.y > SCREEN_HEIGHT)
        {
            enemyBullets.erase(enemyBullets.begin() + i); // Xóa ngay
        }
    }

    // --- Xử lý khi người chơi bị bắn trúng ---
    if (playerIsHit) {
        std::cout << "Player hit! Game Over! Final Score: " << currentScore << std::endl;
        if (currentScore > highScore) {
            std::cout << "New High Score: " << currentScore << std::endl;
            highScore = currentScore;
            saveHighScore();
        }
        isPlayerDragging = false;
        isMovingToTarget = false; // <<< THÊM DÒNG NÀY VÀO ĐÂY
        isFollowingMouse = false;
        state = GAME_OVER;
        return; // Thoát updateGame sớm
    }
    // Không còn vòng lặp dọn dẹp ở cuối nữa
}

void cleanup() { // <<< Bỏ renderer, font
    if (renderer) SDL_DestroyRenderer(renderer); // <<< Dùng renderer global
    if (window) SDL_DestroyWindow(window);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(gameBackgroundTexture);
    SDL_DestroyTexture(gameOverBackgroundTexture);
    if (font) TTF_CloseFont(font); // <<< Dùng font global
    Mix_FreeMusic(bgMusic);
    Mix_FreeChunk(startSound);
    Mix_FreeChunk(explosionSound);

    // Reset con trỏ về null
    renderer = nullptr;
    window = nullptr;
    font = nullptr;
    playerTexture = nullptr;
    enemyTexture = nullptr;
    bulletTexture = nullptr;
    backgroundTexture = nullptr;
    gameBackgroundTexture = nullptr;
    gameOverBackgroundTexture = nullptr;
    bgMusic = nullptr;
    startSound = nullptr;
    explosionSound = nullptr;
}

//// --- Thêm các biến toàn cục mới gần các biến toàn cục khác ---
//bool isPlayerDragging = false;
//bool isMovingToTarget = false; // Cờ báo đang tự động di chuyển
//float targetMoveX = 0.0f;      // Tọa độ X đích
//float targetMoveY = 0.0f;      // Tọa độ Y đích

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
    // enemyBulletTexture = loadTexture("enemy_bullet.png"); // Nếu có texture đạn địch riêng
    bgMusic = Mix_LoadMUS("background.mp3");
    startSound = Mix_LoadWAV("start_sound.wav"); // Âm thanh này hiện không dùng, có thể xóa nếu muốn
    explosionSound = Mix_LoadWAV("explosion.wav");

    // --- Check Resource Loading ---
    if (!backgroundTexture || !gameBackgroundTexture || !gameOverBackgroundTexture ||
        !playerTexture || !enemyTexture || !bulletTexture ||
        !bgMusic || !explosionSound /* || !startSound nếu bạn giữ lại */) {
        std::cerr << "Failed to load one or more resources!" << std::endl;
        cleanup(); // Gọi cleanup đã sửa
        Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit();
        return -1;
    }

    if (musicOn && bgMusic) { Mix_PlayMusic(bgMusic, -1); }

    // --- Initialize Player ---
    player.rect = { SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT - 100, PLAYER_WIDTH, PLAYER_HEIGHT };
    player.texture = playerTexture;
    player.xPos = (float)player.rect.x;
    player.yPos = (float)player.rect.y;
    player.lastShotTime = 0; // Khởi tạo thời gian bắn cho auto-fire

    // --- Game Loop Variables ---
    SDL_Event event;
    Uint32 lastFrameTime = SDL_GetTicks();
    running = true;


    // --- Main Game Loop ---
    while (running) {
        // --- Delta Time ---
        Uint32 currentFrameTime = SDL_GetTicks();
        Uint32 frameTicks = SDL_TICKS_PASSED(currentFrameTime, lastFrameTime);

        // <<< KHAI BÁO deltaTime Ở ĐÂY >>>
        float deltaTime = 0.0f; // Khai báo và khởi tạo giá trị mặc định

        // <<< QUAN TRỌNG: Chỉ cập nhật lastFrameTime nếu game KHÔNG bị PAUSE >>>
        // Nếu không, deltaTime sẽ tính cả thời gian pause khi resume
        if (state != PAUSED) {
            deltaTime = frameTicks / 1000.0f;
            lastFrameTime = currentFrameTime; // Cập nhật chỉ khi không pause
            if (deltaTime > 0.1f) { deltaTime = 0.1f; } // Giới hạn delta time
        }
        else {
            deltaTime = 0.0f; // Khi pause, không có thời gian trôi qua trong game
            // Không cập nhật lastFrameTime ở đây
        }
        // --- Input ---
        // Lấy trạng thái chuột MỘT LẦN cho cả frame (dùng cho kéo thả, hover)
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        // Xử lý hàng đợi sự kiện
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            // --- Xử lý nhấn chuột ---
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Lấy tọa độ chính xác tại thời điểm click cho logic click
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
                                    player.lastShotTime = 0; // Reset thời gian bắn
                                    lastEnemySpawnTime = SDL_GetTicks();
                                    if (Mix_PlayingMusic()) Mix_HaltMusic(); // Tắt nhạc menu (nếu đang bật)
                                    isPlayerDragging = false;   // Reset kéo
                                    isMovingToTarget = false; // Reset di chuyển tự động
                                    isFollowingMouse = false;
                                    state = GAME;             // Chuyển sang chơi
                                }
                                else if (i == 1) { // Continue
                                    std::cout << "Continue button clicked (Not implemented)" << std::endl;
                                }
                                else if (i == 2) { // Music On/Off
                                    musicOn = !musicOn;
                                    buttons[i].text = musicOn ? "Music: On" : "Music: Off";
                                    if (musicOn) {
                                        if (Mix_PausedMusic()) Mix_ResumeMusic();
                                        else if (bgMusic && !Mix_PlayingMusic()) Mix_PlayMusic(bgMusic, -1);
                                    }
                                    else if (Mix_PlayingMusic()) {
                                        Mix_PauseMusic();
                                    }
                                }
                                else if (i == 3) { // Exit Game
                                    running = false;
                                }
                                break; // Thoát vòng lặp nút khi đã click
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
                                    player.lastShotTime = 0; // Reset thời gian bắn
                                    lastEnemySpawnTime = SDL_GetTicks();
                                    // Không cần xử lý nhạc ở đây, vì không có nhạc game over
                                    isPlayerDragging = false;   // Reset kéo
                                    isMovingToTarget = false; // Reset di chuyển tự động
                                    isFollowingMouse = false;
                                    state = GAME;             // Chuyển sang chơi
                                }
                                else if (i == 1) { // Main Menu
                                    isPlayerDragging = false;   // Reset kéo
                                    isMovingToTarget = false; // Reset di chuyển tự động
                                    isFollowingMouse = false;
                                    state = MENU;             // Về menu
                                    // Bật lại nhạc nền nếu cần
                                    if (bgMusic && musicOn && !Mix_PlayingMusic()) {
                                        Mix_PlayMusic(bgMusic, -1);
                                    }
                                }
                                break; // Thoát vòng lặp nút khi đã click
                            }
                        }
                    }
                    else if (state == GAME || state == PAUSED) { // <<< THAY ĐỔI: Cho phép click Pause khi đang Game hoặc Paused
                        // KIỂM TRA CLICK NÚT PAUSE TRƯỚC
                        if (SDL_PointInRect(&mousePoint, &pauseButton.rect)) {
                            if (state == GAME) {
                                state = PAUSED;
                                pauseButton.text = "Resume"; // Đổi chữ nút (tùy chọn)
                                // Quan trọng: Không cần reset lastFrameTime khi pause
                            }
                            else { // state == PAUSED
                                state = GAME;
                                pauseButton.text = "Pause"; // Đổi chữ nút lại (tùy chọn)
                                // <<< QUAN TRỌNG: Đặt lại lastFrameTime NGAY KHI RESUME >>>
                                // Để tránh deltaTime lớn ở frame đầu tiên sau khi resume
                                lastFrameTime = SDL_GetTicks();
                            }
                        }
                        // Nếu không click vào nút Pause VÀ đang ở trạng thái GAME, mới xử lý click player/di chuyển
                        else if (state == GAME) {
                            // TẮT trạng thái 'following' khi có click mới (nếu không phải click nút pause)
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
                    } // end left click
                } // end mouse down
            }
            // --- Xử lý nhả chuột ---
            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Chỉ xử lý nhả chuột liên quan đến player nếu đang chơi
                    if (state == GAME) { // <<< THÊM KIỂM TRA STATE
                        if (isPlayerDragging) {
                            isPlayerDragging = false;
                            isFollowingMouse = false; // Có thể cần hoặc không tùy logic mong muốn sau khi kéo
                        }
                        // Không cần làm gì với isMovingToTarget khi nhả chuột
                    }
                }
            } // end mouse up
        } // end poll event

        // --- Update --- //

        if (state == GAME) {
            // --- Cập nhật vị trí người chơi ---
            if (isPlayerDragging) {
                // 1. Kéo thả: Theo chuột ngay lập tức
                float targetX = (float)mouseX - player.rect.w / 2.0f;
                float targetY = (float)mouseY - player.rect.h / 2.0f;
                player.xPos = targetX;
                player.yPos = targetY;
            }
            else if (isMovingToTarget) {
                // 2. Nếu không kéo, kiểm tra Di chuyển tự động
                float dirX = targetMoveX - player.xPos;
                float dirY = targetMoveY - player.yPos;
                float dist = std::sqrt(dirX * dirX + dirY * dirY);

                bool reachedTarget = false; // Cờ tạm để biết đã đến đích chưa

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

                // KIỂM TRA NẾU ĐÃ ĐẾN ĐÍCH TRONG FRAME NÀY
                if (reachedTarget) {
                    player.xPos = targetMoveX; // Snap vào đích
                    player.yPos = targetMoveY;
                    isMovingToTarget = false; // Dừng di chuyển tự động
                    isFollowingMouse = true;  // <<< BẮT ĐẦU ĐI THEO CHUỘT >>>
                }

            }
            else if (isFollowingMouse) {
                // 3. Nếu không kéo và không tự đi, kiểm tra Đi theo chuột
                // Logic giống hệt kéo thả (đặt vị trí theo chuột)
                float targetX = (float)mouseX - player.rect.w / 2.0f;
                float targetY = (float)mouseY - player.rect.h / 2.0f;
                player.xPos = targetX;
                player.yPos = targetY;
            }
            // 4. Nếu cả 3 cờ đều false -> Đứng yên

            // Áp dụng giới hạn màn hình và cập nhật rect sau khi vị trí được xác định
            player.xPos = std::max(0.0f, std::min((float)SCREEN_WIDTH - player.rect.w, player.xPos));
            player.yPos = std::max(0.0f, std::min((float)SCREEN_HEIGHT - player.rect.h, player.yPos));
            player.rect.x = (int)player.xPos;
            player.rect.y = (int)player.yPos;

            // --- Bắn tự động ---
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime > player.lastShotTime + PLAYER_AUTO_FIRE_COOLDOWN) {
                Entity newBullet;
                newBullet.rect = { player.rect.x + player.rect.w / 2 - PLAYER_BULLET_WIDTH / 2, player.rect.y, PLAYER_BULLET_WIDTH, PLAYER_BULLET_HEIGHT };
                newBullet.texture = bulletTexture;
                newBullet.xPos = (float)newBullet.rect.x;
                newBullet.yPos = (float)player.rect.y;
                bullets.push_back(newBullet);
                player.lastShotTime = currentTime;
            }

            // --- Cập nhật logic game khác ---
            // Spawn kẻ địch
            // (Lưu ý: Có thể dùng lại biến currentTime ở trên thay vì gọi lại SDL_GetTicks())
            if (currentTime > lastEnemySpawnTime + ENEMY_SPAWN_INTERVAL) {
                spawnEnemy();
                lastEnemySpawnTime = currentTime;
            }
            // Kẻ địch bắn
            enemyShoot();
            // Cập nhật va chạm, di chuyển đạn/kẻ địch,... và kiểm tra game over
            updateGame(deltaTime); // updateGame sẽ tự đổi state nếu game over

        }
        else { // Nếu không phải state GAME (là MENU hoặc GAME_OVER)
            // Đảm bảo reset các trạng thái điều khiển
            isPlayerDragging = false;
            isMovingToTarget = false;
            isFollowingMouse = false; // <<< Reset thêm cờ này
        }

        // --- Cập nhật trạng thái hover của nút (cho MENU và GAME_OVER) ---
        // (Phần này có thể đặt trong if state==MENU hoặc if state==GAME_OVER nếu muốn tối ưu chút)
        if (state == MENU) {
            for (int i = 0; i < BUTTON_COUNT; i++) {
                buttons[i].hovered = (mouseX >= buttons[i].rect.x && mouseX <= buttons[i].rect.x + buttons[i].rect.w &&
                    mouseY >= buttons[i].rect.y && mouseY <= buttons[i].rect.y + buttons[i].rect.h);
            }
        }
        else if (state == GAME_OVER) {
            for (int i = 0; i < GAMEOVER_BUTTON_COUNT; i++) {
                gameOverButtons[i].hovered = (mouseX >= gameOverButtons[i].rect.x && mouseX <= gameOverButtons[i].rect.x + gameOverButtons[i].rect.w &&
                    mouseY >= gameOverButtons[i].rect.y && mouseY <= gameOverButtons[i].rect.y + gameOverButtons[i].rect.h);
            }
        }
        else if (state == GAME || state == PAUSED) { // <<< THAY ĐỔI: Cập nhật hover nút Pause khi GAME hoặc PAUSED
            pauseButton.hovered = (mouseX >= pauseButton.rect.x && mouseX <= pauseButton.rect.x + pauseButton.rect.w &&
                mouseY >= pauseButton.rect.y && mouseY <= pauseButton.rect.y + pauseButton.rect.h);
        }


        // --- Render --- //
        SDL_RenderClear(renderer);

        // Vẽ nền theo state
        if (state == MENU) {
            if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        }
        else if (state == GAME_OVER) {
            if (gameOverBackgroundTexture) SDL_RenderCopy(renderer, gameOverBackgroundTexture, NULL, NULL);
        }
        else { // state == GAME or state == PAUSED
            if (gameBackgroundTexture) SDL_RenderCopy(renderer, gameBackgroundTexture, NULL, NULL);
        }

        SDL_Color white = { 255, 255, 255, 255 };
        SDL_Color yellow = { 255, 255, 0, 255 };
        SDL_Color gray = { 128, 128, 128, 150 }; // Màu xám trong suốt cho overlay

        // Vẽ nội dung theo state
        if (state == MENU) {
            for (int i = 0; i < BUTTON_COUNT; i++) renderButton(buttons[i]);
            renderText("High Score: " + std::to_string(highScore), 10, 10, white);
        }
        else if (state == GAME_OVER) {
            int textW = 0, textH = 0; // Khởi tạo để tránh lỗi nếu font null
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
        else {
            // Vẽ các đối tượng game
            if (player.texture) SDL_RenderCopy(renderer, player.texture, NULL, &player.rect);
            for (auto& enemy : enemies) if (enemy.texture) SDL_RenderCopy(renderer, enemy.texture, NULL, &enemy.rect);
            for (auto& bullet : bullets) if (bullet.texture) SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);
            for (auto& bullet : enemyBullets) if (bullet.texture) SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);

            // Vẽ điểm
            renderText("Score: " + std::to_string(currentScore), 10, 10, white);

            // <<< THÊM MỚI: Vẽ nút Pause >>>
            renderButton(pauseButton);

            // <<< THÊM MỚI: Vẽ overlay và chữ "Paused" nếu đang pause >>>
            if (state == PAUSED) {
                // Vẽ một hình chữ nhật màu xám bán trong suốt phủ lên màn hình
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Bật chế độ blend
                SDL_SetRenderDrawColor(renderer, gray.r, gray.g, gray.b, gray.a);
                SDL_Rect pauseOverlayRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
                SDL_RenderFillRect(renderer, &pauseOverlayRect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); // Tắt chế độ blend

                // Vẽ chữ "Paused" ở giữa
                int textW = 0, textH = 0;
                std::string pauseMsg = "Paused";
                if (font) TTF_SizeText(font, pauseMsg.c_str(), &textW, &textH);
                renderText(pauseMsg, SCREEN_WIDTH / 2 - textW / 2, SCREEN_HEIGHT / 2 - textH / 2, yellow);
            }
        }
        SDL_RenderPresent(renderer); // Hiển thị tất cả lên màn hình

    } // <<< Kết thúc while(running)

    // --- Cleanup ---
    cleanup(); // Dọn dẹp texture, sound chunk, font, renderer, window
    // Dọn dẹp các subsystem SDL theo thứ tự ngược lại khởi tạo
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
} // <<< Kết thúc main