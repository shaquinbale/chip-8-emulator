#include "main.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>


uint8_t memory[MEMORY_SIZE] = {0};
uint8_t v[8] = {0};
uint16_t stack[16] = {0};
int display[32][64] = {0};

uint8_t delay = 0;
uint8_t sound = 0;

uint16_t i = 0;
uint16_t pc = MEMORY_START;
uint8_t sp = 0;

SDL_Window *window;
SDL_Surface *surface;
SDL_Event event;
SDL_Texture *texture;


int main(void) {
	init_sdl();
	init_chip8();

	bool quit = false;
	while (!quit) {
		execute();
		draw();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void init_sdl (){
	if (SDL_Init(SDL_INIT_EVERYTHING < 0)) fprintf(stderr, "Error initializing: %s\n", SDL_GetError());

	window = SDL_CreateWindow("Chip 8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, 0);
	if (window == NULL) fprintf(stderr, "Error creating window: %s\n", SDL_GetError());

	surface = SDL_GetWindowSurface(window);
	if (surface == NULL) fprintf(stderr, "Error getting window surface: %s\n", SDL_GetError());
}

void init_chip8() {
	FILE *rom = fopen("../roms/test01.ch8", "rb");
	fread(&memory[MEMORY_START], sizeof(uint8_t), MEMORY_SIZE - MEMORY_START, rom);
}

void execute() {
	int opcode = ((memory[pc] << 8) + memory[pc+1]);	
	int nnn = opcode & 0x0fff;
	int n = opcode & 0x000f;
	int x = opcode & 0x0f00;
	int y = opcode & 0x00f0;
	int kk = opcode & 0x00ff;

	switch(opcode & 0xf000) {

		case 0x0000:
			break;
		case 0x1000:
			break;
		case 0x2000:
			break;
		case 0x3000:
			break;
		case 0x4000:
			break;
		case 0x5000:
			break;
		case 0x6000:
			break;
		case 0x7000:
			break;
		case 0x8000:
			break;
		case 0x9000:
			break;
		case 0xa000:
			break;
		case 0xb000:
			break;
		case 0xc000:
			break;
		case 0xd000:
			break;
		case 0xe000:
			
		case 0xf000:
			break;
	}
}

void draw() {

}
