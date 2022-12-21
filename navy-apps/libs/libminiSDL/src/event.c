#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define keyname(k) #k,

int SDL_SetKeyState(SDL_Event *ev);
static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
	assert(0);
  return 0;
}



static uint8_t keyboard_status[256];
#define buffer_for_key_size 16
static char buffer_for_key[buffer_for_key_size];
int SDL_PollEvent(SDL_Event *ev) {
	if (!NDL_PollEvent(buffer_for_key, buffer_for_key_size)) return 0;
	switch (buffer_for_key[1]){
		case 'd': ev->type = SDL_KEYDOWN; ev->key.type = SDL_KEYDOWN; break;
		case 'u': ev->type = SDL_KEYUP; ev->key.type = SDL_KEYUP; break;
	}
	int temp = 0;
	for (temp = 0; buffer_for_key[temp] != '\n'; temp++){}
	buffer_for_key[temp] = '\0';
	//获得按键
	int keys_num = sizeof(keyname) / sizeof(char *);
	for (int i = 0; i < keys_num; i++){
		if (strcmp(buffer_for_key + 3, keyname[i]) == 0){
			ev->key.keysym.sym = i;
			break;
		}
	}
	//如果更新按键状态，如果没有更新就返回0
  int ret = SDL_SetKeyState(ev);
	printf("debug ret = %d\n", ret);
	return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
	assert(0);
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
	assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
	//不确定这个实现对不对
	printf("Unkown right 查一下web\n");
	assert(0);
  if (numkeys){
		return keyboard_status;
	}
	assert(0);
	return NULL;
}
int SDL_SetKeyState(SDL_Event *ev) {
	printf("key : %s debug origin : %d, get : %d\n",keyname[ev->key.keysym.sym],keyboard_status[ev->key.keysym.sym], ev->key.type);
	if(keyboard_status[ev->key.keysym.sym] == ev->key.type) return 0;
	else{
		keyboard_status[ev->key.keysym.sym] == ev->key.type;
		return 1;
	}
}
