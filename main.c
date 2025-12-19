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
#define POS_Y_TIMER		112
#define POS_Y_CLOCK		135

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
	app_data->score.points[0] = POINT_00;
	app_data->score.points[1] = POINT_00;
	app_data->score.games[0] = 0;
	app_data->score.games[1] = 0;
	app_data->score.serving_player = -1;
	app_data->score.serving_player_tie_break = -1;
	_strcpy(app_data->score.previous_sets, "");
	app_data->score.advantage_count = 0;
	app_data->time_last_point = -1;
}

void init_score_history(struct app_data_* app_data) {
	app_data->score_history.next_point_index = 0;
	app_data->score_history.size = 0;
}

void copy_score(struct score_status* source, struct score_status* target) {
	target->serving_player = source->serving_player;
	target->serving_player_tie_break  = source->serving_player_tie_break;
	target->games[0] = source->games[0];
	target->games[1] = source->games[1];
	target->points[0] = source->points[0];
	target->points[1] = source->points[1];
	_strcpy(target->previous_sets, source->previous_sets);
	target->advantage_count = source->advantage_count;
}

void add_score_to_history(struct app_data_* app_data) {
	copy_score(&(app_data->score), &(app_data->score_history.scores[app_data->score_history.next_point_index]));

	if (app_data->score_history.next_point_index == SCORE_HISTORY_MAX_SIZE - 1) {
		app_data->score_history.next_point_index = 0;
	} else {
		++app_data->score_history.next_point_index;
	}

	if (app_data->score_history.size < SCORE_HISTORY_MAX_SIZE) {
		app_data->score_history.size++;
	}
}

int set_last_score(struct app_data_* app_data) {
	if (app_data->score_history.size == 0) {
		return false;
	}

	if (app_data->score_history.next_point_index == 0) {
		app_data->score_history.next_point_index = SCORE_HISTORY_MAX_SIZE - 1;
	} else {
		--app_data->score_history.next_point_index;
	}
	app_data->score_history.size--;

	copy_score(&(app_data->score_history.scores[app_data->score_history.next_point_index]), &(app_data->score));

	return true;
}

struct app_data_ * get_app_data() {
	// pointer to a pointer to screen data
	struct app_data_**  app_data_p = get_ptr_temp_buf_2();
	// pointer to screen data
	return *app_data_p;
}

void draw_clock(int set_bg) {
	// Draw clock

	if (set_bg == true) {
		set_bg_color(COLOR_BLACK);
		draw_filled_rect_bg(1, POS_Y_CLOCK, VIDEO_X, POS_Y_CLOCK + get_text_height());
	}

	set_fg_color(COLOR_AQUA);
	struct datetime_ datetime;
	get_current_date_time(&datetime);
	char time_text[10];
	_sprintf(time_text, "%02d:%02d:%02d", datetime.hour, datetime.min, datetime.sec);
	text_out_center(time_text, VIDEO_X/2, POS_Y_CLOCK);
}

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
	text_out_center(string_game, (int)pos_x[player_index], 52);
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

void draw_advantage_counter(int advantage_count, int points[2]) {
	// Check if we should display anything (deuce or advantage state)
	int is_deuce = (points[0] == POINT_40 && points[1] == POINT_40);
	int is_advantage = (points[0] == POINT_AD || points[1] == POINT_AD);

	if (!is_deuce && !is_advantage) {
		return;
	}

	// Only display if advantage_count > 0
	if (advantage_count == 0) {
		return;
	}

	char* text;
	int color;
	int needs_free = 0;

	// Determine what to display
	if (advantage_count == 2 && is_deuce) {
		// Star point: display "STAR" in cyan
		text = STAR_POINT_TEXT;
		color = COLOR_STAR_POINT;
	} else {
		// Numeric counter: display "N-AD" in light gray
		text = (char*)pvPortMalloc(10);
		_sprintf(text, "%d-AD", advantage_count);
		color = COLOR_ADVANTAGE_COUNTER;
		needs_free = 1;
	}

	// Load font and calculate centered position
	load_font();
	int text_w = text_width(text);
	int x = (VIDEO_X / 2) - (text_w / 2);
	int y = 84;

	// Render centered text
	text_out_font(FONT_LETTER_MIDDLE_5, text, x, y, color);

	// Free allocated memory if needed
	if (needs_free) {
		vPortFree(text);
	}
}

void draw_screen(int games[2], int serving_player, int serving_player_tie_break, int points[2], char *previous_sets, int advantage_count) {
	// Header
	set_bg_color(COLOR_BLACK);
	fill_screen_bg();
	load_font();
	set_fg_color(COLOR_YELLOW);
	text_out_center("Tennis Score", 88, 3);

	int pos_x[2] = { get_player_pos_x(0), get_player_pos_x(1) };

	// Sets
	set_fg_color(COLOR_WHITE);
	text_out_center(previous_sets, VIDEO_X/2, 30);

	// Games
	print_game(games, pos_x, 0, serving_player, serving_player_tie_break);
	print_game(games, pos_x, 1, serving_player, serving_player_tie_break);

	// Points
	print_point(points, pos_x, 0, serving_player, serving_player_tie_break);
	print_point(points, pos_x, 1, serving_player, serving_player_tie_break);

	// Advantage counter
	draw_advantage_counter(advantage_count, points);

	draw_clock(false);
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
		init_score_history(app_data);
	}

	// Here we draw the interface, updating (transferring to video memory) the screen is not necessary
	draw_screen(app_data->score.games, app_data->score.serving_player, app_data->score.serving_player_tie_break, app_data->score.points, app_data->score.previous_sets, app_data->score.advantage_count);

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

void draw_time_last_point(int time_last_point) {
	if (time_last_point != -1) {
		set_bg_color(COLOR_BLACK);
		draw_filled_rect_bg(1, POS_Y_TIMER, VIDEO_X, POS_Y_TIMER + get_text_height());
		load_font();
		set_fg_color(COLOR_GREEN);

		int diff_time = get_current_timestamp() - time_last_point;
		char string_diff_time[10];
		_sprintf(string_diff_time, "%d", diff_time);
		text_out_center(string_diff_time, VIDEO_X/2, POS_Y_TIMER);
	}
}

void screen_job() {
	// pointer to a pointer to screen data
	struct app_data_** 	app_data_p = get_ptr_temp_buf_2();
	// pointer to screen data
	struct app_data_ *	app_data = *app_data_p;

	draw_time_last_point(app_data->time_last_point);
	draw_clock(true);
	set_update_period(1, 1000);
}

void concat_set_result(char *dest, int games[2]) {
	char *separator;
	if (_strlen(dest) == 0) {
		separator = "";
	} else {
		separator = " ";
	}

	_sprintf(dest + _strlen(dest), "%s%d-%d", separator, games[0], games[1]);
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

			int tapped_index = -1;

			if (gest->touch_pos_x < VIDEO_X * 0.45) {
				tapped_index = 0;
			} else if (gest->touch_pos_x > VIDEO_X * 0.55) {
				tapped_index = 1;
			}

			if (tapped_index == -1) {
				break;
			}

			add_score_to_history(app_data);

			if (app_data->score.serving_player == -1) {
				app_data->score.serving_player = tapped_index;
			} else {
				int tapped_player = (app_data->score.serving_player+tapped_index)%2;

				app_data->time_last_point = get_current_timestamp();

				int other_player = (tapped_player + 1)%2;
				int tapped_player_score = app_data->score.points[tapped_player];
				int other_player_score = app_data->score.points[other_player];

				if (app_data->score.serving_player_tie_break != -1) {
					// Tie break
					if (tapped_player_score >= 6 && tapped_player_score > other_player_score) {
						// Tapped player wins the tie break

						app_data->score.games[tapped_player]++;

						// Update previous sets string
						concat_set_result(app_data->score.previous_sets, app_data->score.games);

						app_data->score.points[0] = POINT_00;
						app_data->score.points[1] = POINT_00;

						app_data->score.games[0] = 0;
						app_data->score.games[1] = 0;

						// Choose server at the beginning of each set
						app_data->score.serving_player = -1;
						app_data->score.serving_player_tie_break = -1;
					} else {
						if ((tapped_player_score + other_player_score)%2 == 0) {
							app_data->score.serving_player_tie_break = (app_data->score.serving_player_tie_break+1)%2;
						}
						app_data->score.points[tapped_player]++;
					}
				} else {
					if (tapped_player_score == POINT_40 && other_player_score == POINT_AD) {
						// Tapped player was in deuce and other player is in AD => set other player to deuce
						app_data->score.points[other_player] = POINT_40;
					} else if (tapped_player_score == POINT_AD || (tapped_player_score == POINT_40 && other_player_score < POINT_40)) {
						// Tapped player wins the game

						// Clear points
						app_data->score.points[0] = POINT_00;
						app_data->score.points[1] = POINT_00;
						// Add a game to the winner
						app_data->score.games[tapped_player]++;

						if (app_data->score.games[tapped_player] >= MAX_GAMES && app_data->score.games[tapped_player] - app_data->score.games[other_player] >= 2) {
							// Wins the set as well

							// Update previous sets string
							concat_set_result(app_data->score.previous_sets, app_data->score.games);

							app_data->score.games[0] = 0;
							app_data->score.games[1] = 0;

							// Choose server at the beginning of each set
							app_data->score.serving_player = -1;
						} else {
							// Change the server
							app_data->score.serving_player = (app_data->score.serving_player+1)%2;
						}

						// Tie break
						if (app_data->score.games[tapped_player] == MAX_GAMES && app_data->score.games[other_player] == MAX_GAMES) {
							app_data->score.serving_player_tie_break = app_data->score.serving_player;
						}
					} else {
						// Check if we're moving from deuce (40-40) to advantage
						if (tapped_player_score == POINT_40 && other_player_score == POINT_40) {
							// Just moved from deuce to advantage
							app_data->score.advantage_count++;
						}
						app_data->score.points[tapped_player]++;
					}
				}
			}

			// redraw the screen
			draw_screen(app_data->score.games, app_data->score.serving_player, app_data->score.serving_player_tie_break, app_data->score.points, app_data->score.previous_sets, app_data->score.advantage_count);
			repaint_screen_lines(1, VIDEO_Y);

			break;
		};
		case GESTURE_SWIPE_RIGHT: {
			break;
		};
		case GESTURE_SWIPE_LEFT: {
			if (set_last_score(app_data)) {
				vibrate(1, 70, 0);
				app_data->time_last_point = get_current_timestamp();
				// redraw the screen
				draw_screen(app_data->score.games, app_data->score.serving_player, app_data->score.serving_player_tie_break, app_data->score.points, app_data->score.previous_sets, app_data->score.advantage_count);
				repaint_screen_lines(1, VIDEO_Y);
			}
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
