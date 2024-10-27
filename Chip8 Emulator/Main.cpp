#include <chrono>
#include <thread>
#include "stdint.h"
#include "SDL.h"
#include "cpu.h"

uint8_t keymap[16] = {SDLK_x,SDLK_1,SDLK_2,SDLK_3,SDLK_q,SDLK_w,SDLK_e,SDLK_a,SDLK_s,SDLK_d,SDLK_z,SDLK_c,SDLK_4,SDLK_r,SDLK_f,SDLK_v,};

int main(int argc, char** argv)
{
    int windowWidth = 1280;                  
    int windowHeight = 720;                    

    SDL_Window* window = NULL;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        exit(1);
    }
    window = SDL_CreateWindow(
        "CHIP-8 Emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        exit(2);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, windowWidth, windowHeight);

    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        64, 32);

    uint32_t pixels[2048];

load:
    if (!load(argv[1]))
        return 2;

    while (true) {
        emulation_cycle();

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit(0);

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    exit(0);

                if (e.key.keysym.sym == SDLK_F1)
                    goto load;      

                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        key[i] = 1;
                    }
                }
            }
            if (e.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        key[i] = 0;
                    }
                }
            }
        }

        if (drawFlag) {
            drawFlag = false;

            for (int i = 0; i < 2048; ++i) {
                uint8_t pixel = gfx[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }
            SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        std::this_thread::sleep_for(std::chrono::microseconds(1200));

    }
}
