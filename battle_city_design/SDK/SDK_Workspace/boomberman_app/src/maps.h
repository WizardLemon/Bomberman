#ifndef _MAPS_H_
#define _MAPS_H_

#include "macros.h"

typedef struct enemy {
	unsigned char x;
	unsigned char y;
	unsigned char type;
	unsigned char current_wait_cycle; //NEPRIJATELJI SU SE PREBRZO KRETALI, TAKO DA SAM UVEO OVAJ WAIT CYCLE.
										//WIAT CYCLE SE MENJA SVAKI PUT KADA POKUSAMO DA POMERIMO NEPRIJATELJA I TO NA SLEDECI NACIN: (current_wait_cycle++)%ENEMY_WAIT_CYCLE_NUMBER
	unsigned char destroyed;
} enemy_t;

typedef struct map_structure {
	unsigned char map_grid[30][40];
	unsigned char bomberman_start_x;
	unsigned char bomberman_start_y;
	unsigned char door_x;
	unsigned char door_y;
	unsigned char plus_bombs_placed;
	unsigned char plus_explosion_placed;
	unsigned char enemy_speed;
	signed char enemy_count; //OVO NE SME DA PREVAZILAZI 10
	enemy_t enemies[ENEMY_MAX_NUMBER];
} map_structure_t;

extern map_structure_t map_structures[MAP_COUNT];
extern unsigned char numbers[9][5][3];
extern unsigned char map_game_over[30][40];
extern unsigned char map_win[30][40];
extern unsigned char map_main_menu[30][40];



#endif // _MAP_H_
