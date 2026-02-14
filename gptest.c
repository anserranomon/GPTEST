#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define STICK_RADIUS  40
#define DEADZONE      8000

typedef enum { STATE_IDLE, STATE_PRESSED, STATE_WAS_CLICKED } ButtonState;

typedef struct {
    SDL_Rect rect;
    int buttonID;
    const char* label;
    ButtonState state;
} VirtualButton;

float leftX = 0, leftY = 0, rightX = 0, rightY = 0;
ButtonState l3State = STATE_IDLE;
ButtonState r3State = STATE_IDLE;

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color, bool centered) {
    if (!text || text[0] == '\0' || !font) return;
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dstRect = { x, y, surface->w, surface->h };
    
    if (centered) {
        dstRect.x -= surface->w / 2;
        dstRect.y -= surface->h / 2;
    }

    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void draw_circle(SDL_Renderer* renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w, dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) SDL_RenderDrawPoint(renderer, x + dx, y + dy);
        }
    }
}

void apply_stick_color(SDL_Renderer* renderer, ButtonState state) {
    if (state == STATE_IDLE) 
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    else if (state == STATE_PRESSED) 
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    else 
        SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("GPTest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 18);
    
    SDL_Joystick* joystick = NULL;
    if (SDL_NumJoysticks() > 0) joystick = SDL_JoystickOpen(0);

    VirtualButton buttons[] = {
        {{500, 240, 45, 45}, 0, "A", STATE_IDLE},
        {{555, 185, 45, 45}, 1, "B", STATE_IDLE},      
        {{50, 30, 80, 40}, 4, "L1", STATE_IDLE},   
        {{140, 30, 80, 40}, 6, "L2", STATE_IDLE}, 
        {{420, 30, 80, 40}, 7, "R2", STATE_IDLE},  
        {{510, 30, 80, 40}, 5, "R1", STATE_IDLE},
        {{210, 220, 60, 25}, 12, "SEL", STATE_IDLE},
        {{295, 215, 50, 35}, 16, "FN", STATE_IDLE},
        {{370, 220, 60, 25}, 13, "STR", STATE_IDLE},
        {{500, 130, 45, 45}, 2, "X", STATE_IDLE},
        {{445, 185, 45, 45}, 3, "Y", STATE_IDLE}, 
        {{100, 130, 40, 40}, 8, "U", STATE_IDLE},
        {{100, 230, 40, 40}, 9, "D", STATE_IDLE}, 
        {{50, 180, 40, 40}, 10, "L", STATE_IDLE},
        {{150, 180, 40, 40}, 11, "R", STATE_IDLE}
    };
    int num_buttons = sizeof(buttons) / sizeof(VirtualButton);

    bool running = true;
    SDL_Event e;
    SDL_Color white = {255, 255, 255, 255};

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_JOYBUTTONDOWN || e.type == SDL_JOYBUTTONUP) {
                bool isDown = (e.type == SDL_JOYBUTTONDOWN);
                for(int i=0; i<num_buttons; i++) {
                    if(buttons[i].buttonID == e.jbutton.button) buttons[i].state = isDown ? STATE_PRESSED : STATE_WAS_CLICKED;
                }
                if (e.jbutton.button == 14) l3State = isDown ? STATE_PRESSED : STATE_WAS_CLICKED;
                if (e.jbutton.button == 15) r3State = isDown ? STATE_PRESSED : STATE_WAS_CLICKED;
            }
            if (e.type == SDL_JOYAXISMOTION) {
                float val = e.jaxis.value / 32767.0f;
                if (e.jaxis.axis == 0) leftX = (abs(e.jaxis.value) > DEADZONE) ? val : 0;
                if (e.jaxis.axis == 1) leftY = (abs(e.jaxis.value) > DEADZONE) ? val : 0;
                if (e.jaxis.axis == 2) rightX = (abs(e.jaxis.value) > DEADZONE) ? val : 0;
                if (e.jaxis.axis == 3) rightY = (abs(e.jaxis.value) > DEADZONE) ? val : 0;
            }
            // Condición de salida: L3 y R3 presionados
            if (l3State == STATE_PRESSED && r3State == STATE_PRESSED) running = false;
        }

        SDL_SetRenderDrawColor(renderer, 20, 22, 26, 255);
        SDL_RenderClear(renderer);

        // Buttons with text
        for (int i = 0; i < num_buttons; i++) {
            if (buttons[i].state == STATE_IDLE) SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            else if (buttons[i].state == STATE_PRESSED) SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
            else SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255);
            
            SDL_RenderFillRect(renderer, &buttons[i].rect);
            render_text(renderer, font, buttons[i].label, 
                        buttons[i].rect.x + buttons[i].rect.w / 2, 
                        buttons[i].rect.y + buttons[i].rect.h / 2, white, true);
        }

        int lx = 200, rx = 440, y_low = 380;
        
        // Left and Right Sticks
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        draw_circle(renderer, lx, y_low, STICK_RADIUS);
        apply_stick_color(renderer, l3State);
        draw_circle(renderer, lx + (leftX * 28), y_low + (leftY * 28), 12);
        render_text(renderer, font, "L3", lx, y_low - 55, white, true);

        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        draw_circle(renderer, rx, y_low, STICK_RADIUS);
        apply_stick_color(renderer, r3State);
        draw_circle(renderer, rx + (rightX * 28), y_low + (rightY * 28), 12);
        render_text(renderer, font, "R3", rx, y_low - 55, white, true);

        // Info to exit
        render_text(renderer, font, "Press L3 + R3 to exit...", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30, white, true);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (font) TTF_CloseFont(font);
    if (joystick) SDL_JoystickClose(joystick);
    TTF_Quit();
    SDL_Quit();
    return 0;
}