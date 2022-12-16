#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>


static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static uint32_t init_time = 0;

uint32_t NDL_GetTicks() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec /1000 - init_time;
}

int NDL_PollEvent(char *buf, int len) {
  int fp = open("/dev/events", O_RDONLY);
  return read(fp, buf, len);
}


static int canvas_w = 0, canvas_h = 0, canvas_off = 0, canvas_x = 0, canvas_y = 0;

void NDL_OpenCanvas(int *w, int *h) {
	if (*w | *h) {
		canvas_w = *w;
		canvas_h = *h;
	}
	else {
		canvas_w = screen_w;
		canvas_h = screen_h;
	}
	
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
	uint32_t offset = 0;
	int fb = open("/dev/fb", O_RDWR);
	for (int i = 0; i < h; i++){
		offset = (canvas_off + (i + y) * screen_w + x) * 4;
		write(fb, pixels + w * i, w * 4);
	}
	close(fb);
	return;
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
	//初始化时间
	struct timeval tv;
	gettimeofday(&tv, NULL);
	init_time = tv.tv_sec * 1000 + tv.tv_usec /1000;
	//初始化screen
  int fd = open("/proc/dispinfo", O_RDONLY);
	char buf[128];
	read(fd, buf, sizeof(buf));
	//获得屏幕宽与高
	int i = 0;
	for (i = 8; buf[i] != '\n'; i++){
		screen_w = screen_w * 10 + buf[i] - '0';
	}
	while(buf[i] < '0' || buf[i] > '9') i++;
	for (; buf[i] != '\0'; i++){
		screen_h = screen_h * 10 + buf[i] - '0';
	}
	close(fd);
  return 0;
}

void NDL_Quit() {
}
