#ifndef __APP_TEMPLATE_H__
#define __APP_TEMPLATE_H__

// Game scoring constants
#define POINT_00	0
#define POINT_15	1
#define POINT_30	2
#define POINT_40	3
#define POINT_AD	4
#define MAX_GAMES	6

// Display positioning constants
#define POS_Y_HEADER		3
#define POS_Y_SETS		30
#define POS_Y_GAMES		52
#define POS_Y_POINTS		89
#define POS_Y_TIMER		112
#define POS_Y_CLOCK		135
#define OFFSET_X_ADVANTAGE	2

// Point text rendering widths
#define POINT_TEXT_WIDTH_AD		19
#define POINT_TEXT_WIDTH_NORMAL	15

// Touch input thresholds
#define TAP_THRESHOLD_LEFT		0.45f
#define TAP_THRESHOLD_RIGHT		0.55f

// Game configuration
#define PLAYER_COUNT		2

// Memory and buffer constants
#define SCORE_HISTORY_MAX_SIZE 8
#define SETS_BUFFER_SIZE 200

// Color and display constants
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