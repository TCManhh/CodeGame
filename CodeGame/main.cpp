#include <SDL.h>
#include <iostream>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 380;

#undef main

//Cửa sổ hiển thị
SDL_Window* gWindow = NULL;

//Bề mặt chứa trong cửa sổ
SDL_Surface* gScreenSurface = NULL;

//Hình ảnh chúng ta sẽ tải và hiển thị trên màn hình
SDL_Surface* gHelloWorld = NULL;

//Khởi động SDL và tạo cửa sổ 
bool init() {
    //Cờ khởi tạo
    bool success = true;

    //Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        success = false;
    }
    else
    {
        //Tạo cửa sổ
        gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            std::cout <<"Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            success = false;
        }
        else
        {
            //Dùng bề mặt cửa sổ
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }

    return success;
}

//Tải phương tiện 
bool loadMedia() {
    //Tải cờ lên thành công
    bool success = true;

    //Tải ảnh "tung tóe"
    gHelloWorld = SDL_LoadBMP("D:\\Code\\CodeGame\\CodeGame\\background.bmp");
    if (gHelloWorld == NULL)
    {
        std::cout <<"Unable to load image %s! SDL Error: " << "D:\\Code\\CodeGame\\CodeGame\\background.bmp " << SDL_GetError() << std::endl;
        success = false;
    }

    return success;
}

//Giải phóng phương tiện và tắt SDL 
void close() {
    //Hủy bề mặt
    SDL_FreeSurface(gHelloWorld);
    gHelloWorld = NULL;

    //Destroy window (hủy cửa sổ)
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    //Quit SDL subsystems (Thoát khỏi hệ thống SDL)
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    //Start up SDL and create window (Khởi động SDL và tạo cửa sổ)
    if (!init())
    {
        std::cout << "Failed to initialize!" << std::endl;
    }
    else
    {
        //Load media (Tải media)
        if (!loadMedia())
        {
            std::cout << "Failed to load media!" << std::endl;
        }
        else
        {
            //Apply the image (Áp dụng hình ảnh)
            SDL_BlitSurface(gHelloWorld, NULL, gScreenSurface, NULL); //(hình ảnh nguồn, ..., đích, ...)
        }
        // Update surface(Cập nhật bề mặt)
        SDL_UpdateWindowSurface(gWindow);

        // Giữ cửa sổ luôn mở
        SDL_Event e; 
        bool quit = false; 
        while (quit == false) { 
            while (SDL_PollEvent(&e)) { 
                if (e.type == SDL_QUIT) 
                    quit = true; 
            } 
        }
    }
    //Free resources and close SDL (giải phóng tài nguyên và đóng SDL)
    close();
    return 0;
}