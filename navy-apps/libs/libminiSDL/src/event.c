#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}



#define buffer_for_key_size 128
static char buffer_for_key[buffer_for_key_size];
int SDL_PollEvent(SDL_Event *ev) {
	if (!NDL_PollEvent(buffer_for_key, buffer_for_key_size)) return 0;
	switch (buffer_for_key[1]){
		case 'd': ev->type = SDL_KEYDOWN;ev->key.type = SDL_KEYDOWN; break;
		case 'u': ev->type = SDL_KEYUP;ev->key.type = SDL_KEYUP; break;
	}
	//获得按键
	int keys_num = sizeof(keyname) / sizeof(char *);
	for (int i = 0; i < keys_num; i++){
		if (strcmp(buffer_for_key + 3, keyname[i]) == 0){
			ev->key.keysym.sym = i;
			break;
		}
	}
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
