#include "main_menu.h"
#include "battle_city.h"
#include "xio.h"
#include "xparameters.h"
#include "xil_io.h"

#define COUNT_OPTION_X 24
#define COUNT_OPTION_Y 18
#define SPEED_OPTION_X 24
#define SPEED_OPTION_Y 24
#define MENU_INDICATOR_COUNT_X COUNT_OPTION_X + 4
#define MENU_INDICATOR_COUNT_Y COUNT_OPTION_Y + 2
#define MENU_INDICATOR_SPEED_X MENUINDICATOR_COUNT_X
#define MENU_INDICATOR_SPEED_Y MENU_INDICATOR_COUNT_Y + 6

typedef enum {
	SPEED_OPTION = 0, COUNT_OPTION = 1
}option_t;

static void print_number(unsigned char number, option_t option) {
	long int addr;
	unsigned char i = -1, j = -1;
	unsigned char x, y;
	if(option == SPEED_OPTION) {
		for (y = SPEED_OPTION_Y; y < SPEED_OPTION_Y + 5; y++) {							// base mapa
			j++;
			for (x = SPEED_OPTION_X; x < SPEED_OPTION_X + 3; x++) {
				i++;
				addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR
						+ 4 * (MAP_BASE_ADDRESS + y * MAP_WIDTH + x);
				switch(numbers[number - 1][j%5][i%3]) {
				case BLOCK:
					Xil_Out32(addr, IMG_16x16_block);
					break;
				default:
					Xil_Out32(addr, IMG_16x16_bckgnd);
					break;
				}
			}
		}
	} else if(option == COUNT_OPTION) {
		for (y = COUNT_OPTION_Y; y < COUNT_OPTION_Y + 5; y++) {							// base mapa
			j++;
			for (x = COUNT_OPTION_X; x < COUNT_OPTION_X + 3; x++) {
				i++;
				addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR
						+ 4 * (MAP_BASE_ADDRESS + y * MAP_WIDTH + x);
				switch(numbers[number - 1][j%5][i%3]) {
				case BLOCK:
					Xil_Out32(addr, IMG_16x16_block);
					break;
				default:
					Xil_Out32(addr, IMG_16x16_bckgnd);
					break;
				}

			}
		}
	}

}

void main_menu(){
	unsigned char enemy_number = 1;
	unsigned char enemy_speed = 1;
	unsigned char buttons = 0;

	bomberman_t menu_indicator; //INICIJALIZACIJA POTREBNA ZA CHAR_SPAWN
	menu_indicator.x = MENU_INDICATOR_COUNT_X;
	menu_indicator.y = MENU_INDICATOR_COUNT_Y;
	menu_indicator.image = IMG_16x16_bomberman;
	menu_indicator.lives = 3;
	menu_indicator.reg_l = 	TANK1_REG_L;
	menu_indicator.reg_h = TANK1_REG_H;

	draw_map(map_main_menu);

	while(1) {
		buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );
		if (BTN_LEFT(buttons)) {
			if(menu_indicator.y == MENU_INDICATOR_SPEED_Y) {
				if(!(--enemy_speed)%ENEMY_MAX_SPEED) {
					enemy_speed = ENEMY_MAX_SPEED;
				}
			} else {
				if(!(--enemy_number)%ENEMY_MAX_NUMBER) {
					enemy_number = ENEMY_MAX_NUMBER;
				}
			}
		} else if (BTN_RIGHT(buttons)) {
			if(menu_indicator.y == MENU_INDICATOR_SPEED_Y) {
				if(!((++enemy_speed)%ENEMY_MAX_SPEED)) {
					enemy_speed = 1;
				}
			} else {
				if(!((++enemy_number)%ENEMY_MAX_NUMBER)) {
					enemy_number = 1;
				}
			}
		} else if (BTN_UP(buttons) || BTN_DOWN(buttons)){
			if(menu_indicator.y == MENU_INDICATOR_COUNT_Y) {
				menu_indicator.y = MENU_INDICATOR_SPEED_Y;
			} else {
				menu_indicator.y = MENU_INDICATOR_COUNT_Y;
			}
		} else if(BTN_SHOOT(buttons)){
			battle_city(&map_structures[0]);
		}

		print_number(enemy_speed, SPEED_OPTION);
		print_number(enemy_number, COUNT_OPTION);
		char_spawn(map_main_menu, &menu_indicator);
		wait(100);
	}
}
