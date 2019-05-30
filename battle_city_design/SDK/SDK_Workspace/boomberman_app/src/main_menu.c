#include "main_menu.h"
#include "battle_city.h"
#include "xio.h"
#include "xparameters.h"
#include "xil_io.h"

#define SPEED_OPTION_X 24
#define SPEED_OPTION_Y 21
#define COUNT_OPTION_X 24
#define COUNT_OPTION_Y 16
#define ENEMY_MAX_NUMBER 9
#define ENEMY_MAX_SPEED 9

typedef enum {
	SPEED_OPTION = 0, COUNT_OPTION = 1
}option_t;

void print_number(unsigned char number, option_t option) {
	long int addr;
	unsigned char x, y;
	for (y = SPEED_OPTION_Y; y < SPEED_OPTION_Y + 5; y++) {							// base mapa
		for (x = SPEED_OPTION_X; x < SPEED_OPTION_X + 3; x++) {
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR
					+ 4 * (MAP_BASE_ADDRESS + y * MAP_WIDTH + x);
			Xil_Out32(addr, numbers[number][y][x]);
		}
	}
}

map_structure_t main_menu(){
	option_t menu_option = SPEED_OPTION;
	unsigned char enemy_number = 1;
	unsigned char enemy_speed = 1;
	unsigned char buttons = 0;

	bomberman_t menu_indicator;
	menu_indicator.x = COUNT_OPTION_X;
	menu_indicator.y = COUNT_OPTION_Y;
	draw_map(map_main_menu);

	while(1) {
		buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );
		if (BTN_LEFT(buttons)) {
			if(menu_option == SPEED_OPTION) {
				if(!(--enemy_speed)%ENEMY_MAX_SPEED) {
					enemy_speed = ENEMY_MAX_SPEED;
				}
			} else {
				if(!(--enemy_number)%ENEMY_MAX_NUMBER) {
					enemy_number = ENEMY_MAX_NUMBER;
				}
			}
		} else if (BTN_RIGHT(buttons)) {
			if(menu_option == SPEED_OPTION) {
				if(!(++enemy_speed)%ENEMY_MAX_SPEED) {
					enemy_speed = 1;
				}
			} else {
				if((++enemy_number)%ENEMY_MAX_NUMBER) {
					enemy_number = 1;
				}
			}
		} else if (BTN_UP(buttons)){
			if((--menu_option)%2 == SPEED_OPTION) {
				menu_indicator.y = SPEED_OPTION_Y;
			} else {
				menu_indicator.y = COUNT_OPTION_Y;
			}

		} else if (BTN_DOWN(buttons)){
			if((++menu_option)%2 == SPEED_OPTION) {
				menu_indicator.y = SPEED_OPTION_Y;
			} else {
				menu_indicator.y = COUNT_OPTION_Y;
			}

		} else if(BTN_SHOOT(buttons)){

		}

		print_number(enemy_number, SPEED_OPTION);
		print_number(enemy_number, COUNT_OPTION);
		char_spawn(map_main_menu, &menu_indicator);
		wait(300000);
	}
}
