#ifndef TIPE_GFX
#define TIPE_GFX

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "rect.h"
#include "stb_image_write.h"
#include <SDL.h>
#include <stdbool.h>

typedef struct {
  SDL_Window* window;
  SDL_Renderer* renderer;
  bool valid;
} Graphics;

static bool gfx_init(Graphics* ctx, const char* title, int width, int height) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("SDL_Init Error: %s", SDL_GetError());
    ctx->valid = false;
    return false;
  }

  ctx->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, width, height, 0);
  if (ctx->window == NULL) {
    SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
    SDL_Quit();
    ctx->valid = false;
    return false;
  }

  ctx->renderer = SDL_CreateRenderer(
      ctx->window, -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC |
          SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

  if (ctx->renderer == NULL) {
    SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
    SDL_DestroyWindow(ctx->window);
    SDL_Quit();
    ctx->valid = false;
    return false;
  }

  ctx->valid = true;
  return true;
}

static void gfx_free(Graphics* ctx) {
  if (ctx->renderer)
    SDL_DestroyRenderer(ctx->renderer);
  if (ctx->window)
    SDL_DestroyWindow(ctx->window);
  SDL_Quit();
  ctx->valid = false;
}

static void gfx_clear(Graphics* ctx, SDL_Color color) {
  SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
  SDL_RenderClear(ctx->renderer);
}

static void gfx_present(Graphics* ctx) { SDL_RenderPresent(ctx->renderer); }

static void gfx_draw_rect(Graphics* ctx, Rect* r, SDL_Color color) {
  SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
  SDL_Rect rect = {r->min[0], r->min[1], r->max[0] - r->min[0],
                   r->max[1] - r->min[1]};
  SDL_RenderDrawRect(ctx->renderer, &rect);
}

static void gfx_draw_circle(Graphics* ctx, int cx, int cy, int radius,
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

static void save_screenshot(Graphics* ctx, const char* filename) {
  int w, h;
  SDL_GetRendererOutputSize(ctx->renderer, &w, &h);

  size_t buffer_size = w * h * 4;
  unsigned char* pixels = malloc(buffer_size);
  unsigned char* converted = malloc(buffer_size);
  if (!pixels || !converted) {
    fprintf(stderr, "Memory allocation failed\n");
    return;
  }

  if (SDL_RenderReadPixels(ctx->renderer, NULL, SDL_PIXELFORMAT_RGBA32, pixels,
                           w * 4) != 0) {
    fprintf(stderr, "SDL_RenderReadPixels failed: %s\n", SDL_GetError());
    free(pixels);
    free(converted);
    return;
  }

  // Swizzle BGRA -> RGBA
  for (int i = 0; i < w * h; ++i) {
    converted[i * 4 + 0] = pixels[i * 4 + 0]; // R
    converted[i * 4 + 1] = pixels[i * 4 + 1]; // G
    converted[i * 4 + 2] = pixels[i * 4 + 2]; // B
    converted[i * 4 + 3] = 255;               // A
  }

  stbi_write_png(filename, w, h, 4, converted, w * 4);

  printf("Saved screenshot.\n");
  free(pixels);
  free(converted);
}

#endif
