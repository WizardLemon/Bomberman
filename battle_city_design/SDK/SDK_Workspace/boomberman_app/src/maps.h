#ifndef _MAPS_H_
#define _MAPS_H_

typedef struct map_structure {
	unsigned char map_grid[30][40];
	unsigned char bomberman_start_x;
	unsigned char bomberman_start_y;
	unsigned char enemy_speed;
	signed char enemy_count; //OVO NE SME DA PREVAZILAZI 10
	unsigned char enemy_start_x[10]; //Pocetna x
	unsigned char enemy_start_y[10];
} map_structure_t;

extern map_structure_t map[3];

extern unsigned char numbers[9][5][3];
extern unsigned char map_game_over[30][40];
extern unsigned char map_win[30][40];
extern unsigned char map_main_menu[30][40];
extern unsigned char map1[30][40];

#endif // _MAP_H_
