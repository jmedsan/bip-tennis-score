#ifndef __APP_TEMPLATE_H__
#define __APP_TEMPLATE_H__

#define SCORE_HISTORY_MAX_SIZE 8
#define SETS_BUFFER_SIZE 200
#define COLOR_ADVANTAGE_COUNTER  0x808080
#define COLOR_STAR_POINT         COLOR_AQUA
#define STAR_POINT_TEXT          "STAR"

struct score_status {
	int serving_player;
	int serving_player_tie_break;
	int games[2];
	int points[2];
	char previous_sets[SETS_BUFFER_SIZE];
	int advantage_count;
};

struct score_history_status {
	struct score_status scores[SCORE_HISTORY_MAX_SIZE];
	int next_point_index;
	int size;
};

// the data structure for our screen
struct app_data_ {
	Elf_proc_* proc;
	// return function address
	void* ret_f;
	int time_last_point;
	struct score_status score;
	struct score_history_status score_history;
};

// template.c
void show_screen (void *return_screen);
void key_press_screen();
int dispatch_screen (void *param);
void screen_job();
#endif