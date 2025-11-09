#pragma once

void* nanoshell_console_create(int width, int height, unsigned char* framebuffer);

void nanoshell_console_redraw(void* opaque, int x, int y, int width, int height);

void nanoshell_console_poll(void* opaque);

void nanoshell_console_set_ptrs(void* opaque, void* shutdownStatePtr, void* keyboardPtr, void* mousePtr);
