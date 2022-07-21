/*
	Application template for Amazfit Bip BipOS
	(C) Maxim Volkov  2019 <Maxim.N.Volkov@ya.ru>
	
	Application template, header file
*/

#ifndef __APP_TEMPLATE_H__
#define __APP_TEMPLATE_H__

#define COLORS_COUNT 4

// the data structure for our screen
struct app_data_ {
	Elf_proc_* proc;
	// return function address
	void* ret_f;

	int serving_player;
	int serving_player_tie_break;
	int games[2];
	int points[2];
};

// template.c
void show_screen (void *return_screen);
void key_press_screen();
void long_key_press();
int dispatch_screen (void *param);
void screen_job();
void draw_screen(int games[2], int serving_player, int serving_player_tie_break, int points[2]);
#endif