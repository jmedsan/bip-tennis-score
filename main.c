/*
	Application template for Amazfit Bip BipOS
	(C) Maxim Volkov  2019 <Maxim.N.Volkov@ya.ru>

	Application template for BipOS loader
*/

#include <libbip.h>
#include "main.h"

#define POINT_00	0
#define POINT_15	1
#define POINT_30	2
#define POINT_40	3
#define POINT_AD	4
#define MAX_GAMES	6
#define TIMER_Y		115

static char* ALL_POINTS[] = {"00", "15", "30", "40", "AD"};

// screen menu structure - for each screen is different
struct regmenu_ screen_data = {
	55,					// main screen number, value 0-255, for user windows it is better to take from 50 and above
	1,					// auxiliary screen number (usually equal to 1)
	0,
	dispatch_screen,	// Screen gesture processing function
	key_press_screen, 	// Side key processing function
	screen_job,			// Timer callback function
	0,
	show_screen,		// pointer to the screen display function
	0,
	0					// Long button press
};

int main(int param0, char** argv) {
	show_screen((void*) param0);
}

void clear_score(struct app_data_* app_data) {
	app_data->points[0] = POINT_00;
	app_data->points[1] = POINT_00;
	app_data->games[0] = 0;
	app_data->games[1] = 0;
	app_data->serving_player = 0;
	app_data->serving_player_tie_break = -1;
	app_data->time_last_point = -1;
}

struct app_data_ * get_app_data() {
	// pointer to a pointer to screen data
	struct app_data_**  app_data_p = get_ptr_temp_buf_2();
	// pointer to screen data
	return *app_data_p;
}

void show_screen (void *param0) {
	// pointer to a pointer to screen data
	struct app_data_** app_data_p = get_ptr_temp_buf_2();
	// pointer to screen data
	struct app_data_* app_data;

	// check the source of the procedure
	if ((param0 == *app_data_p) && get_var_menu_overlay()) {
		// return from the overlay screen (incoming call, notification, alarm, target, etc.)

		// the pointer to the data must be saved to exclude
		app_data = *app_data_p;
												// release memory with the reg_menu function
		*app_data_p = NULL;						// zero the pointer to pass to the reg_menu function

		// create a new screen, with the temp_buf_2 pointer equal to 0 and no memory free
		// menu_overlay=0
		reg_menu(&screen_data, 0);

		// restore the data pointer after the reg_menu function
		*app_data_p = app_data;

		// Here we perform actions when returning from the overlay screen: restore data, etc.
	} else { // if the function was started for the first time, i.e. from the menu
		// create a screen (register it in the system)
		reg_menu(&screen_data, 0);

		// allocate the necessary memory and place the data in it
		// (the memory by the pointer stored at the address temp_buf_2 is released automatically by the reg_menu function of the other screen)
		*app_data_p = (struct app_data_ *)pvPortMalloc(sizeof(struct app_data_));
		// data pointer
		app_data = *app_data_p;

		// Clear memory for data
		_memclr(app_data, sizeof(struct app_data_));

		//	значение param0 содержит указатель на данные запущенного процесса структура Elf_proc_
		app_data->proc = param0;

		// memorize the address of the pointer to the function to which you want to return after finishing this screen
		// If a pointer to the return is passed, then we return to it
		if (param0 && app_data->proc->elf_finish)
			app_data->ret_f = app_data->proc->elf_finish;
		// If not, on the dial
		else
			app_data->ret_f = show_watchface;

		// here we do what is necessary if the function is started for the first time from the menu: fill all data structures, etc.
		clear_score(app_data);
	}

	// Here we draw the interface, updating (transferring to video memory) the screen is not necessary
	draw_screen(app_data->games, app_data->serving_player, app_data->serving_player_tie_break, app_data->points);

	// In case of inaction, turn off the backlight and avoid app termination
	set_display_state_value(8, 1);
	set_display_state_value(2, 1);

	set_update_period(1, 1000);
}

void key_press_screen() {
	// pointer to a pointer to screen data
	struct app_data_**  app_data_p = get_ptr_temp_buf_2();
	// pointer to screen data
	struct app_data_ *	app_data = *app_data_p;

	// Call the return function (usually the start menu), and give the address of the function of our application as a parameter
	show_menu_animate(app_data->ret_f, (unsigned int)show_screen, ANIMATE_RIGHT);
}

void screen_job() {
	// pointer to a pointer to screen data
	struct app_data_** 	app_data_p = get_ptr_temp_buf_2();
	// pointer to screen data
	struct app_data_ *	app_data = *app_data_p;

	draw_time_last_point(app_data->time_last_point);
	set_update_period(1, 1000);
}

int dispatch_screen (void *param) {
	// pointer to a pointer to screen data
	struct app_data_** 	app_data_p = get_ptr_temp_buf_2();
	// pointer to screen data
	struct app_data_ *	app_data = *app_data_p;

	// in the case of rendering the interface, update (transfer to video memory) the screen must be performed
	struct gesture_ *gest = param;
	int result = 0;

	switch (gest->gesture) {
		case GESTURE_CLICK: {
			// tap
			vibrate(1, 40, 0);

			int tapped_player = -1;

			if (gest->touch_pos_x < VIDEO_X * 0.45) {
				tapped_player = app_data->serving_player;
			} else if (gest->touch_pos_x > VIDEO_X * 0.55) {
				tapped_player = (app_data->serving_player+1)%2;
			}

			if (tapped_player == -1) {
				break;
			}

			app_data->time_last_point = get_current_timestamp();

			int other_player = (tapped_player + 1)%2;
			int tapped_player_score = app_data->points[tapped_player];
			int other_player_score = app_data->points[other_player];

			if (app_data->serving_player_tie_break != -1) {
				// Tie break
				if (tapped_player_score >= 6 && tapped_player_score > other_player_score) {
					// Tapped player wins the tie break
					app_data->points[0] = POINT_00;
					app_data->points[1] = POINT_00;
					app_data->games[0] = 0;
					app_data->games[1] = 0;
					// Change the server
					app_data->serving_player = (app_data->serving_player+1)%2;
					app_data->serving_player_tie_break = -1;
				} else {
					if ((tapped_player_score + other_player_score)%2 == 0) {
						app_data->serving_player_tie_break = (app_data->serving_player_tie_break+1)%2;
					}
					app_data->points[tapped_player]++;
				}
			} else {
				if (tapped_player_score == POINT_40 && other_player_score == POINT_AD) {
					// Tapped player was in deuce and other player is in AD => set other player to deuce
					app_data->points[other_player] = POINT_40;
				} else if (tapped_player_score == POINT_AD || (tapped_player_score == POINT_40 && other_player_score < POINT_40)) {
					// Tapped player wins the game

					// Clear points
					app_data->points[0] = POINT_00;
					app_data->points[1] = POINT_00;
					// Add a game to the winner
					if (app_data->games[tapped_player] >= 5 && app_data->games[tapped_player] > app_data->games[other_player]) {
						// Wins the set as well
						app_data->points[0] = POINT_00;
						app_data->points[1] = POINT_00;
						app_data->games[0] = 0;
						app_data->games[1] = 0;
					} else {
						app_data->games[tapped_player]++;
					}

					// Change the server
					app_data->serving_player = (app_data->serving_player+1)%2;
					// Tie break
					if (app_data->games[tapped_player] == MAX_GAMES && app_data->games[other_player] == MAX_GAMES) {
						app_data->serving_player_tie_break = app_data->serving_player;
					}
				} else {
					app_data->points[tapped_player]++;
				}
			}

			// redraw the screen
			draw_screen(app_data->games, app_data->serving_player, app_data->serving_player_tie_break, app_data->points);
			repaint_screen_lines(1, VIDEO_Y);

			break;
		};
		case GESTURE_SWIPE_RIGHT: {
			break;
		};
		case GESTURE_SWIPE_LEFT: {
			break;
		};
		case GESTURE_SWIPE_UP: {
			break;
		};
		case GESTURE_SWIPE_DOWN: {
			break;
		};
		default:{
			// something went wrong
			break;
		};
	}

	return result;
}

// user code
int get_player_pos_x(int player_index) {
	float player_quarter = (float)((float)player_index + 0.5) * 2.0;
	float quarter_video_x = ((float)VIDEO_X / 4.0);
	float pos_x = player_quarter * quarter_video_x;
	return (int)pos_x;
}

void print_game(int games[2], int pos_x[2], int player_index, int serving_player, int serving_player_tie_break) {
	int color;
	char* format;
	int length;

	if (player_index == serving_player_tie_break || (serving_player_tie_break == -1 && player_index == serving_player)) {
		color = COLOR_YELLOW;
		format = "(%d)";
		length = 3;
	} else {
		color = COLOR_WHITE;
		format = "%d";
		length = 1;
	}
	
	char* string_game = (char*)pvPortMalloc(length + 1);
	_sprintf(string_game, format, games[player_index]);

	set_fg_color(color);
	text_out_center(string_game, (int)pos_x[player_index], 36);
}

void print_point(int points[2], int pos_x[2], int screen_index, int serving_player, int serving_player_tie_break) {
	int points_text_width;
	int player_index;

	if (screen_index == 0) {
		player_index = serving_player;
	} else {
		player_index = (serving_player+1)%2;
	}

	int player_points = points[player_index];
	char* text;
	if (serving_player_tie_break != -1) {
		text = (char*)pvPortMalloc(2);
		_sprintf(text, "%02d", player_points);
	} else {
		text = ALL_POINTS[player_points];
	}

	if (serving_player_tie_break == -1 && player_points == POINT_AD) {
		points_text_width = 19;
	} else {
		points_text_width = 15;
	}

	text_out_font(FONT_LETTER_BIG_6, text, pos_x[screen_index]-points_text_width, 89, 5);
}

void draw_time_last_point(int time_last_point) {
	if (time_last_point != -1) {
		set_bg_color(COLOR_BLACK);
		draw_filled_rect_bg(1, TIMER_Y, VIDEO_X, TIMER_Y + get_text_height());
		load_font();
		set_fg_color(COLOR_GREEN);

		int diff_time = get_current_timestamp() - time_last_point;
		char string_diff_time[10];
		_sprintf(string_diff_time, "%d", diff_time);
		text_out_center(string_diff_time, VIDEO_X/2, TIMER_Y);
	}
}

void draw_screen(int games[2], int serving_player, int serving_player_tie_break, int points[2]) {
	// Header
	set_bg_color(COLOR_BLACK);
	fill_screen_bg();
	load_font();
	set_fg_color(COLOR_YELLOW);
	text_out_center("Tennis Score", 88, 3);

	int pos_x[2] = { get_player_pos_x(0), get_player_pos_x(1) };

	// Games
	print_game(games, pos_x, 0, serving_player, serving_player_tie_break);
	print_game(games, pos_x, 1, serving_player, serving_player_tie_break);

	// Points
	print_point(points, pos_x, 0, serving_player, serving_player_tie_break);
	print_point(points, pos_x, 1, serving_player, serving_player_tie_break);

	// Footer
	set_fg_color(COLOR_AQUA);
	text_out_center("Tap to increase", 88, 150);
}
