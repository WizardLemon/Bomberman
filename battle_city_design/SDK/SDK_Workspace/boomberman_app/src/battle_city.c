#include "battle_city.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xio.h"
#include <math.h>
#include "macros.h"

// ***** GLOBAL VARIABLES *****
//unsigned char zameniSaExplosion = 6;

typedef enum {
	DIR_LEFT = 0, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_STILL
} direction_t;

// struktura koja sadrzi osobine protivnika

typedef struct bomb {
	unsigned char x; 	//x i y koordinata bombe
	unsigned char y;
	char tick_counter;
	unsigned char available;
	unsigned char placed;
}bomb_t;

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

void wait(long int milliseconds) {
	long int cycles = milliseconds*24000;
	while(--cycles);
}

static unsigned char bomberman_win(map_structure_t * map, bomberman_t *bomberman){
	if(bomberman->enemies_destroyed >= map->enemy_count) {
		if(((bomberman->y) == map->door_y) && ((bomberman->x) == map->door_x)){
			return 1;
		}
	}
	return 0;
}

// character promenljiva je tip karaktera koji treba postaviti na mapu
void char_spawn(bomberman_t * character) {
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + character->reg_l ),
			(unsigned int )0x8F000000 | (unsigned int )character->image);
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + character->reg_h ),
			((character->y)*16) << 16 | (character->x*16));
}


//OVA FUNKCIJA JEDINA PRIMA unsigned char map[30][40] DA BISIMO MOGLI DA JOJ PROSLEDIMO game_over/win mapu A DA NE MORAMO DA IH PAKUJEMO U map_structure_t
void draw_map(unsigned char map[30][40]) {
	long int addr;
	unsigned char x, y;
	for (y = 0; y < MAP_HEIGHT; y++) {							// base mapa
		for (x = 0; x < MAP_WIDTH; x++) {
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR
					+ 4 * (MAP_BASE_ADDRESS + y * MAP_WIDTH + x);
			switch (map[y][x]) {								//ovde menjam mapu
			case BACKGROUND:
				Xil_Out32(addr, IMG_16x16_background);
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
			case PLUS_BOMB:
				Xil_Out32(addr, IMG_16x16_plus_bomb);
				break;
			case PLUS_EXPLOSION:
				Xil_Out32(addr, IMG_16x16_plus_explosion);
				break;
			default:
				Xil_Out32(addr, IMG_16x16_background);
				break;
			}
		}
	}
}

static void map_update(map_structure_t * map, bomberman_t * bomberman) {
	bomberman->win_condition = bomberman_win(map, bomberman);
	if(bomberman->lives > 0 && bomberman->win_condition == 0){
		draw_map(map->map_grid);
	}else if(bomberman->lives <= 0) { //OVDE IMAMO BUG, AKO JE BOMBERMAN UMRO NA POCETNOM POLJU ONDA CE SE TU I RESPAVNOVATI I KONSTATNO CE UMIRATI
		bomberman->lose_condition = 1; //Ovo se koristi da bi smo onemogucili da se bomberman krece nakon izgubljene igre
		draw_map(map_game_over);
	}
	else if(bomberman->lives > 0 && bomberman->win_condition == 1) {	// game won mapa
		draw_map(map_win);
	}
}

static void map_reset() {
	unsigned int i;

	for (i = 0; i <= 20; i += 2) {
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + i ),
				(unsigned int )0x0F000000);
	}
}

static int find_bomberman(bomberman_t *bomberman, unsigned char x, unsigned char y, direction_t dir, int distance)
{

	unsigned char bombermanX = bomberman->x;
	unsigned char bombermanY = bomberman->y;

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

static int obstacles_detection(map_structure_t * map, int x, int y, direction_t dir, int position_distance) {

	if (dir == DIR_LEFT) {

		return map->map_grid[y][x - position_distance];;

	} else if (dir == DIR_RIGHT) {

		return map->map_grid[y][x + position_distance];

	} else if (dir == DIR_UP) {

		return map->map_grid[y - position_distance][x];

	} else if (dir == DIR_DOWN) {

		return map->map_grid[y + position_distance][x];

	} else {
		return -1;
	}
}

static void kill_bomberman(map_structure_t * map, bomberman_t * bomberman) {
	bomberman->lives--;
	map->map_grid[LIVES_STARTING_Y][LIVES_STARTING_X + (STARTING_LIFE_COUNT - bomberman->lives - 1)] = BACKGROUND; //OVA LINIJA BRISE ODGOVARAJUCE ZIVOTE
																										//PRVI IZGUBLJENI ZIVOT CE POMERITI X OSU ZA  + 0
	bomberman->x = map->bomberman_start_x;														//DRUGI IZGUBLJENI ZICOT CE POMERITI X OSU ZA + 1 I TO ZATO STO SE bomberman->lives smanjuje
	bomberman->y = map->bomberman_start_y;
	char_spawn(bomberman);
}

// Prototip funkcije za detekciju eksplozije
static unsigned char explosion_detection(map_structure_t * map, int x, int y, bomberman_t * bomberman, direction_t dir, int position_distance) {

	int bomberman_close = find_bomberman(bomberman, x, y, dir, position_distance);
	if(bomberman_close) {
		return BOMBERMAN;
	} else {
		switch(dir) {
		case DIR_LEFT:
			return map->map_grid[y][x - position_distance];
			break;
		case DIR_RIGHT:
			return map->map_grid[y][x + position_distance];
			break;
		case DIR_UP:
			return map->map_grid[y - position_distance][x];
			break;
		case DIR_DOWN:
			return map->map_grid[y + position_distance][x];
			break;
		default:
			return -1;
		}
	}
}

//Brisanje polja u prosledjenom pravcu na distanci distance
static void destroy_field(map_structure_t * map, unsigned char x, unsigned char y, bomberman_t * bomberman, direction_t dir, unsigned char distance) {
	switch(dir) {
	case DIR_LEFT:
		map->map_grid[y][x - distance] = BACKGROUND;
		break;
	case DIR_RIGHT:
		map->map_grid[y][x + distance] = BACKGROUND;
		break;
	case DIR_UP:
		map->map_grid[y - distance][x] = BACKGROUND;
		break;
	case DIR_DOWN:
		map->map_grid[y + distance][x] = BACKGROUND;
		break;
	default:;
	}
}

static void bomberman_move(map_structure_t * map, bomberman_t * bomberman, direction_t dir) {
	unsigned char x = bomberman->x;
	unsigned char y = bomberman->y;
	unsigned char obstacle = 0;

	obstacle = obstacles_detection(map, x, y, dir, 1);

	switch(obstacle) {
	case PLUS_BOMB:
		bomberman->bomb_count++;
		destroy_field(map, x, y, bomberman, dir, 1);
		goto move_label;
	case PLUS_EXPLOSION:
		bomberman->bomb_power++;
		destroy_field(map, x, y, bomberman, dir, 1);
		break;
	case BACKGROUND:
	case DOOR:
		//UNUTRASNJI SWITCH
		move_label:
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
		char_spawn(bomberman);
		break;

	case ENEMY:
		kill_bomberman(map, bomberman);
		break;
	default:;
	}

	wait(25);
}

static void destroy_enemy(map_structure_t * map, enemy_t * enm, int x, int y, direction_t dir, int distance){
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
		map->map_grid[enm->y][enm->x] = BACKGROUND;
		enm->destroyed = 1;
		enm->type = 0;
		enm->x = 0;
		enm->y = 0;
		bomberman->enemies_destroyed++;
	}

}

static void place_random_power_up(map_structure_t * map, unsigned char x, unsigned char y, bomberman_t * bomberman, direction_t direction, unsigned char distance) {
	unsigned char pom = rand();
	unsigned char power_up_type = 0;
	if(bomberman->bomb_count < BOMB_MAX_NUMBER && map->plus_bombs_placed < BOMB_MAX_NUMBER) {
		if(!(pom%PLUS_BOMB_CHANCE_MOD)) {
			map->plus_bombs_placed++;
			power_up_type = PLUS_BOMB;
			//OVO STOJI OVDE DA NE BISMO DVA POWER-UP-a NA ISTO POLJE STAVILI
		}
	} else if(bomberman->bomb_power < BOMB_MAX_POWER && map->plus_explosion_placed < BOMB_MAX_POWER) {
		if(!(pom%PLUS_EXPLOSION_CHANCE_MOD)) {
			map->plus_explosion_placed++;
			power_up_type = PLUS_EXPLOSION;
		}
	}
	switch(direction)  {
	case DIR_LEFT:
		map->map_grid[y][x - distance] = power_up_type;
		break;
	case DIR_RIGHT:
		map->map_grid[y][x + distance] = power_up_type;
		break;
	case DIR_UP:
		map->map_grid[y - distance][x] = power_up_type;
		break;
	case DIR_DOWN:
		map->map_grid[y + distance][x] = power_up_type;
		break;
	default:
		break;
	}
}



//OVO SE KORISTI ZA REKURZIVNI POZIV DA BI OTKRILI KOJU BOMBU AKTIVIRAMO SA DRUGOM BOMBOM
static unsigned char find_bomb_index(map_structure_t * map, unsigned char x, unsigned char y, bomberman_t * bomberman) {
	unsigned char i;
	for(i = 0; i < bomberman->bomb_count; i++) {
		if(bombs[i].placed) {
			if(bombs[i].x == x && bombs[i].y == y){
				return i;
			}
		}
	}
	return BOMB_MAX_NUMBER;
}

//PREIMENOVANO IZ DESTROY
static void detonate(map_structure_t * map, unsigned char x, unsigned char y, bomberman_t * bomberman, unsigned char bomb_index){
	unsigned char directions, i, j;
	unsigned char stop_flag;
	unsigned char explosion_obstacle;

	//OVDE DEAKTIVIRAMO BOMBU
	map->map_grid[y][x] = BACKGROUND;
	bomberman->active_bombs--;
	bombs[bomb_index].placed = 0;
	bombs[bomb_index].tick_counter = -1;
	bombs[bomb_index].x = 0;
	bombs[bomb_index].y = 0;
	//-

	for(directions = 0; directions < 4; directions++) {
		for(i = 0; i <= bomberman->bomb_power; i++) {
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
				place_random_power_up(map, x, y, bomberman, directions, i);
				stop_flag = 1;
				break;
			case BLOCK:
				stop_flag = 1;
				break;
			case ENEMY:
				//OVDE SAM UBACIO DA SE ITERIRA KROZ NEPRIJATELJE
				//TO NAM DAJE DA MOZEMO DA BIRAMO KOLIKO CEMO NEPRIJATELJA
				for(j = 0; j < map->enemy_count; j++) {
					if(!map->enemies[j].destroyed) {
						destroy_enemy(map, &map->enemies[j], x, y, (direction_t)directions, i);
					}
				}

				destroy_field(map, x, y, bomberman, (direction_t)directions, i);
				if(bomberman->enemies_destroyed == map->enemy_count) {
					map->map_grid[map->door_y][map->door_x] = DOOR;
					bomberman->win_condition = bomberman_win(map, bomberman);
				}
				break;
			case BOMB:
				bomb_label:
				stop_flag = 1;	//MORA SE AKTIVIRATI STOP FLAG ZATO STO BI INACE DVE UZASTOPNE BOMBE MOGLE DA ODUZMU DVA ZIVOTA
				switch((direction_t)directions) {
				case DIR_LEFT:
					detonate(map, x - i, y, bomberman, find_bomb_index(map, x - i, y, bomberman));
					break;
				case DIR_RIGHT:
					detonate(map, x + i, y, bomberman, find_bomb_index(map, x + i, y, bomberman));
					break;
				case DIR_UP:
					detonate(map, x, y - i, bomberman, find_bomb_index(map, x, y - i, bomberman));
					break;
				case DIR_DOWN:
					detonate(map, x, y + i, bomberman, find_bomb_index(map, x, y + i, bomberman));
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

static void place_bomb(map_structure_t * map, bomberman_t * bomberman) {
	unsigned char x = bomberman->x;
	unsigned char y = bomberman->y;
	unsigned char i;
	if(map->map_grid[y][x] == BACKGROUND && bomberman->active_bombs < bomberman->bomb_count) { //OVO JE DA NE BISMO MOGLI DA STAVLJAMO NA BOMBU
		bomberman->active_bombs++;
		for(i = 0; i < bomberman->bomb_count; i++) { //PROLAZIMO KROZ SVE BOMBE
			if(!bombs[i].placed) {
				//INICIJALIZACIJA BOMBE
				bombs[i].placed = 1;
				bombs[i].tick_counter = BOMB_TICK_COUNT;
				bombs[i].x = x;
				bombs[i].y = y;
				map->map_grid[y][x] = BOMB;
				break;
			}
		}
	}
}

static void check_and_detonate_bombs(map_structure_t * map, bomberman_t * bomberman) {
	unsigned char i;
	if(bomberman->active_bombs > 0){
		for(i = 0; i < bomberman->bomb_count; i++) {
			if(bombs[i].placed) { //PROVERAVAMO ZA SVAKU BOMBU DA LI JE POSTAVLJENA
				if(bombs[i].tick_counter-- == 0) {  // I DA LI JOJ JE ISTEKLO VREME
					detonate(map, bombs[i].x, bombs[i].y, bomberman, i);
				}
			}
		}
	}
}

static void check_and_move_enemies(map_structure_t * map, bomberman_t * bomberman) {
	unsigned char i;
	direction_t dir;
	unsigned char fb;
	unsigned char x, y;
	unsigned char obstacle;
	for(i = 0; i < map->enemy_count; i++) {
		x = map->enemies[i].x;
		y = map->enemies[i].y;
		dir = (direction_t)(rand()%4);
		fb = find_bomberman(bomberman, x, y, dir, 1);
		obstacle = obstacles_detection(map, x, y, dir, 1);
		if(!map->enemies[i].destroyed) {
			if(!((map->enemies[i].current_wait_cycle++) % (ENEMY_MAX_SPEED - map->enemy_speed))) {
				if(obstacle == BACKGROUND || (fb && obstacle != BOMB)) { //NEPRIJATELJ NE SME DA STANE NA POLJE NA KOJEM JE I BOMBERMAN I BOMBA
					map->map_grid[y][x] = BACKGROUND;
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
					map->map_grid[y][x] = map->enemies[i].type;
					map->enemies[i].x = x;
					map->enemies[i].y = y;
					if(fb) {
						kill_bomberman(map, bomberman);
					}
				}
			}
		}
	}
}

void init_battle(map_structure_t * map) {
	unsigned char i;

	for(i = 0; i < map->enemy_count; i++) {
		map->map_grid[map->enemies[i].y][map->enemies[i].x] = ENEMY;
	}
}

void battle_city(map_structure_t * map) {
	unsigned int buttons;

	bomberman_t player_one = {
		map->bomberman_start_x,	         				 // x trenutni
		map->bomberman_start_y, 		                     // y trenutni
		IMG_16x16_bomberman,
		0, // NOT DESTROYED
		STARTING_BOMB_POWER,
		STARTING_BOMB_NUMBER,               		 
		STARTING_LIFE_COUNT, 	
		0, //ENEMIES DESTROYED
		0,	//WIN CONDITION
		0,	//LOSE CONDITION
		0,	//ACIVE BOMBS
		0,	//INVERBABLE
		
		TANK1_REG_L,            		 // reg_l ?
		TANK1_REG_H             		 // reg_h ?
	};


	wait(500); // SETUP WAIT, POTREBNO DA SE NE BI DETEKTOVALO PRITISKANJE DUGMETA NAKON IZLASKA IZ MENIJA

	map_reset(map);

	init_battle(map);

	map_update(map, &player_one);

	char_spawn(&player_one);

	while (1) {
		buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );

		direction_t d = DIR_STILL;
		if(!bomberman->win_condition && !bomberman->lose_condition) {
			if (BTN_LEFT(buttons)) {
				d = DIR_LEFT;
			} else if (BTN_RIGHT(buttons)) {
				d = DIR_RIGHT;
			} else if (BTN_UP(buttons)) {
				d = DIR_UP;
			} else if (BTN_DOWN(buttons)) {
				d = DIR_DOWN;
			} else if(BTN_SHOOT(buttons)) {
				place_bomb(map, &player_one);
			}
		}

		check_and_detonate_bombs(map, &player_one);

		check_and_move_enemies(map, &player_one);

		map_update(map, &player_one);

		bomberman_move(map, &player_one, d);

		wait(50);
	}

}

