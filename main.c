#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_image.h>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;
typedef float  f32;
typedef double f64;

typedef SDL_FPoint Vector2;

SDL_Window *window     = NULL;
SDL_Renderer *renderer = NULL;
 
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 620
#define HALF_WINDOW_WIDTH  (WINDOW_WIDTH  / 2)
#define HALF_WINDOW_HEIGHT (WINDOW_HEIGHT / 2)

bool float_equals(f32 a, f32 b) {
  return SDL_fabsf(a - b) < 0.0005f;
}

s32 GetRandomValue(s32 min, s32 max) {
  s32 value = 0;

  if (min > max) {
    s32 tmp = max;
    max = min;
    min = tmp;
  }

  value = (SDL_rand(100)%(abs(max - min) + 1) + min);
  return value;
}

bool circles_are_colliding(Vector2 center1, f32 radius1, Vector2 center2, f32 radius2) {
  f32 dx = center2.x - center1.x;
  f32 dy = center2.y - center1.y;

  f32 distanceSquared = dx*dx + dy*dy;
  f32 radiusSum = radius1 + radius2;

  bool collision = (distanceSquared <= (radiusSum*radiusSum));
  return collision;
}

void draw_circle(Vector2 center, f32 radius, SDL_Color c) {
  SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, SDL_ALPHA_OPAQUE);

  for(s32 x = -radius; x <= radius; x++) {
    for(s32 y = -radius; y <= radius; y++) {
      if(x*x + y*y < radius*radius) {
        SDL_RenderPoint(renderer, center.x + x, center.y + y);
      }
    }
  }
}

f32 Lerp(f32 start, f32 end, f32 amount) {
  f32 result = start + amount*(end - start);
  return result;
}

u32 default_button_width  = 384;
u32 default_button_height = 128;
u32 half_default_button_width  = 0;
u32 half_default_button_height = 0;

SDL_Texture* button_texture;
SDL_Texture* button_pressed;

SDL_Texture* slider_texture;
SDL_Texture* arrow_texture;

u8 scale = 4;

typedef struct {
  f32 mouse_x;
  f32 mouse_y;
  bool mouse_down;
  s32 hot_item;
  s32 active_item;
} UIState;

UIState ui_state = {0};
SDL_FRect button_rect = {0};
SDL_FRect slider_rect = {0};
SDL_FRect arrow_rect = {0};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "120");

  SDL_SetAppMetadata("IMGUI", "0.1", "com.douglasselias.imgui");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("IMGUI", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_srand(0);

  button_texture = IMG_LoadTexture(renderer, "../assets/button_blue.png");
  button_pressed = IMG_LoadTexture(renderer, "../assets/button_blue_flat.png");

  slider_texture = IMG_LoadTexture(renderer, "../assets/slider.png");
  arrow_texture = IMG_LoadTexture(renderer, "../assets/arrow_up.png");

  button_rect.x = HALF_WINDOW_WIDTH  - (button_texture->w / scale);
  button_rect.y = HALF_WINDOW_HEIGHT - (button_texture->h / scale);
  button_rect.w = (button_texture->w / 2);
  button_rect.h = (button_texture->h / 2);

  slider_rect.w = (slider_texture->w * 2);
  slider_rect.h = (slider_texture->h * 2);
  slider_rect.x = HALF_WINDOW_WIDTH  - (slider_rect.w / scale);
  slider_rect.y = HALF_WINDOW_HEIGHT - (slider_rect.h / scale);

  arrow_rect.w = (arrow_texture->w * 0.7);
  arrow_rect.h = (arrow_texture->h * 0.7);
  arrow_rect.x = HALF_WINDOW_WIDTH  - (arrow_rect.w / scale) - 80;
  arrow_rect.y = HALF_WINDOW_HEIGHT - (arrow_rect.h / scale) + 80;

  return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if(event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }

  if(event->type == SDL_EVENT_KEY_DOWN) {
    SDL_Scancode key_code = event->key.scancode;
    if(key_code == SDL_SCANCODE_ESCAPE || key_code == SDL_SCANCODE_Q) {
      return SDL_APP_SUCCESS;
    }

    if(key_code == SDL_SCANCODE_W) {}

    if(key_code == SDL_SCANCODE_J && !(event->key.repeat)) {}
  }

  if(event->type == SDL_EVENT_KEY_UP) {
    SDL_Scancode key_code = event->key.scancode;

    if(key_code == SDL_SCANCODE_W) {}
  }

  if(event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    ui_state.mouse_down = true;
  }

  if(event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
    ui_state.mouse_down = false;
  }

  return SDL_APP_CONTINUE;
}

u64 last_time = 0;

bool button(s32 id, SDL_FRect rect) {
  Vector2 mouse = {ui_state.mouse_x, ui_state.mouse_y};

  if(SDL_PointInRectFloat(&mouse, &rect)) {
    ui_state.hot_item = id;
    if(ui_state.active_item == false && ui_state.mouse_down) {
      ui_state.active_item = id;
    }
  }

  if(ui_state.hot_item == id) {
    if(ui_state.active_item == id) {
      SDL_SetTextureColorMod(button_pressed, 18, 114, 154);
      SDL_RenderTexture(renderer, button_pressed, NULL, &rect);
    } else {
      SDL_SetTextureColorMod(button_texture, 18, 114, 154);
      SDL_RenderTexture(renderer, button_texture, NULL, &rect);
    }
  } else {
    SDL_SetTextureColorMod(button_texture, 255, 255, 255);
    SDL_RenderTexture(renderer, button_texture, NULL, &rect);
  }

  if(ui_state.mouse_down == false
  && ui_state.hot_item == id
  && ui_state.active_item == id) {
    return true;
  }

  return false;
}

#define mapRange(a1,a2,b1,b2,s) (b1 + (s-a1)*(b2-b1)/(a2-a1))

SDL_AppResult SDL_AppIterate(void *appstate) {
  u64 now = SDL_GetTicks();
  f32 delta_time = ((f32) (now - last_time)) / 1000.0f;

  SDL_GetMouseState(&(ui_state.mouse_x), &(ui_state.mouse_y));

  ui_state.hot_item = 0;

  Vector2 mouse = {ui_state.mouse_x, ui_state.mouse_y};

  if(SDL_PointInRectFloat(&mouse, &arrow_rect) && ui_state.mouse_down) {
    f32 new_x = mouse.x - (arrow_rect.w / 2);
    f32 a_start = slider_rect.x;
    f32 a_end = slider_rect.x + slider_rect.w - (arrow_rect.w / 2);

    if(new_x >= a_start && new_x <= a_end) {
      arrow_rect.x = new_x;
      f32 value = mapRange(a_start, a_end, 0, 1, new_x);
      SDL_Log("Slider value: %0.2f\n", value);
    }
  }

  /********************* RENDERER *********************/
  SDL_SetRenderDrawColor(renderer, 15, 15, 15, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  // if(button(1, button_rect)) {
  //   SDL_Log("Clicked\n");
  // }

  SDL_RenderTexture(renderer, slider_texture, NULL, &slider_rect);
  SDL_RenderTexture(renderer, arrow_texture, NULL, &arrow_rect);

  SDL_SetRenderScale(renderer, scale, scale);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  // SDL_RenderDebugText(renderer, (button_rect.x/scale) + 5, (button_rect.y/scale) + (pressed ? 5 : 4), "Click");
  SDL_SetRenderScale(renderer, 1, 1);

  SDL_RenderPresent(renderer);
  last_time = now;

  if(ui_state.mouse_down == false) {
    ui_state.active_item = 0;
  }

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {}
