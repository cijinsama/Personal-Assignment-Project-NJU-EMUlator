#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
	if (inl(KBD_ADDR) == AM_KEY_NONE) {
		kbd->keydown = false;
		kbd->keycode = 0;
	}

	kbd->keydown = inl(KBD_ADDR) & KEYDOWN_MASK;
	kbd->keycode = inl(KBD_ADDR) & (~KEYDOWN_MASK);
	
}
