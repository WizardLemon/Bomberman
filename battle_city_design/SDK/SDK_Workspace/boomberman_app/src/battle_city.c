#include "battle_city.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xio.h"
#include <math.h>

	// MAP_OFFSET in battle_city.vhd
#define MAP_X					0
#define MAP_X2					640
#define MAP_Y					4
#define MAP_W					64
#define MAP_H					56

#define REGS_BASE_ADDRESS       ( MAP_BASE_ADDRESS + MAP_WIDTH * MAP_HEIGHT )
//#define REGS_BASE_ADDRESS     (5439)

#define TANK1_REG_L                     8
#define TANK1_REG_H                     9
#define BASE_REG_L						0
#define BASE_REG_H	                    1

#define BOMB_TICK_COUNT					20
#define BOMB_MAX_NUMBER					3
#define ENEMY_NUMBER					4
#define enemy1X 						19
#define enemy1Y 						3
#define enemy2X 						27
#define enemy2Y 						5
#define enemy3X 						20
#define enemy3Y 						17
#define enemy4X 						8
#define enemy4Y 						18
#define BOMBERMAN_STARTING_POSITION_X	8
#define BOMBERMAN_STARTING_POSITION_Y	3
#define DOOR_POSITION_X					18
#define DOOR_POSITION_Y					12
#define STARTING_LIFE_COUNT				3
#define ENEMY_WAIT_CYCLE_NUMBER			3
#define LIVES_STARTING_X				36
#define LIVES_STARTING_Y				2

// ***** GLOBAL VARIABLES *****
unsigned char enemies_destroyed = 0;
unsigned char win_condition = 0;
unsigned char lose_condition = 0;
unsigned char active_bombs;
unsigned char bomb_power;
unsigned char available_bombs;
//unsigned char zameniSaExplosion = 6;

typedef enum {
	DIR_LEFT = 0, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_STILL
} direction_t;

// struktura koja sadrzi osobine protivnika
typedef struct{
	unsigned char x;
	unsigned char y;
	unsigned char type;
	unsigned char current_wait_cycle; //NEPRIJATELJI SU SE PREBRZO KRETALI, TAKO DA SAM UVEO OVAJ WAIT CYCLE.
										//WIAT CYCLE SE MENJA SVAKI PUT KADA POKUSAMO DA POMERIMO NEPRIJATELJA I TO NA SLEDECI NACIN: (current_wait_cycle++)%ENEMY_WAIT_CYCLE_NUMBER
	unsigned char destroyed;
} enemy;

typedef struct bomb {
	unsigned char x; 	//x i y koordinata bombe
	unsigned char y;
	char tick_counter;
	unsigned char available;
	unsigned char placed;
}bomb_t;

bomberman_t player_one = {
		BOMBERMAN_STARTING_POSITION_X,	         				 // x trenutni
		BOMBERMAN_STARTING_POSITION_Y, 		                     // y trenutni
		IMG_16x16_bomberman,  			 // type

		0,                		 // nije destroyed
		STARTING_LIFE_COUNT, 	//BROJ POCETNIH ZIVOTA

		TANK1_REG_L,            		 // reg_l ?
		TANK1_REG_H             		 // reg_h ?
		};

//INICIJALIZACIJA BOMBI
bomb_t bombs[BOMB_MAX_NUMBER] = {
		{
				0,
				0,
				-1,
				0,
				0
		},
		{
				0,
				0,
				-1,
				0,
				0
		},
		{
				0,
				0,
				-1,
				0,
				0
		}
};

//ZAMENJENO ZA LISTU NEPRIJATELJA
enemy enemies[4] = {
		{
				enemy1X,						// x
				enemy1Y,						// y
				5,              		        // tip objekta, 5 je za protivnike
				0,								// wait cycle
				0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
		},

		{
				enemy2X,			    		// x
				enemy2Y,						// y
				5,              		        // tip objekta, 5 je za protivnike
				0,								// wait cycle
				0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
		},

		{
				enemy3X,						// x
				enemy3Y,						// y
				5,              		        // tip objekta, 5 je za protivnike
				0,								// wait cycle
				0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
		},

		{
				enemy4X,						// x
				enemy4Y,						// y
				5,              		        // tip objekta, 5 je za protivnike
				0,								// wait cycle
				0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
		}
};

void wait(long int milliseconds) {
	long int cycles = milliseconds*24000;
	while(--cycles);
}

static unsigned char bomberman_win(bomberman_t *bomberman){
	if(enemies_destroyed == ENEMY_NUMBER) {
		if((bomberman->y) == DOOR_POSITION_Y && (bomberman->x) == DOOR_POSITION_X){
			return 1;
		}
	}

	return 0;
}

// character promenljiva je tip karaktera koji treba postaviti na mapu
void char_spawn(unsigned char map[30][40], bomberman_t * character) {
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + character->reg_l ),
			(unsigned int )0x8F000000 | (unsigned int )character->image);
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + character->reg_h ),
			((character->y)*16) << 16 | (character->x*16));
}

void draw_map(unsigned char map[30][40]) {
	long int addr;
	unsigned char x, y;
	for (y = 0; y < MAP_HEIGHT; y++) {							// base mapa
		for (x = 0; x < MAP_WIDTH; x++) {
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR
					+ 4 * (MAP_BASE_ADDRESS + y * MAP_WIDTH + x);
			switch (map[y][x]) {								//ovde menjam mapu
			case BACKGROUND:
				Xil_Out32(addr, IMG_16x16_bckgnd);
				break;
			case BOMBERMAN:
				Xil_Out32(addr, IMG_16x16_bomberman);
				break;
			case BLOCK:
				Xil_Out32(addr, IMG_16x16_block);
				break;
			case BRICK:
				Xil_Out32(addr, IMG_16x16_brick);
				break;
			case DOOR:
				Xil_Out32(addr, IMG_16x16_door);
				break;
			case ENEMY:
				Xil_Out32(addr, IMG_16x16_enemy);
				break;
			case BOMB:
				Xil_Out32(addr, IMG_16x16_bomb);
				break;
			default:
				Xil_Out32(addr, IMG_16x16_bckgnd);
				break;
			}
		}
	}
}

static void map_update(unsigned char map[30][40], bomberman_t * bomberman) {
	win_condition = bomberman_win(bomberman);
	if(bomberman->lives > 0 && win_condition == 0){
		draw_map(map);
	}else if(bomberman->lives <= 0) { //OVDE IMAMO BUG, AKO JE BOMBERMAN UMRO NA POCETNOM POLJU ONDA CE SE TU I RESPAVNOVATI I KONSTATNO CE UMIRATI
		lose_condition = 1; //Ovo se koristi da bi smo onemogucili da se bomberman krece nakon izgubljene igre
		draw_map(map_game_over);
	}
	else if(bomberman->lives > 0 && win_condition == 1) {	// game won mapa
		draw_map(map_win);
	}
}

static void map_reset(unsigned char map[30][40]) {
	unsigned int i;

	for (i = 0; i <= 20; i += 2) {
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + i ),
				(unsigned int )0x0F000000);
	}
}

static int find_bomberman(bomberman_t *bomberman, int x, int y, direction_t dir, int distance)
{

	unsigned int bombermanX = bomberman->x;
	unsigned int bombermanY = bomberman->y;

	switch(dir) {
	case DIR_LEFT:
		if((x - distance) == bombermanX && y == bombermanY) {
			return 1;
		}
		break;
	case DIR_RIGHT:
		if((x + distance) == bombermanX && y == bombermanY) {
			return 1;
		}
		break;
	case DIR_UP:
		if(x == bombermanX && (y - distance) == bombermanY) {
			return 1;
		}
		break;
	case DIR_DOWN:
		if(x == bombermanX && (y + distance) == bombermanY) {
			return 1;
		}
		break;
	default:;
	}
	return 0;
}

static int obstacles_detection(unsigned char map[30][40], int x, int y, direction_t dir, int position_distance) {

	int position_right = map[y][x + position_distance];
	int position_left = map[y][x - position_distance];
	int position_up = map[y - position_distance][x];
	int position_down = map[y + position_distance][x];

	if (dir == DIR_LEFT) {

		return position_left;

	} else if (dir == DIR_RIGHT) {

		return position_right;

	} else if (dir == DIR_UP) {

		return position_up;

	} else if (dir == DIR_DOWN) {

		return position_down;

	} else {
		return -1;
	}
}

static void kill_bomberman(unsigned char map[30][40], bomberman_t * bomberman) {
	bomberman->lives--;
	map[LIVES_STARTING_Y][LIVES_STARTING_X + (STARTING_LIFE_COUNT - bomberman->lives - 1)] = BACKGROUND; //OVA LINIJA BRISE ODGOVARAJUCE ZIVOTE
																										//PRVI IZGUBLJENI ZIVOT CE POMERITI X OSU ZA  + 0
	bomberman->x = BOMBERMAN_STARTING_POSITION_X;														//DRUGI IZGUBLJENI ZICOT CE POMERITI X OSU ZA + 1 I TO ZATO STO SE bomberman->lives smanjuje
	bomberman->y = BOMBERMAN_STARTING_POSITION_Y;
	char_spawn(map, bomberman);
}

// Prototip funkcije za detekciju eksplozije
static int explosion_detection(unsigned char map[30][40], int x, int y, bomberman_t * bomberman, direction_t dir, int position_distance) {

	int position_right = map[y][x + position_distance];
	int position_left = map[y][x - position_distance];
	int position_up = map[y - position_distance][x];
	int position_down = map[y + position_distance][x];

	int bomberman_close = find_bomberman(bomberman, x, y, dir, position_distance);
	if(bomberman_close) {
		return BOMBERMAN;
	} else {
		switch(dir) {
		case DIR_LEFT:
			return position_left;
			break;
		case DIR_RIGHT:
			return position_right;
			break;
		case DIR_UP:
			return position_up;
			break;
		case DIR_DOWN:
			return position_down;
			break;
		default:
			return -1;
		}
	}
}


static void bomberman_move(unsigned char map[30][40], bomberman_t * bomberman, direction_t dir) {
	unsigned int x = bomberman->x;
	unsigned int y = bomberman->y;
	int obstacle = 0;

	obstacle = obstacles_detection(map, x, y, dir, 1);
	if(obstacle == BACKGROUND || obstacle == DOOR) {
		switch(dir) {
		case DIR_LEFT:
			x -= 1;
			break;
		case DIR_RIGHT:
			x += 1;
			break;
		case DIR_UP:
			y -= 1;
			break;
		case DIR_DOWN:
			y += 1;
			break;
		default:
			break;
		}
		bomberman->x = x;
		bomberman->y = y;
		char_spawn(map, bomberman);
	} else if(obstacle == ENEMY) {
		//KADA DODIRNEMO NEPRIJATELJA RESETUJEMO SE NA POCETNU i SMANJUJU SE ZIVOTI
		kill_bomberman(map, bomberman);

	}
	wait(25);
}

static void destroy_enemy(unsigned char map[30][40], enemy * enm, int x, int y, direction_t dir, int distance){
	unsigned char flag = 0;

	switch(dir) {
	case DIR_LEFT:
		if((enm->x == x - distance) && enm->y == y){
				flag = 1;
			}
		break;
	case DIR_RIGHT:
		if((enm->x == x + distance) && enm->y == y){
				flag = 1;
			}
		break;
	case DIR_UP:
		if(enm->x == x && enm->y == (y - distance)){
				flag = 1;
			}
		break;
	case DIR_DOWN:
		if(enm->x == x && enm->y == (y + distance)){
				flag = 1;
			}
		break;
	default:
		break;
	}

	if(flag) {
		map[enm->y][enm->x] = BACKGROUND;
		enm->destroyed = 1;
		enm->type = 0;
		enm->x = 0;
		enm->y = 0;
		enemies_destroyed++;
	}

}

//Brisanje polja u prosledjenom pravcu na distanci distance
static void destroy_field(unsigned char map[30][40], int x, int y, bomberman_t * bomberman, direction_t dir, int distance) {
	switch(dir) {
	case DIR_LEFT:
		map[y][x - distance] = BACKGROUND;
		break;
	case DIR_RIGHT:
		map[y][x + distance] = BACKGROUND;
		break;
	case DIR_UP:
		map[y - distance][x] = BACKGROUND;
		break;
	case DIR_DOWN:
		map[y + distance][x] = BACKGROUND;
		break;
	default:;
	}
}

//OVO SE KORISTI ZA REKURZIVNI POZIV DA BI OTKRILI KOJU BOMBU AKTIVIRAMO SA DRUGOM BOMBOM
static unsigned char find_bomb_index(unsigned char map[30][40], unsigned char x, unsigned char y) {
	unsigned char i;
	for(i = 0; i < available_bombs; i++) {
		if(bombs[i].placed) {
			if(bombs[i].x == x && bombs[i].y == y){
				return i;
			}
		}
	}
	return BOMB_MAX_NUMBER;
}

//PREIMENOVANO IZ DESTROY
static void detonate(unsigned char map[30][40], unsigned char x, unsigned char y, bomberman_t * bomberman, unsigned char bomb_index){
	unsigned char directions, i, j;
	unsigned char stop_flag;
	unsigned char explosion_obstacle;

	//OVDE DEAKTIVIRAMO BOMBU
	map[y][x] = BACKGROUND;
	active_bombs--;
	bombs[bomb_index].placed = 0;
	bombs[bomb_index].tick_counter = -1;
	bombs[bomb_index].x = 0;
	bombs[bomb_index].y = 0;
	//

	for(directions = 0; directions < 4; directions++) {
		for(i = 0; i <= bomb_power; i++) {
			stop_flag = 0;
			explosion_obstacle = explosion_detection(map, x, y, bomberman, (direction_t)directions, i);
			switch(explosion_obstacle) {
			case BACKGROUND:
				destroy_field(map, x, y, bomberman, (direction_t)directions, i);
				break;
			case BOMBERMAN:
				kill_bomberman(map, bomberman);
				if(obstacles_detection(map, x, y, (direction_t)directions, i) == BOMB)
					goto bomb_label;//AKO STOJIMO NA BOMBI MORAMO POKRITI OVAJ SLUCAJ
				break;
			case BRICK:
				destroy_field(map, x, y, bomberman, (direction_t)directions, i);
				stop_flag = 1;
				break;
			case BLOCK:
				stop_flag = 1;
				break;
			case ENEMY:
				//OVDE SAM UBACIO DA SE ITERIRA KROZ NEPRIJATELJE
				//TO NAM DAJE DA MOZEMO DA BIRAMO KOLIKO CEMO NEPRIJATELJA
				for(j = 0; j < ENEMY_NUMBER; j++) {
					if(!enemies[j].destroyed) {
						destroy_enemy(map, &enemies[j], x, y, (direction_t)directions, i);
					}
				}

				destroy_field(map, x, y, bomberman, (direction_t)directions, i);
				if(enemies_destroyed == 4) {
					map[DOOR_POSITION_Y][DOOR_POSITION_X] = DOOR;
					win_condition = bomberman_win(bomberman);
					if(win_condition) {
						map_update(map, bomberman);
					}
				}
				break;
			case BOMB:
				bomb_label:
				stop_flag = 1;	//MORA SE AKTIVIRATI STOP FLAG ZATO STO BI INACE DVE UZASTOPNE BOMBE MOGLE DA ODUZMU DVA ZIVOTA
				switch((direction_t)directions) {
				case DIR_LEFT:
					detonate(map, x - i, y, bomberman, find_bomb_index(map, x - i, y));
					break;
				case DIR_RIGHT:
					detonate(map, x + i, y, bomberman, find_bomb_index(map, x + i, y));
					break;
				case DIR_UP:
					detonate(map, x, y - i, bomberman, find_bomb_index(map, x, y - i));
					break;
				case DIR_DOWN:
					detonate(map, x, y + i, bomberman, find_bomb_index(map, x, y + i));
					break;
				default:;
				}
				break;
			default:;
			}

			if(stop_flag) {
				break;
			}
		}
	}
}

static void place_bomb(unsigned char map[30][40], bomberman_t * bomberman) {
	unsigned char x = bomberman->x;
	unsigned char y = bomberman->y;
	unsigned char i;
	if(map[y][x] == BACKGROUND && active_bombs < available_bombs) { //OVO JE DA NE BISMO MOGLI DA STAVLJAMO NA BOMBU
		active_bombs++;
		for(i = 0; i < available_bombs; i++) { //PROLAZIMO KROZ SVE BOMBE
			if(!bombs[i].placed) {
				//INICIJALIZACIJA BOMBE
				bombs[i].placed = 1;
				bombs[i].tick_counter = BOMB_TICK_COUNT;
				bombs[i].x = x;
				bombs[i].y = y;
				map[y][x] = BOMB;
				break;
			}
		}
	}
}

static void check_and_detonate_bombs(unsigned char map[30][40], bomberman_t * bomberman) {
	unsigned char i;
	if(active_bombs > 0){
		for(i = 0; i < available_bombs; i++) {
			if(bombs[i].placed) { //PROVERAVAMO ZA SVAKU BOMBU DA LI JE POSTAVLJENA
				if(bombs[i].tick_counter-- == 0) {  // I DA LI JOJ JE ISTEKLO VREME
					detonate(map, bombs[i].x, bombs[i].y, bomberman, i);
				}
			}
		}
	}
}

static void check_and_move_enemies(unsigned char map[30][40], bomberman_t * bomberman) {
	unsigned char i;
	direction_t dir;
	unsigned char fb;
	unsigned char x, y;
	unsigned char obstacle;
	for(i = 0; i < ENEMY_NUMBER; i++) {
		x = enemies[i].x;
		y = enemies[i].y;
		dir = (direction_t)(rand()%4);
		fb = find_bomberman(bomberman, x, y, dir, 1);
		obstacle = obstacles_detection(map, x, y, dir, 1);
		if(!enemies[i].destroyed) {
			if(!((enemies[i].current_wait_cycle++)%ENEMY_WAIT_CYCLE_NUMBER)) {
				if(obstacle == BACKGROUND || (fb && obstacle != BOMB)) { //NEPRIJATELJ NE SME DA STANE NA POLJE NA KOJEM JE I BOMBERMAN I BOMBA
					map[y][x] = BACKGROUND;
					switch(dir) {
					case DIR_LEFT:
						x -= 1;
						break;
					case DIR_RIGHT:
						x += 1;
						break;
					case DIR_UP:
						y -= 1;
						break;
					case DIR_DOWN:
						y += 1;
						break;
					default:;
					}
					map[y][x] = enemies[i].type;
					enemies[i].x = x;
					enemies[i].y = y;
					if(fb) {
						kill_bomberman(map, bomberman);
					}
				}
			}
		}
	}
}


void battle_city(map_structure_t * map) {
	unsigned int buttons;

	wait(500); // SETUP WAIT, POTREBNO DA SE NE BI DETEKTOVALO PRITISKANJE DUGMETA NAKON IZLASKA IZ MENIJA
	map_reset(map1);

	map_update(map1, &player_one);

	char_spawn(map1, &player_one);

	enemies_destroyed = 0;
	win_condition = 0;
	lose_condition = 0;
	bomb_power = 2;
	active_bombs = 0;
	available_bombs = BOMB_MAX_NUMBER;

	map1[enemy1Y][enemy1X] = ENEMY;

	map1[enemy2Y][enemy2X] = ENEMY;

	map1[enemy3Y][enemy3X] = ENEMY;

	map1[enemy4Y][enemy4X] = ENEMY;



	while (1) {
		buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );

		direction_t d = DIR_STILL;
		if(!win_condition && !lose_condition) {
			if (BTN_LEFT(buttons)) {
				d = DIR_LEFT;
			} else if (BTN_RIGHT(buttons)) {
				d = DIR_RIGHT;
			} else if (BTN_UP(buttons)){
				d = DIR_UP;
			} else if (BTN_DOWN(buttons)){
				d = DIR_DOWN;
			} else if(BTN_SHOOT(buttons)){
				place_bomb(map1, &player_one);
			}
		}

		check_and_detonate_bombs(map1, &player_one);

		check_and_move_enemies(map1, &player_one);

		map_update(map1, &player_one);

		bomberman_move(map1, &player_one, d);

		wait(50);

	}
}

