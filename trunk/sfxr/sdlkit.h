/*
   Copyright (c) 2007 mjau/GerryJJ

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#ifndef SDLKIT_H
#define SDLKIT_H

#include "SDL.h"
#define ERROR(x) error(__FILE__, __LINE__, #x)
#define VERIFY(x) do { if (!(x)) ERROR(x); } while (0)
#include <stdio.h>
#include <string.h>

static void error (const char *file, unsigned int line, const char *msg)
{
	fprintf(stderr, "[!] %s:%u  %s\n", file, line, msg);
	exit(1);
}

typedef Uint32 DWORD;
typedef Uint16 WORD;

#define DIK_SPACE SDLK_SPACE
#define DIK_RETURN SDLK_RETURN
#define DIK_Z SDLK_z
#define DDK_WINDOW 0

#define hWndMain 0
#define hInstanceMain 0

#define Sleep(x) SDL_Delay(x)

static bool keys[SDLK_LAST];

void ddkInit();      // Will be called on startup
bool ddkCalcFrame(); // Will be called every frame, return true to continue running or false to quit
void ddkFree();      // Will be called on shutdown

class DPInput {
public:
	DPInput(int,int) {}
	~DPInput() {}
	static void Update () {}

	static bool KeyPressed(SDLKey key)
	{
		bool r = keys[key];
		keys[key] = false;
		return r;
	}

};

static Uint32 *ddkscreen32;
static Uint16 *ddkscreen16;
static int ddkpitch;
static int mouse_x, mouse_y, mouse_px, mouse_py;
static bool mouse_left = false, mouse_right = false, mouse_middle = false;
static bool mouse_leftclick = false, mouse_rightclick = false, mouse_middleclick = false;

static SDL_Surface *sdlscreen = NULL;

static void sdlupdate ()
{
	mouse_px = mouse_x;
	mouse_py = mouse_y;
	Uint8 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	bool mouse_left_p = mouse_left;
	bool mouse_right_p = mouse_right;
	bool mouse_middle_p = mouse_middle;
	mouse_left = buttons & SDL_BUTTON(1);
	mouse_right = buttons & SDL_BUTTON(3);
	mouse_middle = buttons & SDL_BUTTON(2);
	mouse_leftclick = mouse_left && !mouse_left_p;
	mouse_rightclick = mouse_right && !mouse_right_p;
	mouse_middleclick = mouse_middle && !mouse_middle_p;
}

static bool ddkLock ()
{
	if(SDL_MUSTLOCK(sdlscreen))
	{
		if(SDL_LockSurface(sdlscreen) < 0)
			return false;
	}
	ddkpitch = sdlscreen->pitch / (sdlscreen->format->BitsPerPixel == 32 ? 4 : 2);
	ddkscreen16 = (Uint16*)(sdlscreen->pixels);
	ddkscreen32 = (Uint32*)(sdlscreen->pixels);
	return true;
}

static void ddkUnlock ()
{
	if(SDL_MUSTLOCK(sdlscreen))
	{
		SDL_UnlockSurface(sdlscreen);
	}
}

static void ddkSetMode (int width, int height, int bpp, int refreshrate, int fullscreen, const char *title)
{
	VERIFY(sdlscreen = SDL_SetVideoMode(width, height, bpp, fullscreen ? SDL_FULLSCREEN : 0));
	SDL_WM_SetCaption(title, title);
}

#include <gtk/gtk.h>
#include <string.h>
#include <malloc.h>

static char *gtkfilename;

static void selected_file (GtkWidget *button, GtkFileSelection *fs)
{
	strncpy(gtkfilename, gtk_file_selection_get_filename(fs), 255);
	gtkfilename[255] = 0;
	gtk_widget_destroy(GTK_WIDGET(fs));
	gtk_main_quit();
}

static bool select_file (char *buf)
{
	gtkfilename = buf;
	GtkWidget *dialog = gtk_file_selection_new("It's file selection time!");
	g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(dialog)->ok_button), "clicked", G_CALLBACK(selected_file), G_OBJECT(dialog));
	g_signal_connect_swapped(G_OBJECT(GTK_FILE_SELECTION(dialog)->cancel_button), "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(dialog));
	gtk_widget_show(GTK_WIDGET(dialog));
	gtk_main();
	return *gtkfilename;
}

#define FileSelectorLoad(x,file,y) select_file(file)
#define FileSelectorSave(x,file,y) select_file(file)

static void sdlquit ()
{
	ddkFree();
	SDL_Quit();
}

static void sdlinit ()
{
	SDL_Surface *icon;
	VERIFY(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));
	icon = SDL_LoadBMP("/usr/share/sfxr/sfxr.bmp");
	if (!icon)
		icon = SDL_LoadBMP("sfxr.bmp");
	if (icon)
		SDL_WM_SetIcon(icon, NULL);
	atexit(sdlquit);
	memset(keys, 0, sizeof(keys));
	ddkInit();
}

static void loop (void)
{
	SDL_Event e;
	while (true)
	{
		SDL_PollEvent(&e);
		switch (e.type)
		{
			case SDL_QUIT:
				return;
	
			case SDL_KEYDOWN:
				keys[e.key.keysym.sym] = true;
	
			default: break;
		}
		sdlupdate();
		if (!ddkCalcFrame())
			return;
		SDL_Flip(sdlscreen);
	}
}

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);
	sdlinit();
	loop();
	return 0;
}

#endif
