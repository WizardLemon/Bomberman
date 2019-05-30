#ifndef _MAPS_H_
#define _MAPS_H_

#define MAP_COUNT 1
#define BOMBERMAN_STARTING_POSITION_1_X	8
#define BOMBERMAN_STARTING_POSITION_1_Y 3
#define ENEMY_DEFAULT_SPEED 1
#define ENEMY_DEFAULT_COUNT 1
#define ENEMY_MAX_NUMBER 9
#define ENEMY_MAX_SPEED 9

typedef struct map_structure {
	unsigned char map_grid[30][40];
	unsigned char bomberman_start_x;
	unsigned char bomberman_start_y;
	unsigned char enemy_speed;
	signed char enemy_count; //OVO NE SME DA PREVAZILAZI 10
	unsigned char enemy_start_x[ENEMY_MAX_NUMBER]; //Pocetna x
	unsigned char enemy_start_y[ENEMY_MAX_NUMBER];
} map_structure_t;

extern map_structure_t map_structures[MAP_COUNT];
extern unsigned char numbers[9][5][3];
extern unsigned char map_game_over[30][40];
extern unsigned char map_win[30][40];
extern unsigned char map_main_menu[30][40];
extern unsigned char map1[30][40];
extern unsigned char enemy_starting_positions_x[MAP_COUNT][ENEMY_MAX_NUMBER];
extern unsigned char enemy_starting_positions_y[MAP_COUNT][ENEMY_MAX_NUMBER];
extern unsigned char bomberman_starting_position_x[MAP_COUNT];
extern unsigned char bomberman_starting_position_y[MAP_COUNT];


#endif // _MAP_H_
