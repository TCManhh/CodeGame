#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const int ENEMY_SPAWN_INTERVAL = 1000; // 1000ms sinh kẻ địch mới
Uint32 lastEnemySpawnTime = 0;
float enemySpeed = 0.1;

enum GameState { MENU, GAME };
GameState state = MENU;

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
    SDL_Rect rect;
    SDL_Texture* texture;
    float yPos;
    float speed;
    int enemyStopY; // Điểm mà kẻ địch sẽ dừng lại
    Uint32 lastShotTime = (Uint32)0;  // Lưu thời điểm bắn cuối cùng
    Uint32 fireCooldown = (Uint32)(1000 + (rand() % 2000)); // Mỗi kẻ địch có thời gian bắn khác nhau (1000 - 3000ms)
};

SDL_Texture* playerTexture = nullptr;
SDL_Texture* enemyTexture = nullptr;
SDL_Texture* bulletTexture = nullptr;
std::vector<Entity> enemies;
std::vector<Entity> bullets;
std::vector<Entity> enemyBullets;
Entity player;

bool musicOn = true;
Mix_Music* bgMusic = nullptr;
Mix_Chunk* startSound = nullptr;
Mix_Chunk* shootSound = nullptr;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* backgroundTexture = nullptr;

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cerr << "Failed to load image: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void renderButton(TTF_Font* font, Button& button) {
    SDL_Color color = button.hovered ? SDL_Color{ 150, 150, 150, 255 } : SDL_Color{ 100, 100, 100, 255 };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &button.rect);

    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, button.text.c_str(), textColor);
    if (!textSurface) return;

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = { button.rect.x + (button.rect.w - textSurface->w) / 2,
                         button.rect.y + (button.rect.h - textSurface->h) / 2,
                         textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderGame() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, player.texture, NULL, &player.rect);
    for (auto& enemy : enemies) SDL_RenderCopy(renderer, enemy.texture, NULL, &enemy.rect);
    for (auto& bullet : bullets) SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);
    SDL_RenderPresent(renderer);
    for (auto& bullet : enemyBullets) {
        SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);
    }

}

void spawnEnemy() {
    enemySpeed += 0.005f; // Tăng tốc độ mỗi lần sinh kẻ địch
    int stopY = rand() % 100 + 50; // Kẻ địch dừng trong khoảng từ 300px đến 500px

    Entity newEnemy = {
        {rand() % (SCREEN_WIDTH - 50), -50, 50, 50}, // Kẻ địch xuất hiện từ trên
        enemyTexture,
        -50.0f,
        enemySpeed,
        stopY, // Lưu lại điểm dừng
        0, // lastShotTime
        1000 + (rand() % 2000) // fireCooldown ngẫu nhiên
    };
    enemies.push_back(newEnemy);
}

const int ENEMY_FIRE_INTERVAL = 2000; // 2 giây
Uint32 lastEnemyFireTime = 0;

void enemyShoot() {
    Uint32 currentTime = SDL_GetTicks();
    for (auto& enemy : enemies) {
        if (currentTime > enemy.lastShotTime + enemy.fireCooldown) { // Bắn sau khoảng thời gian ngẫu nhiên
            Entity enemyBullet = {
                { enemy.rect.x + enemy.rect.w / 2 - 5, enemy.rect.y + enemy.rect.h, 10, 20 },
                bulletTexture,
                (float)(enemy.rect.y + enemy.rect.h),
                0.25 // Thêm tốc độ cho đạn địch
            };
            enemyBullets.push_back(enemyBullet);
            enemy.lastShotTime = currentTime;
            enemy.fireCooldown = 1000 + (rand() % 2000); // Thiết lập lại thời gian bắn tiếp theo
        }
    }
}



void updateGame() {
    float bulletSpeed = 0.25;

    for (auto& bullet : bullets) {
        bullet.yPos -= bulletSpeed;
        bullet.rect.y = (int)bullet.yPos;
    }
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](Entity& b) { return b.rect.y < 0; }), bullets.end());

    for (auto& enemy : enemies) {
        if (enemy.yPos < enemy.enemyStopY) { // Chỉ di chuyển nếu chưa đến điểm dừng
            enemy.yPos += enemy.speed;
            enemy.rect.y = (int)enemy.yPos;
        }
    }

    // Xóa kẻ địch nếu nó đã ngừng di chuyển trong một thời gian (tuỳ chọn)
    enemies.erase(remove_if(enemies.begin(), enemies.end(),
        [](Entity& e) { return e.rect.y > SCREEN_HEIGHT; }), enemies.end());

    // Kiểm tra va chạm với đạn
    for (int i = bullets.size() - 1; i >= 0; i--) {
        for (int j = enemies.size() - 1; j >= 0; j--) {
            if (SDL_HasIntersection(&bullets[i].rect, &enemies[j].rect)) {
                enemies.erase(enemies.begin() + j);  // Xóa kẻ địch
                bullets.erase(bullets.begin() + i);  // Xóa đạn
                break;  // Dừng kiểm tra vì viên đạn đã bị xóa
            }
        }
    }
    // Kiểm tra va chạm giữa kẻ địch và người chơi
    for (auto& enemy : enemies) {
        if (SDL_HasIntersection(&enemy.rect, &player.rect)) {
            std::cout << "Game Over!" << std::endl;
            state = MENU; // Quay về menu
            enemies.clear();
            bullets.clear();
            break;
        }
    }

    for (auto& bullet : enemyBullets) {
        bullet.yPos += bullet.speed;
        bullet.rect.y = (int)bullet.yPos;
    }

    // Xóa đạn khi ra khỏi màn hình
    enemyBullets.erase(remove_if(enemyBullets.begin(), enemyBullets.end(),
        [](Entity& b) { return b.rect.y > SCREEN_HEIGHT; }), enemyBullets.end());

    for (auto it_bullet = enemyBullets.begin(); it_bullet != enemyBullets.end();) {
        if (SDL_HasIntersection(&it_bullet->rect, &player.rect)) {
            std::cout << "Player hit! Game Over!" << std::endl;
            state = MENU; // Quay lại menu khi trúng đạn
            enemies.clear();
            bullets.clear();
            enemyBullets.clear();
            break;
        }
        else {
            ++it_bullet;
        }
    }

}


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    window = SDL_CreateWindow("Airplane Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    backgroundTexture = loadTexture("background.png");
    if (!backgroundTexture) {
        std::cerr << "Failed to load background texture!\n";
        return -1;
    }

    playerTexture = loadTexture("player.png");
    if (!playerTexture) {
        std::cerr << "Failed to load player texture!\n";
        return -1;
    }

    enemyTexture = loadTexture("enemy.png");
    if (!enemyTexture) {
        std::cerr << "Failed to load enemy texture!\n";
        return -1;
    }

    bulletTexture = loadTexture("bullet.png");
    if(!bulletTexture) {
        std::cerr << "Failed to load bullet texture!\n";
        return -1;
    }

    bgMusic = Mix_LoadMUS("background.mp3");
    startSound = Mix_LoadWAV("start_sound.wav");
    shootSound = Mix_LoadWAV("shoot.wav");
    if (bgMusic) Mix_PlayMusic(bgMusic, -1);

    player = { {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 100, 50, 50}, playerTexture, (float)(SCREEN_HEIGHT - 100) };

    TTF_Font* font = TTF_OpenFont("arial.ttf", 32);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return -1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        player.rect.x = mouseX - player.rect.w / 2;
        player.rect.y = mouseY - player.rect.h / 2;

        player.rect.x = std::max(0, std::min(SCREEN_WIDTH - player.rect.w, player.rect.x));
        player.rect.y = std::max(0, std::min(SCREEN_HEIGHT - player.rect.h, player.rect.y));

        for (int i = 0; i < BUTTON_COUNT; i++) {
            buttons[i].hovered = (mouseX >= buttons[i].rect.x && mouseX <= buttons[i].rect.x + buttons[i].rect.w &&
                mouseY >= buttons[i].rect.y && mouseY <= buttons[i].rect.y + buttons[i].rect.h);
        }

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime > lastEnemySpawnTime + ENEMY_SPAWN_INTERVAL) {
            spawnEnemy();
            lastEnemySpawnTime = currentTime;
        }


        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (state == GAME) {
                    bullets.push_back({ {player.rect.x + player.rect.w / 2 - 5, player.rect.y, 10, 20}, bulletTexture, (float)player.rect.y });
                    if (shootSound) Mix_PlayChannel(-1, shootSound, 0);
                }
                else {
                    for (int i = 0; i < BUTTON_COUNT; i++) {
                        if (buttons[i].hovered) {
                            if (i == 0) {
                                state = GAME;
                                if (bgMusic) Mix_HaltMusic();
                                if (startSound) Mix_PlayChannel(-1, startSound, 0);
                            }
                            else if (i == 2) {
                                musicOn = !musicOn;
                                if (musicOn) { Mix_ResumeMusic(); buttons[i].text = "Music: On"; }
                                else { Mix_PauseMusic(); buttons[i].text = "Music: Off"; }
                            }
                            else if (i == 3) running = false;
                        }
                    }
                }
            }
        }

        SDL_RenderClear(renderer);
        if (state == MENU) {
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
            for (int i = 0; i < BUTTON_COUNT; i++) {
                renderButton(font, buttons[i]);
            }
        }
        else {
            if (state == GAME) {
                enemyShoot();
                updateGame();
            }

            updateGame();
            renderGame();
        }
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyTexture(backgroundTexture);
    Mix_FreeMusic(bgMusic);
    Mix_FreeChunk(startSound);
    Mix_FreeChunk(shootSound);
    TTF_CloseFont(font);
    TTF_Quit();
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}