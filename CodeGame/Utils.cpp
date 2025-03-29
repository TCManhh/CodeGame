#include "Utils.h"

#include "Globals.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <iostream>
#include <fstream>
#include <string>
#include <limits>

// --- Function Definitions (Phần triển khai hàm) ---

SDL_Texture* loadTexture(const char* path) {
    // Cần biến toàn cục 'renderer' từ Globals.h
    if (!renderer) {
        std::cerr << "Error: Renderer not initialized before loading texture: " << path << std::endl;
        return nullptr;
    }
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cerr << "Failed to load image: " << path << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Giải phóng surface ngay sau khi tạo texture
    if (!texture) {
        std::cerr << "Failed to create texture from " << path << " - " << SDL_GetError() << std::endl;
    }
    return texture;
}

void saveHighScore() {
    // Cần biến toàn cục 'highScore' từ Globals.h
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
    // Cần biến toàn cục 'highScore' từ Globals.h
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        // Đọc điểm, nếu lỗi hoặc không đọc được thì đặt là 0
        if (!(file >> highScore)) {
            std::cerr << "Warning: Could not read highscore from file or file is corrupted. Resetting to 0." << std::endl;
            highScore = 0;
        }
        else {
            // Kiểm tra nếu đọc được giá trị âm (không hợp lệ)
            if (highScore < 0) {
                std::cerr << "Warning: Invalid negative highscore found. Resetting to 0." << std::endl;
                highScore = 0;
            }
        }
        file.close();
        std::cout << "Loaded high score: " << highScore << std::endl;
    }
    else {
        // File không tồn tại, bắt đầu với điểm 0
        std::cout << "highscore.txt not found. Starting with high score 0." << std::endl;
        highScore = 0;
    }
}


void renderText(const std::string& text, int x, int y, SDL_Color color) {
    // Cần biến toàn cục 'font' và 'renderer' từ Globals.h
    if (!font) {
        std::cerr << "Font not loaded in renderText!\n";
        return;
    }
    if (!renderer) {
        std::cerr << "Renderer not available in renderText!\n";
        return;
    }
    if (text.empty()) {
        return; // Không làm gì nếu chuỗi rỗng
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "Failed to render text surface: '" << text << "' - " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create text texture: " << SDL_GetError() << std::endl;
        // Vẫn phải giải phóng surface ngay cả khi tạo texture lỗi
    }
    else {
        // Tạo hình chữ nhật đích để vẽ texture
        SDL_Rect dstRect = { x, y, surface->w, surface->h };
        // Vẽ texture lên renderer
        SDL_RenderCopy(renderer, texture, NULL, &dstRect);
        // Hủy texture sau khi vẽ xong
        SDL_DestroyTexture(texture);
    }

    // Luôn giải phóng surface sau khi đã dùng xong (hoặc khi lỗi)
    SDL_FreeSurface(surface);
}

void renderButton(Button& button) {
    // Cần biến toàn cục 'font' và 'renderer' từ Globals.h
    if (!font) {
        std::cerr << "Font not loaded in renderButton!\n";
        return;
    }
    if (!renderer) {
        std::cerr << "Renderer not available in renderButton!\n";
        return;
    }

    // Chọn màu nền dựa trên trạng thái hover
    SDL_Color bgColor = button.hovered ? SDL_Color{ 150, 150, 150, 255 } : SDL_Color{ 100, 100, 100, 255 };
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
    SDL_RenderFillRect(renderer, &button.rect); // Vẽ nền nút

    // Vẽ chữ của nút (căn giữa)
    if (!button.text.empty()) {
        SDL_Color textColor = { 255, 255, 255, 255 }; // Màu chữ trắng
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, button.text.c_str(), textColor);
        if (!textSurface) {
            std::cerr << "Failed to render button text surface: " << TTF_GetError() << std::endl;
            // Vẫn tiếp tục vẽ nền nút ngay cả khi chữ lỗi
            return;
        }

        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (!textTexture) {
            std::cerr << "CreateTextureFromSurface Error (button text): " << SDL_GetError() << std::endl;
            SDL_FreeSurface(textSurface);
            return;
        }

        // Tính toán vị trí để căn giữa chữ trong nút
        SDL_Rect textRect = {
            button.rect.x + (button.rect.w - textSurface->w) / 2,
            button.rect.y + (button.rect.h - textSurface->h) / 2,
            textSurface->w,
            textSurface->h
        };

        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // Dọn dẹp texture và surface của chữ
        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);
    }
}


void cleanup() {
    // --- Điền đầy đủ phần giải phóng textures ---
    SDL_DestroyTexture(playerTextureLvl1);
    SDL_DestroyTexture(playerTextureLvl2);
    SDL_DestroyTexture(playerTextureLvl3);
    SDL_DestroyTexture(playerTextureLvl4);
    SDL_DestroyTexture(bulletTextureLvl1);
    SDL_DestroyTexture(bulletTextureLvl2);
    SDL_DestroyTexture(bulletTextureLvl3);
    SDL_DestroyTexture(bulletTextureLvl4);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(enemyTextureStraight);
    SDL_DestroyTexture(enemyTextureWeave);
    SDL_DestroyTexture(enemyTextureTank);
    SDL_DestroyTexture(normalBulletTexture);
    SDL_DestroyTexture(straightShooterBulletTexture);
    SDL_DestroyTexture(tankBulletTexture);
    SDL_DestroyTexture(weaverBulletTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(gameBackgroundTexture);
    SDL_DestroyTexture(gameOverBackgroundTexture);
    SDL_DestroyTexture(explosionTexture);
    // --- Hết phần textures ---

    // Giải phóng Font
    if (font) {
        TTF_CloseFont(font);
    }

    // Giải phóng Âm thanh
    Mix_FreeMusic(bgMusic);
    Mix_FreeMusic(gameMusic);
    Mix_FreeChunk(startSound);
    Mix_FreeChunk(explosionSound);

    // Hủy Renderer và Window (sử dụng biến toàn cục từ Globals.h)
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }

    // --- Điền đầy đủ phần đặt con trỏ về nullptr ---
    playerTextureLvl1 = nullptr;
    playerTextureLvl2 = nullptr;
    playerTextureLvl3 = nullptr;
    playerTextureLvl4 = nullptr;
    bulletTextureLvl1 = nullptr;
    bulletTextureLvl2 = nullptr;
    bulletTextureLvl3 = nullptr;
    bulletTextureLvl4 = nullptr;
    enemyTexture = nullptr;
    enemyTextureStraight = nullptr;
    enemyTextureWeave = nullptr;
    enemyTextureTank = nullptr;
    normalBulletTexture = nullptr;
    straightShooterBulletTexture = nullptr;
    tankBulletTexture = nullptr;
    weaverBulletTexture = nullptr;
    backgroundTexture = nullptr;
    gameBackgroundTexture = nullptr;
    gameOverBackgroundTexture = nullptr;
    explosionTexture = nullptr;
    font = nullptr;
    bgMusic = nullptr;
    gameMusic = nullptr;
    startSound = nullptr;
    explosionSound = nullptr;
    renderer = nullptr;
    window = nullptr;
    // --- Hết phần đặt về nullptr ---

    std::cout << "Resources cleaned up." << std::endl;
}