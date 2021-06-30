// Author: sustainablelab
// Source: Abner Coimbre - Programming a Text Editor in C (5 videos)
//     https://www.youtube.com/watch?v=TrR-suFO4to
// Cheatsheet:
//     Run with :make (shortcut ;m<Space>)
//     Open func sig in SDL header:
//         ;w Ctrl-]  - open in NEW WINDOW
//         ;w Shift-] - open in PREVIEW WINDOW
//     Open and omni-complete uses ctags.
//     Install ctags:
//          $ pacman -S ctags
//     I separate tags into TWO files for speedier tag updates.
//     Setup and update the tags files like this:
//          :make tags # update tags for project src only (no tags for dependencies -- FAST)
//          :make lib-tags # updates tags for header file dependencies (a bit slower)
//     Go to https://wiki.libsdl.org/SDL_blah for docs and examples

#include <assert.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_video.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 400

typedef Uint32 u32;
typedef Uint8 BOOL;

#define TRUE 1
#define FALSE 0

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} rect_t;

void FillRect(rect_t rect, u32 pixel_color, u32 *screen_pixels)
{
    assert(screen_pixels);
    for (int row=0; row < rect.h; row++)
    {
        for (int col=0; col < rect.w; col++)
        {
            screen_pixels[ (row + rect.y)*SCREEN_WIDTH + (col + rect.x) ] = pixel_color;
        }
    }
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *win = SDL_CreateWindow(
            "Spell Checker", // const char *title
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, // int x, int y
            SCREEN_WIDTH, SCREEN_HEIGHT, // int w, int h,
            SDL_WINDOW_RESIZABLE // Uint32 flags
            );
    assert(win);

    SDL_Renderer *renderer = SDL_CreateRenderer(
            win, // SDL_Window *
            0, // int index
            SDL_RENDERER_SOFTWARE // Uint32 flags
            );
    assert(renderer);

    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

    SDL_Texture *screen = SDL_CreateTexture(
            renderer, // SDL_Renderer *
            format->format, // Uint32 format,
            SDL_TEXTUREACCESS_STREAMING, // int access,
            SCREEN_WIDTH, SCREEN_HEIGHT // int w, int h
            );
    assert(screen);

    u32 *screen_pixels = (u32*) calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(u32));
    assert(screen_pixels);

    /* SDL_Delay(3000); */

    BOOL done = FALSE;


    int me_width = 10;
    int me_height = 20;
    rect_t me = {
        SCREEN_WIDTH/2 + me_width/2,
        SCREEN_HEIGHT/2 - me_height,
        me_width,
        me_height
    };

    while (!done)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                done = TRUE;
            }

            BOOL pressed_down  = FALSE;
            BOOL pressed_up    = FALSE;
            BOOL pressed_left  = FALSE;
            BOOL pressed_right = FALSE;

            SDL_Keycode code = event.key.keysym.sym;

            switch (code)
            {
                case SDLK_ESCAPE:
                    done = TRUE;
                    break;

                case SDLK_j:
                    /* pressed_down = TRUE; */
                    pressed_down = (event.type == SDL_KEYDOWN);
                    break;

                case SDLK_k:
                    pressed_up = (event.type == SDL_KEYDOWN);
                    break;

                case SDLK_h:
                    pressed_left = (event.type == SDL_KEYDOWN);
                    break;

                case SDLK_l:
                    pressed_right = (event.type == SDL_KEYDOWN);
                    break;

                default:
                    break;
            }

            if (pressed_down)
            {
                if ((me.y + me.h) < SCREEN_HEIGHT) // not at bottom yet
                {
                    me.y += me.h;
                }
                else // wraparound
                {
                    me.y = 0;
                }
            }
            if (pressed_up)
            {
                if (me.y > me.h) // not at top yet
                {
                    me.y -= me.h;
                }
                else // wraparound
                {
                    me.y = SCREEN_HEIGHT - me.h;
                }
            }
            if (pressed_left)
            {
                me.x -= me.w;
            }
            if (pressed_right)
            {
                me.x += me.w;
            }

        }

        // Clear the screen.
        /* memset(screen_pixels, 0, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(u32)); */
        rect_t bgnd = {0,0, SCREEN_WIDTH, SCREEN_HEIGHT};
        u32 bgnd_color = 0xFF0000FF;
        FillRect(bgnd, bgnd_color, screen_pixels);

        // Draw me.
        /* u32 pixel_color = 0x00FF00FF; // RGBA */
        u32 pixel_color = 0x80808000;
        FillRect(me, pixel_color, screen_pixels);

        SDL_UpdateTexture(
                screen,        // SDL_Texture *
                NULL,          // const SDL_Rect * - NULL updates entire texture
                screen_pixels, // const void *pixels
                SCREEN_WIDTH * sizeof(u32) // int pitch - n bytes in a row of pixel data
                );
        SDL_RenderClear(renderer);
        SDL_RenderCopy(
                renderer, // SDL_Renderer *
                screen,   // SDL_Texture *
                NULL, // const SDL_Rect * - SRC rect, NULL for entire TEXTURE
                NULL  // const SDL_Rect * - DEST rect, NULL for entire RENDERING TARGET
                );
        SDL_RenderPresent(renderer);

        SDL_Delay(15); // sets frame rate

    }

    return 0;
    // Why return 0? Tis what 'make' expects.
    //      Returning 0 avoids this:
    //      make: *** [Makefile:6: run] Error 144
}
