#include <common.h>
#include <sys/time.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t do_sys_gettimeofday(struct timeval * tv, struct timezone * tz){
  uint64_t time = io_read(AM_TIMER_UPTIME).us;
  tv->tv_usec = (time % 1000000);
  tv->tv_sec = (time / 1000000);
	return 0;
}

size_t serial_write(const void *buf, size_t offset, size_t len) {
	for (int i = 0; i < len; ++i){
		putch(*((char *)buf + i));
	}
  return len;
}


static char buffer_for_key[1024];
size_t events_read(void *buf, size_t offset, size_t len) {
	if (len > 1024) assert(0);
	AM_INPUT_KEYBRD_T event = io_read(AM_INPUT_KEYBRD);
	if (event.keycode == AM_KEY_NONE) return 0;
	if (event.keydown) strcpy(buffer_for_key, "kd ");
	else strcpy(buffer_for_key, "ku ");
	strcat(buffer_for_key, keyname[event.keycode]);
	strcat(buffer_for_key, "\n");
	strncpy(buf + offset, buffer_for_key, len);
	if (strlen(buffer_for_key) < len) return strlen(buffer_for_key);
	else return len;
}

static int screen_w,screen_h;
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
	char buffer[128];
	sprintf(buffer, "WIDTH : %d\nHEIGHT:%d", screen_w, screen_h);
// 	Log("[debug] : w = %d, h = %d", screen_w, screen_h);
	strcpy(buf, buffer);
  return strlen(buffer);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
	Log("debug: offset = %d", offset);
  io_write(AM_GPU_MEMCPY, offset,(void *) buf, len);
  io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
  screen_w = io_read(AM_GPU_CONFIG).width;
  screen_h = io_read(AM_GPU_CONFIG).height;
}
