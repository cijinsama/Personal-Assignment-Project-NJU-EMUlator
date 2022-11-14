#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
	int i;
	gpu_texturedesc decoder = *((gpu_texturedesc *)VGACTL_ADDR);
	int w = decoder.h;
	int h = decoder.w;
	uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
	for (i = 0; i < w * h; i ++) fb[i] = i;
	outl(SYNC_ADDR, 1);
	panic("wowowo\n");
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
// 	uint16_t height = inw(VGACTL_ADDR);
// 	uint16_t width = inw(VGACTL_ADDR + 2);
//   *cfg = (AM_GPU_CONFIG_T) {
//     .present = true, .has_accel = false,
//     .width = width, .height = height,
//     .vmemsz = 0
//   };
	gpu_texturedesc decoder = *((gpu_texturedesc *)VGACTL_ADDR);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = decoder.h, .height = decoder.w,
    .vmemsz = 0
  };

}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
	panic("errpr\n\n\n");
	//int width = io_read(AM_GPU_CONFIG).width;
	gpuptr_t* pixels = ctl->pixels;
	gpuptr_t* fb = (gpuptr_t *)FB_ADDR;
	gpu_texturedesc decoder = *((gpu_texturedesc *)VGACTL_ADDR);
	for (int i = 0; i < ctl->h; i++){
		for (int j = 0;j < ctl->w; j++){
			fb[(ctl->y + i) * decoder.h + ctl->x + j] = pixels[i * ctl->w + j];
		}
	}
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
