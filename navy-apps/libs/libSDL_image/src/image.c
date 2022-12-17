#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  FILE *img = fopen(filename, "r");
  fseek(img, 0, SEEK_END);
  int size = ftell(img);
  char *content = calloc(1, size);
  fseek(img, 0, SEEK_SET);
  fread(content, size, 1, img);
  SDL_Surface *img_surface = STBIMG_LoadFromMemory(content, size);
  free(content);
  fclose(img);
  return img_surface;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
