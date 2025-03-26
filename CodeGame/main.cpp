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
#include <string>   // <-- THÊM: Cho std::string và std::to_string
#include <fstream>  // <-- THÊM: Cho việc đọc/ghi file (lưu điểm)
#include <sstream>  // <-- THÊM: Có thể dùng để định dạng chuỗi (tùy chọn)
#include <limits>   // <-- THÊM: Dùng khi đọc file điểm

// Screen dimensions
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// Timing
const int ENEMY_SPAWN_INTERVAL = 1000;

// --- Game Parameters / Speeds (Values adjusted for deltaTime) ---
// !!! Adjust these values to fine-tune gameplay !!!
const float PLAYER_BULLET_SPEED = 6000.0f;
const float ENEMY_BULLET_SPEED = 4000.0f;
const float INITIAL_ENEMY_SPEED = 1000.0f;
const float ENEMY_SPEED_INCREMENT = 100.0f;
// -------------------------------------------------------------

Uint32 lastEnemySpawnTime = 0;
float currentEnemySpeed = INITIAL_ENEMY_SPEED;

enum GameState { MENU, GAME };
GameState state = MENU;

// --- Structs ---
struct Button {
    SDL_Rect rect;
    std::string text;
    bool hovered;
};

Button buttons[] = {
    {{500, 200, 280, 70}, "Start New Game", false},
    {{500, 290, 280, 70}, "Continue Game", false},
    {{500, 380, 280, 70}, "Music: On", false},
    {{500, 470, 280, 70}, "Exit Game", false}
};
const int BUTTON_COUNT = sizeof(buttons) / sizeof(Button);

struct Entity {
    SDL_Rect rect{};
    SDL_Texture* texture = nullptr;
    float xPos = 0.0f; // Added for diagonal movement
    float yPos = 0.0f;
    float speedX = 0.0f; // Added for diagonal movement
    float speedY = 0.0f; // Renamed from 'speed'
    int enemyStopY = 0;
    Uint32 lastShotTime = 0;
    Uint32 fireCooldown = 1500;
};

// --- Global Variables ---
SDL_Texture* playerTexture = nullptr;
SDL_Texture* enemyTexture = nullptr;
SDL_Texture* bulletTexture = nullptr;
std::vector<Entity> enemies;
std::vector<Entity> bullets;
std::vector<Entity> enemyBullets;
Entity player;

bool musicOn = true; // <-- GIỮ LẠI BỘ KHAI BÁO NÀY
Mix_Music* bgMusic = nullptr;
Mix_Chunk* startSound = nullptr;
Mix_Chunk* shootSound = nullptr;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* gameBackgroundTexture = nullptr;
TTF_Font* font = nullptr;// Make font global or pass it more carefully if needed outside main loop scope

// --- THÊM BIẾN ĐIỂM ---
int currentScore = 0;
int highScore = 0;
// ------------------------

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

// --- THÊM: Hàm lưu điểm cao nhất ---
void saveHighScore() {
    std::ofstream file("highscore.txt"); // Mở file để ghi
    if (file.is_open()) {
        file << highScore; // Ghi điểm cao nhất vào file
        file.close();
        std::cout << "Saved high score: " << highScore << std::endl;
    }
    else {
        std::cerr << "Error: Could not open highscore.txt for saving!" << std::endl;
    }
}

// --- THÊM: Hàm tải điểm cao nhất ---
void loadHighScore() {
    std::ifstream file("highscore.txt"); // Mở file để đọc
    if (file.is_open()) {
        // Thử đọc một số nguyên từ file vào highScore
        if (!(file >> highScore)) {
            std::cerr << "Warning: Could not read highscore from file or file is corrupted. Resetting to 0." << std::endl;
            highScore = 0; // Đặt lại là 0 nếu đọc lỗi
        }
        else {
            // Kiểm tra thêm nếu điểm đọc vào là số âm (không hợp lệ)
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
        highScore = 0; // Giá trị mặc định nếu không có file
    }
}

// --- THÊM: Hàm vẽ chữ ---
void renderText(const std::string& text, int x, int y, SDL_Color color) {
    if (!font) { // Kiểm tra xem font đã được load chưa
        std::cerr << "Error: Font not loaded for renderText!" << std::endl;
        return;
    }
    if (text.empty()) { // Không vẽ nếu text rỗng
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
        SDL_DestroyTexture(texture); // Giải phóng texture ngay sau khi vẽ
    }
    SDL_FreeSurface(surface); // Giải phóng surface
}
// ------------------------


void renderButton(Button& button) {
    if (!font) { std::cerr << "Error: Font is null in renderButton!" << std::endl; return; }

    SDL_Color color = button.hovered ? SDL_Color{ 150, 150, 150, 255 } : SDL_Color{ 100, 100, 100, 255 };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &button.rect);

    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, button.text.c_str(), textColor);
    if (!textSurface) { std::cerr << "TTF_RenderText_Blended Error: " << TTF_GetError() << std::endl; return; } // Thêm báo lỗi cụ thể

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) { std::cerr << "CreateTextureFromSurface Error: " << SDL_GetError() << std::endl; SDL_FreeSurface(textSurface); return; } // Thêm báo lỗi cụ thể

    SDL_Rect textRect = { button.rect.x + (button.rect.w - textSurface->w) / 2,
                          button.rect.y + (button.rect.h - textSurface->h) / 2,
                          textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void spawnEnemy() {
    currentEnemySpeed += ENEMY_SPEED_INCREMENT;
    int stopY = rand() % 200 + 100; // Stop between Y=100 and Y=299

    Entity newEnemy;
    newEnemy.rect = { rand() % (SCREEN_WIDTH - 50), -50, 50, 50 };
    newEnemy.texture = enemyTexture;
    newEnemy.xPos = (float)newEnemy.rect.x; // Init xPos
    newEnemy.yPos = -50.0f;
    newEnemy.speedY = currentEnemySpeed; // Use speedY
    newEnemy.speedX = 0; // Enemies move straight down initially
    newEnemy.enemyStopY = stopY;
    newEnemy.lastShotTime = 0;
    newEnemy.fireCooldown = static_cast<Uint32>(1000 + (rand() % 2000));

    enemies.push_back(newEnemy);
}

void enemyShoot() {
    Uint32 currentTime = SDL_GetTicks();
    for (auto& enemy : enemies) {
        if (enemy.yPos >= enemy.enemyStopY && currentTime > enemy.lastShotTime + enemy.fireCooldown) {
            float startX = (float)enemy.rect.x + enemy.rect.w / 2.0f;
            float startY = (float)enemy.rect.y + enemy.rect.h;
            float targetX = (float)player.rect.x + player.rect.w / 2.0f;
            float targetY = (float)player.rect.y + player.rect.h / 2.0f;
            float dirX = targetX - startX;
            float dirY = targetY - startY;
            float length = std::sqrt(dirX * dirX + dirY * dirY);

            float bulletSpeedX = 0.0f;
            float bulletSpeedY = ENEMY_BULLET_SPEED;

            if (length > 0.0001f) {
                float invLength = 1.0f / length;
                dirX *= invLength;
                dirY *= invLength;
                bulletSpeedX = dirX * ENEMY_BULLET_SPEED;
                bulletSpeedY = dirY * ENEMY_BULLET_SPEED;
            }

            Entity enemyBullet;
            enemyBullet.rect = { (int)(startX - 5.0f), (int)startY, 10, 20 };
            enemyBullet.texture = bulletTexture;
            enemyBullet.xPos = startX - 5.0f;
            enemyBullet.yPos = (float)startY;
            enemyBullet.speedX = bulletSpeedX;
            enemyBullet.speedY = bulletSpeedY;

            enemyBullets.push_back(enemyBullet);
            enemy.lastShotTime = currentTime;
            enemy.fireCooldown = static_cast<Uint32>(1000 + (rand() % 2500));
        }
    }
}

// *** THIS IS THE CORRECTED updateGame FUNCTION ***
// Hàm updateGame - PHIÊN BẢN THAY THẾ (Duyệt ngược, xóa tức thì)
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
            // Kiểm tra va chạm (không cần kiểm tra index vì duyệt ngược và break)
            if (SDL_HasIntersection(&bullets[i].rect, &enemies[j].rect)) {
                enemies.erase(enemies.begin() + j); // Xóa địch ngay
                bullets.erase(bullets.begin() + i); // Xóa đạn ngay
                currentScore += 10;                 // Cộng điểm
                bulletRemoved = true;               // Đánh dấu đạn đã xóa
                break; // Thoát vòng lặp địch, vì đạn này đã trúng và bị xóa
            }
        }
        // Nếu đạn đã bị xóa, không cần kiểm tra tiếp (vòng lặp ngoài sẽ giảm i)
        // if (bulletRemoved) continue; // Có thể thêm dòng này nếu muốn rõ ràng hơn
    }

    // --- Cập nhật, Va chạm & Xóa Đạn Địch ---
    bool playerIsHit = false;
    for (int i = enemyBullets.size() - 1; i >= 0; --i) { // Duyệt ngược đạn địch
        // Cập nhật vị trí đạn địch TRƯỚC
        enemyBullets[i].xPos += enemyBullets[i].speedX * deltaTime;
        enemyBullets[i].yPos += enemyBullets[i].speedY * deltaTime;
        enemyBullets[i].rect.x = (int)enemyBullets[i].xPos;
        enemyBullets[i].rect.y = (int)enemyBullets[i].yPos;

        // Kiểm tra va chạm với người chơi (chỉ phát hiện, không xóa ngay)
        if (!playerIsHit && SDL_HasIntersection(&enemyBullets[i].rect, &player.rect)) {
            playerIsHit = true;
            // Không break vội, vẫn cần cập nhật và kiểm tra off-screen cho các viên đạn khác
        }

        // Kiểm tra và xóa nếu ra ngoài màn hình
        if (enemyBullets[i].rect.x + enemyBullets[i].rect.w < 0 || enemyBullets[i].rect.x > SCREEN_WIDTH ||
            enemyBullets[i].rect.y + enemyBullets[i].rect.h < 0 || enemyBullets[i].rect.y > SCREEN_HEIGHT)
        {
            enemyBullets.erase(enemyBullets.begin() + i); // Xóa ngay
            // Không cần kiểm tra va chạm player nữa nếu đạn đã bị xóa
            // Tuy nhiên, playerIsHit đã được kiểm tra trước đó trong cùng vòng lặp nên không sao
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
        state = MENU;
        enemies.clear();
        bullets.clear();
        enemyBullets.clear(); // Xóa hết đạn địch khi thua
        currentEnemySpeed = INITIAL_ENEMY_SPEED;
        if (bgMusic && musicOn) { Mix_PlayMusic(bgMusic, -1); }
        return; // Thoát updateGame
    }

    // Logic xóa đạn địch ngoài màn hình đã được gộp vào vòng lặp ở trên
}
// -----------------------------------------------------
// --- Cleanup Function ---
void cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(playerTexture); /* ... và các texture khác ... */
    SDL_DestroyTexture(gameBackgroundTexture);
    // Không cần giải phóng font global ở đây
    Mix_FreeMusic(bgMusic); /* ... và các chunk ... */
    Mix_CloseAudio();
}

// --- Main Function ---
int main(int argc, char* argv[]) {
    // --- Initialize SDL and Subsystems ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) { std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl; return -1; }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) { std::cerr << "IMG Init Error: " << IMG_GetError() << std::endl; SDL_Quit(); return -1; }
    if (TTF_Init() == -1) { std::cerr << "TTF Init Error: " << TTF_GetError() << std::endl; IMG_Quit(); SDL_Quit(); return -1; }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { std::cerr << "Mixer OpenAudio Error: " << Mix_GetError() << std::endl; TTF_Quit(); IMG_Quit(); SDL_Quit(); return -1; }

    srand((unsigned int)time(0));

    // --- Tải điểm cao nhất ---
    loadHighScore(); // <-- GỌI HÀM TẢI ĐIỂM

    // --- Create Window & Renderer ---
    window = SDL_CreateWindow("Airplane Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) { std::cerr << "Window Creation Error: " << SDL_GetError() << std::endl; Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit(); return -1; }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { std::cerr << "Renderer Creation Error: " << SDL_GetError() << std::endl; SDL_DestroyWindow(window); Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit(); return -1; }

    // --- Load Resources ---
    backgroundTexture = loadTexture("background.png");
    gameBackgroundTexture = loadTexture("game_background.png");
    playerTexture = loadTexture("player.png");
    enemyTexture = loadTexture("enemy.png");
    bulletTexture = loadTexture("bullet.png");
    bgMusic = Mix_LoadMUS("background.mp3");
    startSound = Mix_LoadWAV("start_sound.wav");
    shootSound = Mix_LoadWAV("shoot.wav");
    font = TTF_OpenFont("arial.ttf", 28); // Assign to global font pointer

    // --- Check Resource Loading ---
    if (!backgroundTexture || !gameBackgroundTexture || !playerTexture || !enemyTexture || !bulletTexture || !font || !bgMusic || !startSound || !shootSound) {
        std::cerr << "Failed to load one or more resources!" << std::endl;
        if (font) TTF_CloseFont(font); // Close font if loaded
        cleanup(); // Free other loaded resources
        Mix_CloseAudio(); // Ensure audio is closed if Mix_OpenAudio succeeded
        TTF_Quit(); IMG_Quit(); SDL_Quit();
        return -1;
    }

    if (musicOn && bgMusic) { Mix_PlayMusic(bgMusic, -1); } // Bật nhạc nếu cần

    // --- Initialize Player ---
    player.rect = { SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 100, 50, 50 };
    player.texture = playerTexture;
    player.xPos = (float)player.rect.x; // Init player xPos
    player.yPos = (float)player.rect.y; // Init player yPos

    // --- Game Loop Variables ---
    bool running = true;
    SDL_Event event;
    Uint32 lastFrameTime = SDL_GetTicks();


    // --- Main Game Loop ---
    while (running) {
        // --- Delta Time ---
        Uint32 currentFrameTime = SDL_GetTicks();
        Uint32 frameTicks = SDL_TICKS_PASSED(currentFrameTime, lastFrameTime);
        float deltaTime = frameTicks / 1000.0f;
        lastFrameTime = currentFrameTime;
        if (deltaTime > 0.1f) { deltaTime = 0.1f; }

        // --- Input ---
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running = false; }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (state == GAME) {
                        Entity newBullet;
                        newBullet.rect = { player.rect.x + player.rect.w / 2 - 5, player.rect.y, 10, 20 };
                        newBullet.texture = bulletTexture;
                        newBullet.xPos = (float)newBullet.rect.x; // Init bullet xPos
                        newBullet.yPos = (float)player.rect.y;
                        // Player bullets have implicit speedY via PLAYER_BULLET_SPEED, speedX = 0
                        bullets.push_back(newBullet);
                        if (shootSound) Mix_PlayChannel(-1, shootSound, 0);
                    }
                    else { // MENU
                        for (int i = 0; i < BUTTON_COUNT; i++) {
                            if (buttons[i].hovered) {
                                if (i == 0) { // Start Game
                                    currentScore = 0; // <-- RESET ĐIỂM HIỆN TẠI
                                    state = GAME;
                                    enemies.clear(); bullets.clear(); enemyBullets.clear();
                                    currentEnemySpeed = INITIAL_ENEMY_SPEED;
                                    player.rect.x = SCREEN_WIDTH / 2 - 25; player.rect.y = SCREEN_HEIGHT - 100;
                                    player.xPos = (float)player.rect.x; player.yPos = (float)player.rect.y;
                                    lastEnemySpawnTime = SDL_GetTicks();
                                    if (Mix_PlayingMusic()) Mix_HaltMusic();
                                    if (startSound) Mix_PlayChannel(-1, startSound, 0);
                                }
                                else if (i == 1) { /* Continue */ }
                                else if (i == 2) { /* Music */
                                    musicOn = !musicOn;
                                    buttons[i].text = musicOn ? "Music: On" : "Music: Off";
                                    if (musicOn) {
                                        if (Mix_PausedMusic()) Mix_ResumeMusic();
                                        else if (bgMusic && !Mix_PlayingMusic()) Mix_PlayMusic(bgMusic, -1);
                                    }
                                    else if (Mix_PlayingMusic()) { Mix_PauseMusic(); }
                                }
                                else if (i == 3) { running = false; } /* Exit */
                                break;
                            }
                        }
                    }
                } // End Left Click
            } // End Mouse Down
        } // End PollEvent

        // --- Update ---
        player.rect.x = mouseX - player.rect.w / 2;
        player.rect.y = mouseY - player.rect.h / 2;
        player.rect.x = std::max(0, std::min(SCREEN_WIDTH - player.rect.w, player.rect.x));
        player.rect.y = std::max(0, std::min(SCREEN_HEIGHT - player.rect.h, player.rect.y));
        player.xPos = (float)player.rect.x; // Update player float pos
        player.yPos = (float)player.rect.y;

        if (state == MENU) {
            for (int i = 0; i < BUTTON_COUNT; i++) {
                buttons[i].hovered = (mouseX >= buttons[i].rect.x && mouseX <= buttons[i].rect.x + buttons[i].rect.w &&
                    mouseY >= buttons[i].rect.y && mouseY <= buttons[i].rect.y + buttons[i].rect.h);
            }
        }
        else if (state == GAME) {
            Uint32 currentTimeForSpawn = SDL_GetTicks();
            if (currentTimeForSpawn > lastEnemySpawnTime + ENEMY_SPAWN_INTERVAL) {
                spawnEnemy();
                lastEnemySpawnTime = currentTimeForSpawn;
            }
            enemyShoot();
            updateGame(deltaTime); // Pass deltaTime
        }

        // --- Render ---
        SDL_RenderClear(renderer);

        // Vẽ background
        if (state == MENU) { SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL); }
        else { SDL_RenderCopy(renderer, gameBackgroundTexture, NULL, NULL); }

        // --- Vẽ Điểm ---
        SDL_Color white = { 255, 255, 255, 255 };
        if (state == MENU) {
            // Vẽ các button menu
            for (int i = 0; i < BUTTON_COUNT; i++) {
                renderButton(buttons[i]); // Gọi hàm đã sửa
            }
            // Vẽ điểm cao nhất ở Menu (ví dụ: góc trên trái)
            std::string hsText = "High Score: " + std::to_string(highScore);
            renderText(hsText, 10, 10, white);
        }
        else if (state == GAME) {
            // Vẽ các đối tượng game
            SDL_RenderCopy(renderer, player.texture, NULL, &player.rect);
            for (auto& enemy : enemies) { SDL_RenderCopy(renderer, enemy.texture, NULL, &enemy.rect); }
            for (auto& bullet : bullets) { SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect); }
            for (auto& bullet : enemyBullets) { SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect); }

            // Vẽ điểm hiện tại trong Game (ví dụ: góc trên trái)
            std::string scoreText = "Score: " + std::to_string(currentScore);
            renderText(scoreText, 10, 10, white);
        }
        // ---------------

        SDL_RenderPresent(renderer);
        // --- End Render ---

    } // --- End Game Loop ---

    // --- Dọn dẹp và Thoát ---
    cleanup();
    TTF_CloseFont(font); // <-- Giải phóng font global
    TTF_Quit(); IMG_Quit(); Mix_CloseAudio(); SDL_Quit(); // <-- Đảm bảo Mix_CloseAudio được gọi

    return 0;
}