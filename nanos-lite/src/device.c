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

// struct timeval {
// 	 uint64_t			tv_sec;     /* seconds */
// 	 uint64_t			tv_usec;    /* microseconds */
// };
// 
// 
// struct timezone {
// 	int tz_minuteswest;     /* minutes west of Greenwich */
// 	int tz_dsttime;         /* type of DST correction */
// };

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

size_t events_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
