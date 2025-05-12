#ifndef SDL2_WRAPPER_H
#define SDL2_WRAPPER_H

#include <SDL.h>
#include <stdbool.h>

typedef struct {
  SDL_Window* window;
  SDL_Renderer* renderer;
  bool valid;
} GraphicContext;

static inline bool sdl2_init(GraphicContext* ctx, const char* title, int width,
                             int height) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("SDL_Init Error: %s", SDL_GetError());
    ctx->valid = false;
    return false;
  }

  ctx->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, width, height, 0);
  if (!ctx->window) {
    SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
    SDL_Quit();
    ctx->valid = false;
    return false;
  }

  ctx->renderer = SDL_CreateRenderer(
      ctx->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ctx->renderer) {
    SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
    SDL_DestroyWindow(ctx->window);
    SDL_Quit();
    ctx->valid = false;
    return false;
  }

  ctx->valid = true;
  return true;
}

static inline void sdl2_shutdown(GraphicContext* ctx) {
  if (ctx->renderer)
    SDL_DestroyRenderer(ctx->renderer);
  if (ctx->window)
    SDL_DestroyWindow(ctx->window);
  SDL_Quit();
  ctx->valid = false;
}

static inline void sdl2_clear(GraphicContext* ctx, SDL_Color color) {
  SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
  SDL_RenderClear(ctx->renderer);
}

static void sdl2_present(GraphicContext* ctx) {
  SDL_RenderPresent(ctx->renderer);
}

static void sdl2_draw_rect(GraphicContext* ctx, int mx, int my, int Mx, int My,
                           SDL_Color color) {
  SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
  SDL_Rect rect = {mx, my, Mx - mx, My - my};
  SDL_RenderDrawRect(ctx->renderer, &rect);
}

static void sdl2_draw_circle(GraphicContext* ctx, int cx, int cy, int radius,
                             SDL_Color color) {
  SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
  for (int dy = -radius; dy <= radius; dy++) {
    for (int dx = -radius; dx <= radius; dx++) {
      if (dx * dx + dy * dy <= radius * radius) {
        SDL_RenderDrawPoint(ctx->renderer, cx + dx, cy + dy);
      }
    }
  }
}

#endif // SDL2_WRAPPER_H
