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


typedef uint32_t u32;
typedef uint8_t bool;

#define true 1
#define false 0

#define internal static // static functions are "internal"

// ---------------
// | Drawing lib |
// ---------------

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} rect_t;

internal void FillRect(rect_t rect, u32 pixel_color, u32 *screen_pixels)
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

// ---------------
// | Logging lib |
// ---------------

FILE *f;
internal void clear_log_file(void)
{
    f = fopen("log.txt", "w");
    fclose(f);
}

internal void log_to_file(char * log_msg)
{
    f = fopen("log.txt", "a");
    fprintf(f, log_msg);
    fclose(f);
}

#define MAX_LOG_MSG 1024
char log_msg[MAX_LOG_MSG];

// -----------------------
// | Logging game things |
// -----------------------
bool log_me_xy = false;

// ----------------------
// | Logging SDL things |
// ----------------------

internal void log_renderer_info(SDL_Renderer * renderer)
{
    // Get renderer info for logging.

    SDL_RendererInfo info;
    SDL_GetRendererInfo(
            renderer, // SDL_Renderer *
            &info // SDL_RendererInfo *
            );

    // Log name.
    log_to_file("Renderer info:\n");
    sprintf(log_msg, "\tname: %s\n", info.name);
    log_to_file(log_msg);

    // Log which flags are supported.
    log_to_file("\tSupported SDL_RendererFlags:\n");
    sprintf(log_msg,
            "\t\tSDL_RENDERER_SOFTWARE: %s\n",
            (info.flags & SDL_RENDERER_SOFTWARE) ? "True" : "False"
            );
    log_to_file(log_msg);
    sprintf(log_msg,
            "\t\tSDL_RENDERER_ACCELERATED: %s\n",
            (info.flags & SDL_RENDERER_ACCELERATED) ? "True" : "False"
            );
    log_to_file(log_msg);
    sprintf(log_msg,
            "\t\tSDL_RENDERER_PRESENTVSYNC: %s\n",
            (info.flags & SDL_RENDERER_PRESENTVSYNC) ? "True" : "False"
            );
    log_to_file(log_msg);
    sprintf(log_msg,
            "\t\tSDL_RENDERER_TARGETTEXTURE: %s\n",
            (info.flags & SDL_RENDERER_TARGETTEXTURE) ? "True" : "False"
            );
    log_to_file(log_msg);

    // Log texture info.
    sprintf(log_msg, "\tNumber of available texture formats: %d\n", info.num_texture_formats);
    log_to_file(log_msg);
    sprintf(log_msg, "\tMax texture width: %d\n", info.max_texture_width);
    log_to_file(log_msg);
    sprintf(log_msg, "\tMax texture height: %d\n", info.max_texture_height);
    log_to_file(log_msg);
}

int main(int argc, char **argv)
{
    clear_log_file();

    // ---------------
    // | Game window |
    // ---------------
    sprintf(log_msg, "Open game window: %dx%d... ", SCREEN_WIDTH, SCREEN_HEIGHT);
    log_to_file(log_msg);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *win = SDL_CreateWindow(
            "h,j,k,l", // const char *title
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, // int x, int y
            SCREEN_WIDTH, SCREEN_HEIGHT, // int w, int h,
            SDL_WINDOW_RESIZABLE // Uint32 flags
            );
    assert(win); log_to_file("OK\n");

    SDL_Renderer *renderer = SDL_CreateRenderer(
            win, // SDL_Window *
            0, // int index
            SDL_RENDERER_SOFTWARE // Uint32 flags
            );
    assert(renderer); log_renderer_info(renderer);


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

    bool done = false;

    // ---------------------------
    // | Game graphics that move |
    // ---------------------------

    // Me
    int me_w = SCREEN_WIDTH/50;
    int me_h = SCREEN_HEIGHT/50;
    rect_t me = {
        // Center me on the screen:
        SCREEN_WIDTH/2 - me_w/2,
        SCREEN_HEIGHT/2 - me_h,
        me_w,
        me_h
    };
    u32 me_color = 0x22FF00FF; // RGBA

    // ----------------------------------
    // | Game graphics that do not move |
    // ----------------------------------

    // Background
    rect_t bgnd = {0,0, SCREEN_WIDTH, SCREEN_HEIGHT};

    // Debug LED
    int debug_led_w = SCREEN_WIDTH/50;
    int debug_led_h = SCREEN_HEIGHT/50;
    rect_t debug_led = {
        SCREEN_WIDTH - debug_led_w, // top left x
        0,                          // top left y
        debug_led_w,                // width
        debug_led_h                 // height
    };

    // LED starts out green. Turns red on vertical wraparound.
    u32 debug_led_color = 0x00FF0000; // green

    // -----------------
    // | Game controls |
    // -----------------
    bool pressed_down  = false;
    bool pressed_up    = false;
    bool pressed_left  = false;
    bool pressed_right = false;


    while (!done) // GAME LOOP
    {
        // ----------------------
        // | Get keyboard input |
        // ----------------------

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                done = true;
            }

            SDL_Keycode code = event.key.keysym.sym;

            switch (code)
            {
                case SDLK_ESCAPE:
                    done = true;
                    break;

                case SDLK_j:
                    /* pressed_down = true; */
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
        }

        // --------
        // | DRAW |
        // --------

        // ---Clear the screen---
        u32 bgnd_color = 0x111100FF;
        FillRect(bgnd, bgnd_color, screen_pixels);
        //
        // Use memset for quickly making the screen black:
        /* memset(screen_pixels, 0, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(u32)); */

        // ---Draw me---
        //
        // Act on keypresses OUTSIDE the SDL_PollEvent loop!
            // If I calc me position inside the SDL_PollEvent loop, me only
            // responds to one keystroke at a time.
            // For example, press hj to move me diagonal down and left. If I
            // calc me position inside the SDL_PollEvent loop, the movement is
            // spread out over two iterations of the GAME LOOP.

        // TODO: control me speed
        // TODO: add small delay after initial press before repeating movement
        // TODO: change shape based on direction of movement
        if (pressed_down)
        {
            if ((me.y + me.h) < SCREEN_HEIGHT) // not at bottom yet
            {
                me.y += me.h;
                debug_led_color = 0x00FF0000; // green
            }
            else // wraparound
            {
                me.y = 0;
                debug_led_color = 0xFF000000; // red
            }
        }
        if (pressed_up)
        {
            if (me.y > me.h) // not at top yet
            {
                me.y -= me.h;
                debug_led_color = 0x00FF0000; // green
            }
            else // wraparound
            {
                me.y = SCREEN_HEIGHT - me.h;
                debug_led_color = 0xFF000000; // red
            }
        }
        if (pressed_left)
        {
            if (me.x > 0)
            {
                me.x -= me.w;
                debug_led_color = 0x00FF0000; // green
            }
            else // moving left, wrap around to right sight of screen
            {
                me.x = SCREEN_WIDTH - me.w;
                debug_led_color = 0xFF000000; // red
            }
        }
        if (pressed_right)
        {
            if (me.x < (SCREEN_WIDTH - me.w))
            {
                me.x += me.w;
                debug_led_color = 0x00FF0000; // green
            }
            else // moving right, wrap around to left sight of screen
            {
                me.x = 0;
                debug_led_color = 0xFF000000; // red
            }
        }
        if (log_me_xy)
        {
            if (pressed_down || pressed_up || pressed_left || pressed_right)
            {
                sprintf(log_msg, "me (x,y) = (%d, %d)\n", me.x, me.y);
                log_to_file(log_msg);
            }
        }

        FillRect(me, me_color, screen_pixels);

        // ---Draw debug led---
        FillRect(debug_led, debug_led_color, screen_pixels);

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
