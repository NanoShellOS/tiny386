#include <nanoshell/nanoshell.h>
#include "nanoshell_console.h"
#include "i8042.h"

typedef struct NSRefreshEvent
{
	struct NSRefreshEvent* next;
	int x, y, width, height;
}
NSRefreshEvent;

typedef struct
{
	Window* window;
	NSRefreshEvent* refresh_head;
	uint32_t* framebuffer;
	uint32_t* temp_buffer;
	int width;
	int height;
	int* shutdown_state_ptr;
	PS2KbdState* keyboard_ptr;
	PS2MouseState* mouse_ptr;
	bool lmb_pressed;
	bool rmb_pressed;
}
NSConsole;

static void refresh_all(NSConsole* console)
{
	while (console->refresh_head)
	{
		NSRefreshEvent* re = console->refresh_head;
		console->refresh_head = console->refresh_head->next;
		
		uint32_t* curr_row = console->temp_buffer;
		for (int yi = 0; yi < re->height; yi++) {
			memcpy(curr_row, &console->framebuffer[console->width * (re->y + yi) + re->x], re->width * sizeof(uint32_t));
			curr_row += re->width;
		}
		
		Image img;
		img.width = re->width;
		img.height = re->height;
		img.framebuffer = console->temp_buffer;
		VidBlitImage(&img, re->x, re->y);
		
		free(re);
	}
}

static void update_keyboard_input(NSConsole* console, long parm1, UNUSED long parm2)
{
	// TODO: are these the right type of key codes?
	ps2_put_keycode(console->keyboard_ptr, (parm1 & 0x80) != 0, parm1 & 0x7f);
}

static void update_mouse_input(NSConsole* console, int eventType, long parm1, UNUSED long parm2)
{
	switch (eventType) {
		case EVENT_CLICKCURSOR:       console->lmb_pressed = true;  break;
		case EVENT_RELEASECURSOR:     console->lmb_pressed = false; break;
		case EVENT_RIGHTCLICK:        console->rmb_pressed = true;  break;
		case EVENT_RIGHTCLICKRELEASE: console->rmb_pressed = false; break;
	}
	
	int flags = 0;
	if (console->lmb_pressed) flags |= 1 << 0;
	if (console->rmb_pressed) flags |= 1 << 1;
	
	int x = GET_X_PARM(parm1);
	int y = GET_Y_PARM(parm1);
	ps2_mouse_event(console->mouse_ptr, x, y, 0, flags);
}

static void wnd_proc(Window* window, int eventType, long parm1, long parm2)
{
	NSConsole* console = (NSConsole*) GetWindowData(window);
	
	switch (eventType)
	{
		case EVENT_USER:
			refresh_all(console);
			break;
		
		case EVENT_KEYRAW:
			update_keyboard_input(console, parm1, parm2);
			break;
		
		case EVENT_MOVECURSOR:
		case EVENT_CLICKCURSOR:
		case EVENT_RELEASECURSOR:
		case EVENT_RIGHTCLICK:
		case EVENT_RIGHTCLICKRELEASE:
			update_mouse_input(console, eventType, parm1, parm2);
			break;
			
		default:
			DefaultWindowProc(window, eventType, parm1, parm2);
			break;
	}
}

void* nanoshell_console_create(int width, int height, unsigned char* framebuffer)
{
	NSConsole* console = malloc(sizeof(NSConsole));
	memset(console, 0, sizeof *console);
	if (!console) {
		LogMsg("Error, cannot allocate console.");
		return NULL;
	}
	
	console->temp_buffer = malloc(sizeof(uint32_t) * width * height);
	if (!console->temp_buffer) {
		LogMsg("Error, cannot allocate temp_buffer.");
		free(console);
		return NULL;
	}
	
	console->window = CreateWindow(
		"Tiny386",
		CW_AUTOPOSITION,
		CW_AUTOPOSITION,
		width,
		height,
		wnd_proc,
		0
	);
	
	if (!console->window) {
		LogMsg("Error, cannot create window.");
		free(console);
		return NULL;
	}
	
	console->framebuffer = (uint32_t*) framebuffer;
	console->width = width;
	console->height = height;
	SetWindowData(console->window, console);
	AddTimer(console->window, 20, EVENT_USER);
	return console;
}

void nanoshell_console_redraw(void* opaque, int x, int y, int width, int height)
{
	NSConsole* console = (NSConsole*) opaque;
	NSRefreshEvent* re = malloc(sizeof(NSRefreshEvent));
	if (!re)
		return;
	
	re->x = x;
	re->y = y;
	re->width = width;
	re->height = height;
	re->next = console->refresh_head;
	console->refresh_head = re;
}

void nanoshell_console_poll(void* opaque)
{
	NSConsole* console = (NSConsole*) opaque;
	int result = HandleMessages(console->window);
	if (!result) {
		// quit
		*console->shutdown_state_ptr = 8;
	}
}

void nanoshell_console_set_ptrs(void* opaque, void* shutdownStatePtr, void* keyboardPtr, void* mousePtr)
{
	NSConsole* console = (NSConsole*) opaque;
	console->shutdown_state_ptr = shutdownStatePtr;
	console->keyboard_ptr = keyboardPtr;
	console->mouse_ptr = mousePtr;
	
	LogMsg("console->shutdown_state_ptr: %p", console->shutdown_state_ptr);
	LogMsg("console->keyboard_ptr: %p", console->keyboard_ptr);
	LogMsg("console->mouse_ptr: %p", console->mouse_ptr);
}
