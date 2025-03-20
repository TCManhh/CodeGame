#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <iostream>

// Kích thước cửa sổ
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// Trạng thái game
enum GameState { MENU, GAME };
GameState state = MENU;

// Cấu trúc lưu thông tin của nút
struct Button {
    SDL_Rect rect;
    std::string text;
    bool selected;
    bool hovered;
};

// Tạo các nút
Button buttons[] = {
    {{500, 200, 280, 70}, "Start New Game", false, false},
    {{500, 290, 280, 70}, "Continue Game", false, false},
    {{500, 380, 280, 70}, "Music: On", false, false},
    {{500, 470, 280, 70}, "Exit Game", false, false}
};

const int BUTTON_COUNT = sizeof(buttons) / sizeof(Button);
int selectedButton = 0;

// Biến kiểm soát âm nhạc
bool musicOn = true;
Mix_Music* bgMusic = nullptr;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* backgroundTexture = nullptr;

// Load ảnh nền
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Texture* texture = nullptr;
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cerr << "Failed to load image: " << IMG_GetError() << std::endl;
    }
    else {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    return texture;
}

// Vẽ một nút lên màn hình
void renderButton(SDL_Renderer* renderer, TTF_Font* font, Button& button, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &button.rect);

    SDL_Color textColor = { 255, 255, 255, 255 };
    TTF_Font* smallFont = TTF_OpenFont("arial.ttf", 32);
    SDL_Surface* textSurface = TTF_RenderText_Blended(smallFont, button.text.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    int textW = textSurface->w;
    int textH = textSurface->h;
    SDL_Rect textRect = { button.rect.x + (button.rect.w - textW) / 2, button.rect.y + (button.rect.h - textH) / 2, textW, textH };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(smallFont);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        std::cerr << "Mixer Init Error: " << Mix_GetError() << std::endl;
    }
    else {
        bgMusic = Mix_LoadMUS("background.mp3");
        if (bgMusic) {
            Mix_PlayMusic(bgMusic, -1);
        }
    }

    window = SDL_CreateWindow("Tower Defense", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        std::cerr << "SDL Window Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL Renderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    backgroundTexture = loadTexture("background.png", renderer);

    TTF_Font* font = TTF_OpenFont("arial.ttf", 32);
    if (!font) {
        std::cerr << "Font Load Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                for (int i = 0; i < BUTTON_COUNT; i++) {
                    if (mouseX >= buttons[i].rect.x && mouseX <= buttons[i].rect.x + buttons[i].rect.w &&
                        mouseY >= buttons[i].rect.y && mouseY <= buttons[i].rect.y + buttons[i].rect.h) {
                        selectedButton = i;
                        if (selectedButton == 2) {
                            if (Mix_PlayingMusic()) {
                                if (Mix_PausedMusic()) {
                                    Mix_ResumeMusic();
                                    buttons[2].text = "Music: On";
                                }
                                else {
                                    Mix_PauseMusic();
                                    buttons[2].text = "Music: Off";
                                }
                            }
                        }
                        else if (selectedButton == 3) {
                            running = false;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < BUTTON_COUNT; i++) {
            buttons[i].hovered = (mouseX >= buttons[i].rect.x && mouseX <= buttons[i].rect.x + buttons[i].rect.w &&
                mouseY >= buttons[i].rect.y && mouseY <= buttons[i].rect.y + buttons[i].rect.h);
        }

        SDL_RenderClear(renderer);
        if (backgroundTexture) {
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        }

        for (int i = 0; i < BUTTON_COUNT; i++) {
            SDL_Color color = buttons[i].hovered ? SDL_Color{ 150, 150, 150, 255 } : SDL_Color{ 100, 100, 100, 255 };
            renderButton(renderer, font, buttons[i], color);
        }

        SDL_RenderPresent(renderer);
    }

    return 0;
}
