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
#include <string>
#include <fstream>
#include <sstream>
#include <limits>
#include <map>

// --- Include các header tự tạo ---
#include "Constants.h"
#include "Structs.h"
#include "Globals.h"
#include "Utils.h"
#include "GameLogic.h"

// Textures
SDL_Texture* playerTextureLvl1 = nullptr;
SDL_Texture* playerTextureLvl2 = nullptr;
SDL_Texture* playerTextureLvl3 = nullptr;
SDL_Texture* playerTextureLvl4 = nullptr;
SDL_Texture* bulletTextureLvl1 = nullptr;
SDL_Texture* bulletTextureLvl2 = nullptr;
SDL_Texture* bulletTextureLvl3 = nullptr;
SDL_Texture* bulletTextureLvl4 = nullptr;
SDL_Texture* enemyTexture = nullptr;
SDL_Texture* enemyTextureStraight = nullptr;
SDL_Texture* enemyTextureWeave = nullptr;
SDL_Texture* enemyTextureTank = nullptr;
SDL_Texture* normalBulletTexture = nullptr;
SDL_Texture* straightShooterBulletTexture = nullptr;
SDL_Texture* tankBulletTexture = nullptr;
SDL_Texture* weaverBulletTexture = nullptr;
SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* gameBackgroundTexture = nullptr;
SDL_Texture* gameOverBackgroundTexture = nullptr;
SDL_Texture* explosionTexture = nullptr;

// Game Objects & Containers
std::vector<Entity> enemies;
std::vector<Entity> bullets;
std::vector<Entity> enemyBullets;
std::vector<ExplosionEffect> activeExplosions;
Entity player;

// Audio
bool musicOn = true;
Mix_Music* bgMusic = nullptr;
Mix_Music* gameMusic = nullptr;
Mix_Chunk* startSound = nullptr;
Mix_Chunk* explosionSound = nullptr;

// SDL Core & Font
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;

// Game State & Scores
GameState state = MENU;
int currentScore = 0;
int highScore = 0;

// Timing & Speed
Uint32 lastEnemySpawnTime = 0;
float currentEnemySpeed = INITIAL_ENEMY_SPEED;

// Input & Control State
bool running = true;
bool isPlayerDragging = false;
bool isMovingToTarget = false;
float targetMoveX = 0.0f;
float targetMoveY = 0.0f;
bool isFollowingMouse = false;

// Buttons (Định nghĩa mảng và số lượng)
Button buttons[] = {
    {{centeredButtonX, 200, 280, 70}, "Start New Game", false},
    {{centeredButtonX, 290, 280, 70}, "Music: On", false},
    {{centeredButtonX, 380, 280, 70}, "Exit Game", false}
};
const int BUTTON_COUNT = sizeof(buttons) / sizeof(Button);
Button gameOverButtons[] = {
    {{centeredButtonX, 350, 280, 70}, "Play Again", false},
    {{centeredButtonX, 440, 280, 70}, "Main Menu", false}
};
const int GAMEOVER_BUTTON_COUNT = sizeof(gameOverButtons) / sizeof(Button);

Button pauseButton = {
    {SCREEN_WIDTH - PAUSE_BUTTON_WIDTH - 10, 10, PAUSE_BUTTON_WIDTH, PAUSE_BUTTON_HEIGHT},
    "Pause",
    false
};

Button pauseMenuButtons[] = {
    {{pauseMenuButtonX, 250, PAUSE_MENU_BUTTON_WIDTH, PAUSE_MENU_BUTTON_HEIGHT}, "Continue", false},
    {{pauseMenuButtonX, 340, PAUSE_MENU_BUTTON_WIDTH, PAUSE_MENU_BUTTON_HEIGHT}, "Mute Sound", false}, // Text sẽ cập nhật
    {{pauseMenuButtonX, 430, PAUSE_MENU_BUTTON_WIDTH, PAUSE_MENU_BUTTON_HEIGHT}, "Exit to Menu", false}
};
const int PAUSE_MENU_BUTTON_COUNT = sizeof(pauseMenuButtons) / sizeof(Button);

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
    // ... (Kiểm tra lỗi window)
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    // ... (Kiểm tra lỗi renderer)
    font = TTF_OpenFont("arial.ttf", 28);
    // ... (Kiểm tra lỗi font)

    // --- Load Resources (Gọi hàm loadTexture từ Utils.cpp) ---
    backgroundTexture = loadTexture("background.png");
    gameBackgroundTexture = loadTexture("game_background.png");
    gameOverBackgroundTexture = loadTexture("gameover_background.png");
    playerTextureLvl1 = loadTexture("player_lvl1.png");
    playerTextureLvl2 = loadTexture("player_lvl2.png");
    playerTextureLvl3 = loadTexture("player_lvl3.png");
    playerTextureLvl4 = loadTexture("player_lvl4.png");
    bulletTextureLvl1 = loadTexture("bullet_lvl1.png");
    bulletTextureLvl2 = loadTexture("bullet_lvl2.png");
    bulletTextureLvl3 = loadTexture("bullet_lvl3.png");
    bulletTextureLvl4 = loadTexture("bullet_lvl4.png");
    enemyTexture = loadTexture("enemy.png");
    enemyTextureStraight = loadTexture("enemy_straight.png");
    enemyTextureWeave = loadTexture("enemy_weave.png");
    enemyTextureTank = loadTexture("enemy_tank.png");
    normalBulletTexture = loadTexture("normal_bullet.png");
    straightShooterBulletTexture = loadTexture("straight_shooter_bullet.png");
    tankBulletTexture = loadTexture("tank_bullet.png");
    weaverBulletTexture = loadTexture("weaver_bullet.png");
    explosionTexture = loadTexture("explosion_spritesheet.png");

    // Load Audio
    bgMusic = Mix_LoadMUS("background.mp3");
    gameMusic = Mix_LoadMUS("game_music.mp3");
    startSound = Mix_LoadWAV("start_sound.wav");
    explosionSound = Mix_LoadWAV("explosion.wav");

    // --- Check Resource Loading ---
    if (!backgroundTexture || !gameBackgroundTexture || !gameOverBackgroundTexture ||
        !playerTextureLvl1 || !playerTextureLvl2 || !playerTextureLvl3 || !playerTextureLvl4 ||
        !bulletTextureLvl1 || !bulletTextureLvl2 || !bulletTextureLvl3 || !bulletTextureLvl4 ||
        !enemyTexture || !enemyTextureStraight || !enemyTextureWeave || !enemyTextureTank ||
        !normalBulletTexture || !straightShooterBulletTexture || !tankBulletTexture ||
        !weaverBulletTexture || !explosionTexture ||
        !bgMusic || !gameMusic || !startSound || !explosionSound || !font)
    {
        std::cerr << "Failed to load one or more resources!" << std::endl;
        cleanup();
        Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit();
        return -1;
    }


    // --- PHÁT NHẠC MENU BAN ĐẦU ---
    if (musicOn && bgMusic) {
        Mix_PlayMusic(bgMusic, -1);
    }

    // --- Initialize Player State ---
    player.rect.w = PLAYER_WIDTH_LVL1;
    player.rect.h = PLAYER_HEIGHT_LVL1;
    player.rect.x = SCREEN_WIDTH / 2 - player.rect.w / 2;
    player.rect.y = SCREEN_HEIGHT - 100 - player.rect.h;
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
    Uint32 lastFrameTimeTicks = SDL_GetTicks();
    running = true;

    // --- Main Game Loop ---
    while (running) {
        // --- Delta Time Calculation ---
        Uint32 currentFrameTimeTicks = SDL_GetTicks();
        Uint32 frameTicks = SDL_TICKS_PASSED(currentFrameTimeTicks, lastFrameTimeTicks);
        float deltaTime = 0.0f;

        if (state != PAUSED) {
            deltaTime = frameTicks / 1000.0f;
            lastFrameTimeTicks = currentFrameTimeTicks;
            if (deltaTime > 0.1f) { deltaTime = 0.1f; }
        }
        else {
            deltaTime = 0.0f;
        }

        // --- Input Handling ---
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_Point mousePointCheckHover = { mouseX, mouseY };

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            // --- Xử lý nhấn chuột ---
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_Point mouseClickPoint = { event.button.x, event.button.y };

                    // Xử lý click dựa trên Game State
                    if (state == MENU) {
                        for (int i = 0; i < BUTTON_COUNT; i++) {
                            if (buttons[i].hovered) {
                                if (i == 0) { // Start New Game
                                    // --- RESET GAME STATE ---
                                    currentScore = 0;
                                    enemies.clear(); bullets.clear(); enemyBullets.clear();
                                    activeExplosions.clear();
                                    currentEnemySpeed = INITIAL_ENEMY_SPEED;

                                    // Reset player về trạng thái ban đầu Lvl 1
                                    player.rect.w = PLAYER_WIDTH_LVL1;
                                    player.rect.h = PLAYER_HEIGHT_LVL1;
                                    player.rect.x = SCREEN_WIDTH / 2 - player.rect.w / 2;
                                    player.rect.y = SCREEN_HEIGHT - 100 - player.rect.h;
                                    player.xPos = (float)player.rect.x;
                                    player.yPos = (float)player.rect.y;
                                    player.health = PLAYER_INITIAL_HEALTH;
                                    player.level = 1;
                                    player.texture = playerTextureLvl1;
                                    player.lastShotTime = 0;

                                    lastEnemySpawnTime = SDL_GetTicks();

                                    // Chuyển nhạc và trạng thái
                                    Mix_HaltMusic();
                                    if (musicOn && gameMusic) { Mix_PlayMusic(gameMusic, -1); }
                                    isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                                    state = GAME;
                                    lastFrameTimeTicks = SDL_GetTicks();
                                    // --- END RESET GAME STATE ---
                                }
                                else if (i == 1) { // Music On/Off
                                    musicOn = !musicOn;
                                    buttons[i].text = musicOn ? "Music: On" : "Music: Off";
                                    pauseMenuButtons[1].text = musicOn ? "Mute Sound" : "Unmute Sound";
                                    if (musicOn) {
                                        if (Mix_PausedMusic()) Mix_ResumeMusic();
                                        else if (!Mix_PlayingMusic()) Mix_PlayMusic(bgMusic, -1);
                                        Mix_Resume(-1);
                                    }
                                    else {
                                        if (Mix_PlayingMusic()) Mix_PauseMusic(); // Pause nhạc nền
                                        Mix_Pause(-1);
                                    }
                                }
                                else if (i == 2) { // Exit Game
                                    running = false;
                                }
                                break; // Thoát khỏi vòng lặp button khi đã tìm thấy nút được click
                            }
                        }
                    }
                    else if (state == GAME_OVER) {
                        for (int i = 0; i < GAMEOVER_BUTTON_COUNT; i++) {
                            if (gameOverButtons[i].hovered) {
                                if (i == 0) { // Play Again (Tương tự Start New Game)
                                    // --- RESET GAME STATE ---
                                    currentScore = 0;
                                    enemies.clear(); bullets.clear(); enemyBullets.clear();
                                    activeExplosions.clear();
                                    currentEnemySpeed = INITIAL_ENEMY_SPEED;
                                    player.rect.w = PLAYER_WIDTH_LVL1;
                                    player.rect.h = PLAYER_HEIGHT_LVL1;
                                    player.rect.x = SCREEN_WIDTH / 2 - player.rect.w / 2;
                                    player.rect.y = SCREEN_HEIGHT - 100 - player.rect.h;
                                    player.xPos = (float)player.rect.x;
                                    player.yPos = (float)player.rect.y;
                                    player.health = PLAYER_INITIAL_HEALTH;
                                    player.level = 1;
                                    player.texture = playerTextureLvl1;
                                    player.lastShotTime = 0;
                                    lastEnemySpawnTime = SDL_GetTicks();
                                    Mix_HaltMusic();
                                    if (musicOn && gameMusic) { Mix_PlayMusic(gameMusic, -1); }
                                    isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                                    state = GAME;
                                    lastFrameTimeTicks = SDL_GetTicks(); // << QUAN TRỌNG: Reset time
                                    // --- END RESET GAME STATE ---
                                }
                                else if (i == 1) { // Main Menu
                                    isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                                    activeExplosions.clear(); // Dọn dẹp hiệu ứng nổ còn sót lại
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
                        if (SDL_PointInRect(&mouseClickPoint, &pauseButton.rect)) {
                            state = PAUSED;
                            pauseMenuButtons[1].text = musicOn ? "Mute Sound" : "Unmute Sound"; // Cập nhật text nút mute/unmute
                            // Không cần reset lastFrameTimeTicks ở đây vì delta time sẽ = 0
                        }
                        else {
                            // Xử lý di chuyển player
                            isFollowingMouse = false; // Tắt chế độ follow nếu có click mới
                            if (SDL_PointInRect(&mouseClickPoint, &player.rect)) {
                                // Click vào player -> bắt đầu kéo
                                isPlayerDragging = true;
                                isMovingToTarget = false;
                            }
                            else {
                                // Click ra ngoài -> di chuyển tới điểm đó
                                isMovingToTarget = true;
                                isPlayerDragging = false;
                                // Tính toán vị trí đích (tâm player tại điểm click)
                                targetMoveX = (float)mouseClickPoint.x - player.rect.w / 2.0f;
                                targetMoveY = (float)mouseClickPoint.y - player.rect.h / 2.0f;
                                // Giới hạn vị trí đích trong màn hình
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
                                    lastFrameTimeTicks = SDL_GetTicks();
                                }
                                else if (i == 1) { // Mute/Unmute Sound
                                    musicOn = !musicOn;
                                    pauseMenuButtons[i].text = musicOn ? "Mute Sound" : "Unmute Sound";
                                    buttons[1].text = musicOn ? "Music: On" : "Music: Off";
                                    if (musicOn) {
                                        if (Mix_PausedMusic()) Mix_ResumeMusic();
                                        Mix_Resume(-1);
                                    }
                                    else {
                                        if (Mix_PlayingMusic()) Mix_PauseMusic();
                                        Mix_Pause(-1);
                                    }
                                }
                                else if (i == 2) { // Exit to Menu
                                    // Lưu điểm cao nếu cần trước khi thoát ra menu
                                    if (currentScore > highScore) {
                                        highScore = currentScore;
                                        saveHighScore();
                                    }
                                    Mix_HaltMusic();
                                    isPlayerDragging = false; isMovingToTarget = false; isFollowingMouse = false;
                                    activeExplosions.clear();
                                    state = MENU;
                                    if (musicOn && bgMusic) {
                                        Mix_PlayMusic(bgMusic, -1);
                                    }
                                    // Cập nhật lại text nút music ở menu chính
                                    buttons[1].text = musicOn ? "Music: On" : "Music: Off";
                                }
                                break;
                            }
                        }
                    }
                } // End if SDL_BUTTON_LEFT
            } // End if SDL_MOUSEBUTTONDOWN
            // --- Xử lý nhả chuột ---
            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (state == GAME) {
                        if (isPlayerDragging) {
                            isPlayerDragging = false;
                        }
                    }
                }
            } // End if SDL_MOUSEBUTTONUP
        } // --- Kết thúc vòng lặp SDL_PollEvent ---


        // --- Update Logic (Chỉ chạy khi state == GAME) ---
        if (state == GAME) {
            // 1. Cập nhật vị trí người chơi dựa trên trạng thái input
            if (isPlayerDragging) {
                player.xPos = (float)mouseX - player.rect.w / 2.0f;
                player.yPos = (float)mouseY - player.rect.h / 2.0f;
            }
            else if (isMovingToTarget) {
                float dirX = targetMoveX - player.xPos;
                float dirY = targetMoveY - player.yPos;
                float dist = std::sqrt(dirX * dirX + dirY * dirY);
                bool reachedTarget = false;

                if (dist < 5.0f) {
                    reachedTarget = true;
                }
                else {
                    float moveAmount = PLAYER_SPEED * deltaTime;
                    if (moveAmount >= dist) {
                        reachedTarget = true;
                    }
                    else { // Di chuyển một phần quãng đường
                        player.xPos += (dirX / dist) * moveAmount;
                        player.yPos += (dirY / dist) * moveAmount;
                    }
                }

                if (reachedTarget) {
                    player.xPos = targetMoveX; // Snap tới vị trí đích
                    player.yPos = targetMoveY;
                    isMovingToTarget = false; // Dừng chế độ di chuyển tới đích
                    // Optional: Chuyển sang chế độ follow chuột sau khi tới nơi
                    isFollowingMouse = true;
                }
            }
            else if (isFollowingMouse) { // Chế độ follow (nếu có)
                player.xPos = (float)mouseX - player.rect.w / 2.0f;
                player.yPos = (float)mouseY - player.rect.h / 2.0f;
            }

            // Giới hạn vị trí player trong màn hình sau khi cập nhật
            player.xPos = std::max(0.0f, std::min((float)SCREEN_WIDTH - player.rect.w, player.xPos));
            player.yPos = std::max(0.0f, std::min((float)SCREEN_HEIGHT - player.rect.h, player.yPos));
            player.rect.x = (int)player.xPos; // Cập nhật rect từ vị trí float
            player.rect.y = (int)player.yPos;


            // 2. Xử lý bắn tự động của người chơi
            Uint32 currentTime = SDL_GetTicks(); // Lấy thời gian hiện tại
            if (currentTime > player.lastShotTime + PLAYER_AUTO_FIRE_COOLDOWN) {
                Entity newBullet;
                int bulletW = 0, bulletH = 0;

                // Chọn texture và kích thước đạn dựa trên level player
                switch (player.level) {
                case 1:
                    newBullet.texture = bulletTextureLvl1;
                    bulletW = PLAYER_BULLET_WIDTH_LVL1; bulletH = PLAYER_BULLET_HEIGHT_LVL1;
                    break;
                case 2:
                    newBullet.texture = bulletTextureLvl2;
                    bulletW = PLAYER_BULLET_WIDTH_LVL2; bulletH = PLAYER_BULLET_HEIGHT_LVL2;
                    break;
                case 3:
                    newBullet.texture = bulletTextureLvl3;
                    bulletW = PLAYER_BULLET_WIDTH_LVL3; bulletH = PLAYER_BULLET_HEIGHT_LVL3;
                    break;
                case 4: default:
                    newBullet.texture = bulletTextureLvl4;
                    bulletW = PLAYER_BULLET_WIDTH_LVL4; bulletH = PLAYER_BULLET_HEIGHT_LVL4;
                    break;
                }

                // Tính vị trí bắn (từ giữa đỉnh player)
                float bulletStartX = player.xPos + player.rect.w / 2.0f - bulletW / 2.0f;
                float bulletStartY = player.yPos;

                newBullet.rect = { (int)bulletStartX, (int)bulletStartY, bulletW, bulletH };
                newBullet.xPos = bulletStartX;
                newBullet.yPos = bulletStartY;
                newBullet.damage = player.bulletDamage;
                newBullet.type = NORMAL;

                bullets.push_back(newBullet);
                player.lastShotTime = currentTime;
            }

            // 3. Gọi các hàm cập nhật logic game từ GameLogic.cpp
            // Chỉ gọi spawn nếu đủ thời gian
            if (currentTime > lastEnemySpawnTime + ENEMY_SPAWN_INTERVAL) {
                spawnEnemy();
                lastEnemySpawnTime = currentTime;
            }
            enemyShoot();
            updateGame(deltaTime);
            updateExplosions(deltaTime);

        }
        else { // Nếu state không phải GAME (MENU, PAUSED, GAME_OVER)
            // Đảm bảo các trạng thái điều khiển player được tắt
            isPlayerDragging = false;
            isMovingToTarget = false;
            isFollowingMouse = false;
        }


        // --- Cập nhật trạng thái hover của nút (cho tất cả các state) ---
        // Reset hover state trước khi kiểm tra
        for (int i = 0; i < BUTTON_COUNT; ++i) buttons[i].hovered = false;
        for (int i = 0; i < GAMEOVER_BUTTON_COUNT; ++i) gameOverButtons[i].hovered = false;
        pauseButton.hovered = false;
        for (int i = 0; i < PAUSE_MENU_BUTTON_COUNT; ++i) pauseMenuButtons[i].hovered = false;

        // Kiểm tra hover dựa trên state hiện tại
        if (state == MENU) {
            for (int i = 0; i < BUTTON_COUNT; i++) {
                buttons[i].hovered = SDL_PointInRect(&mousePointCheckHover, &buttons[i].rect);
            }
        }
        else if (state == GAME_OVER) {
            for (int i = 0; i < GAMEOVER_BUTTON_COUNT; i++) {
                gameOverButtons[i].hovered = SDL_PointInRect(&mousePointCheckHover, &gameOverButtons[i].rect);
            }
        }
        else if (state == GAME) {
            pauseButton.hovered = SDL_PointInRect(&mousePointCheckHover, &pauseButton.rect);
        }
        else if (state == PAUSED) {
            // Có thể cho phép hover cả nút Pause phía sau và các nút menu pause
            pauseButton.hovered = SDL_PointInRect(&mousePointCheckHover, &pauseButton.rect);
            for (int i = 0; i < PAUSE_MENU_BUTTON_COUNT; ++i) {
                pauseMenuButtons[i].hovered = SDL_PointInRect(&mousePointCheckHover, &pauseMenuButtons[i].rect);
            }
        }


        // --- Render ---
        SDL_RenderClear(renderer);

        // 1. Vẽ nền phù hợp với state
        if (state == MENU) {
            if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        }
        else if (state == GAME_OVER) {
            if (gameOverBackgroundTexture) SDL_RenderCopy(renderer, gameOverBackgroundTexture, NULL, NULL);
        }
        else { // GAME or PAUSED
            if (gameBackgroundTexture) SDL_RenderCopy(renderer, gameBackgroundTexture, NULL, NULL);
        }

        // 2. Vẽ nội dung chính dựa trên state
        if (state == MENU) {
            // Vẽ các nút Menu
            for (int i = 0; i < BUTTON_COUNT; i++) renderButton(buttons[i]);
            // Vẽ điểm cao
            renderText("High Score: " + std::to_string(highScore), 10, 10, { 255, 255, 255, 255 });
        }
        else if (state == GAME_OVER) {
            // Vẽ chữ Game Over, Score, High Score
            int textW = 0, textH = 0;
            std::string gameOverMsg = "Game Over!";
            if (font) TTF_SizeText(font, gameOverMsg.c_str(), &textW, &textH);
            renderText(gameOverMsg, SCREEN_WIDTH / 2 - textW / 2, 150, { 255, 255, 0, 255 });

            std::string finalScoreText = "Your Score: " + std::to_string(currentScore);
            if (font) TTF_SizeText(font, finalScoreText.c_str(), &textW, &textH);
            renderText(finalScoreText, SCREEN_WIDTH / 2 - textW / 2, 220, { 255, 255, 255, 255 });

            std::string hsText = "High Score: " + std::to_string(highScore);
            if (font) TTF_SizeText(font, hsText.c_str(), &textW, &textH);
            renderText(hsText, SCREEN_WIDTH / 2 - textW / 2, 260, { 255, 255, 255, 255 });

            // Vẽ các nút Game Over
            for (int i = 0; i < GAMEOVER_BUTTON_COUNT; i++) renderButton(gameOverButtons[i]);
        }
        else { // GAME or PAUSED
            // Vẽ các đối tượng game
            if (player.texture) SDL_RenderCopy(renderer, player.texture, NULL, &player.rect);
            for (const auto& bullet : bullets) if (bullet.texture) SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);
            for (const auto& enemy : enemies) if (enemy.texture) SDL_RenderCopy(renderer, enemy.texture, NULL, &enemy.rect);
            for (const auto& bullet : enemyBullets) if (bullet.texture) SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);

            // Vẽ hiệu ứng nổ
            for (const auto& explosion : activeExplosions) {
                if (!explosion.finished && explosion.texture) {
                    SDL_Rect srcRect = {
                        explosion.currentFrame * explosion.frameWidth, 0,
                        explosion.frameWidth, explosion.frameHeight
                    };
                    SDL_RenderCopy(renderer, explosion.texture, &srcRect, &explosion.position);
                }
            }

            // Vẽ UI trong game (Score, Level, Health)
            renderText("Score: " + std::to_string(currentScore), 10, 10, { 255, 255, 255, 255 });
            renderText("Level: " + std::to_string(player.level), 10, 70, { 255, 255, 255, 255 });

            // Vẽ thanh máu
            int healthBarX = 10, healthBarY = 40, healthBarW = 200, healthBarH = 20;
            float healthPercent = (player.health > 0) ? (float)player.health / player.maxHealth : 0.0f;
            int currentHealthBarW = (int)(healthBarW * healthPercent);
            if (currentHealthBarW < 0) currentHealthBarW = 0;
            SDL_Rect healthBarBgRect = { healthBarX, healthBarY, healthBarW, healthBarH };
            SDL_SetRenderDrawColor(renderer, 100, 0, 0, 255);
            SDL_RenderFillRect(renderer, &healthBarBgRect);
            if (currentHealthBarW > 0) {
                SDL_Rect currentHealthRect = { healthBarX, healthBarY, currentHealthBarW, healthBarH };
                SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
                SDL_RenderFillRect(renderer, &currentHealthRect);
            }
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &healthBarBgRect);

            // Vẽ nút Pause
            renderButton(pauseButton);

            // Nếu đang PAUSED, vẽ lớp phủ mờ và menu pause lên trên
            if (state == PAUSED) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
                SDL_Rect pauseOverlayRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
                SDL_RenderFillRect(renderer, &pauseOverlayRect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

                // Vẽ các nút của menu Pause
                for (int i = 0; i < PAUSE_MENU_BUTTON_COUNT; ++i) {
                    renderButton(pauseMenuButtons[i]);
                }
            }
        }

        // 3. Hiển thị mọi thứ đã vẽ lên màn hình
        SDL_RenderPresent(renderer);

    } // --- Kết thúc while(running) ---

    // --- Cleanup ---
    cleanup();
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
} // --- Kết thúc main ---