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

//Current displayed image ( Hình ảnh hiện tại được hiển thị )
SDL_Surface* gCurrentSurface = NULL;

//Key press surfaces constants( Hằng số bề mặt bàn phím )
enum KeyPressSurfaces {
    KEY_PRESS_SURFACE_DEFAULT,
    KEY_PRESS_SURFACE_UP,
    KEY_PRESS_SURFACE_DOWN,
    KEY_PRESS_SURFACE_LEFT,
    KEY_PRESS_SURFACE_RIGHT,
    KEY_PRESS_SURFACE_TOTAL,
};

//The images that correspond to a keypress ( Các hình ảnh tương ứng với một lần nhấn phím )
SDL_Surface* gKeyPressSurfaces[KEY_PRESS_SURFACE_TOTAL];

//Loads individual image ( Tải từng hình ảnh )
SDL_Surface* loadSurface(std::string path) {
    //Load image at specified path ( Tải hình ảnh lên theo đường dẫn )
    SDL_Surface* loadedSurface = SDL_LoadBMP(path.c_str());
    if (loadedSurface == NULL) {
        std::cout << "Unable to load image! SDL Error: " << path.c_str() << SDL_GetError() << std::endl;
    }
    return loadedSurface;
}

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

    //Tải bề mặt mặc định
    gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] = loadSurface("D:/Code/CodeGame/image/background.bmp");
    if (gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] == NULL) {
        std::cout << "Failed to load default image" << std::endl;
        success = false;
    }

    //Load up surface
    gKeyPressSurfaces[KEY_PRESS_SURFACE_UP] = loadSurface("D:/Code/CodeGame/image/up.bmp");
    if (gKeyPressSurfaces[KEY_PRESS_SURFACE_UP] == NULL) {
        std::cout << "Failed to load up image" << std::endl;
        success = false;
    }

    //Load down surface
    gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN] = loadSurface("D:/Code/CodeGame/image/down.bmp");
    if (gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN] == NULL) {
        std::cout << "Failed to load down image" << std::endl;
        success = false;
    }

    //Load right surface
    gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] = loadSurface("D:/Code/CodeGame/image/right.bmp");
    if (gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] == NULL) {
        std::cout << "Failed to load right image" << std::endl;
        success = false;
    }

    //Load left surface
    gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT] = loadSurface("D:/Code/CodeGame/image/left.bmp");
    if (gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT] == NULL) {
        std::cout << "Failed to load left image" << std::endl;
        success = false;
    }


    ////Tải ảnh "tung tóe"
    //gHelloWorld = SDL_LoadBMP("D:\\Code\\CodeGame\\CodeGame\\background.bmp");
    //if (gHelloWorld == NULL)
    //{
    //    std::cout <<"Unable to load image %s! SDL Error: " << "D:\\Code\\CodeGame\\CodeGame\\background.bmp " << SDL_GetError() << std::endl;
    //    success = false;
    //}

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
            SDL_BlitSurface(gCurrentSurface, NULL, gScreenSurface, NULL); //(hình ảnh nguồn, ..., đích, ...)
        }
        // Update surface(Cập nhật bề mặt)
        SDL_UpdateWindowSurface(gWindow);

        //Main loop flag (Cờ vòng lặp chính)
        bool quit = false;

        //Event handler (Trình xử lý sự kiện)
        SDL_Event e;

        //Set default current surface
        gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];

        //
        while ( !quit ) 
        {
            //Handle events on queue (Xử lý các sự kiện hàng đợi)
            while ( SDL_PollEvent(&e) != 0 ) 
            { 
                //User requests quit (Người dùng yêu cầu thoát)
                if ( e.type == SDL_QUIT )
                {
                    quit = true; 
                }
                //User presses a key (Người dùng nhấn một phím)
                else if (e.type == SDL_KEYDOWN) {
                    //Select surfaces based on key press (chọn bền mặt dựa trên phím người dùng nhấn)
                    switch (e.key.keysym.sym)
                    {
                    case SDLK_UP:
                        gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_UP];
                        break;
                        
                    case SDLK_DOWN:
                        gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN];
                        break;

                    case SDLK_LEFT:
                        gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT];
                        break;

                    case SDLK_RIGHT:
                        gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT];
                        break;

                    default:
                        gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];
                        break;

                    }
                }
            } 
            //Apply the image (Áp dụng hình ảnh)
            SDL_BlitSurface(gCurrentSurface, NULL, gScreenSurface, NULL);

            //Update the surface(Cập nhật bề mặt)
            SDL_UpdateWindowSurface(gWindow);
        }
    }
    //Free resources and close SDL (giải phóng tài nguyên và đóng SDL)
    close();
    return 0;
}