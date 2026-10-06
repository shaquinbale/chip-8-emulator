#include "main.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>


uint8_t memory[MEMORY_SIZE] = {0};
uint8_t v[16] = {0};
uint16_t stack[16] = {0};
int display[32][64] = {0};
uint16_t opcode;

uint8_t delay = 0;
uint8_t sound = 0;

uint16_t i = 0;
uint16_t pc = MEMORY_START;
uint8_t sp = 0;

SDL_Window *window;
SDL_Surface *surface;
SDL_Event event;
SDL_Texture *texture;
SDL_Renderer *renderer;


int main(void) {
	init_sdl();
	init_chip8();

	int death = 0;
	bool quit = false;
	while (!quit) {
		execute();
		draw();

		death++;
		if (death > 20) quit = true;
		SDL_Delay(100);
	}


	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void init_sdl (){
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Chip 8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, 0);
	surface = SDL_GetWindowSurface(window);
	renderer = SDL_CreateRenderer(window, -1, 0);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

	printf("SDL Succesfully Initialized\n");
}

void init_chip8() {
	FILE *rom = fopen("../roms/test01.ch8", "rb");
	fread(&memory[MEMORY_START], sizeof(uint8_t), MEMORY_SIZE - MEMORY_START, rom);

	printf("ROM succesfully initialized\n");
}

void execute() {
	// printf("opcode is 0x%x\n", opcode);
	uint8_t x, y, kk, n;
	uint16_t nnn;
	uint16_t sum; // Used for logging the carry

	opcode = (memory[pc] << 8 | memory[pc+1]);	

	nnn = opcode & 0x0fff;
	n = opcode & 0x000f;
	x = (opcode & 0x0f00) >> 8;
	y = (opcode & 0x00f0) >> 4;
	kk = opcode & 0x00ff;


	switch(opcode & 0xf000) {

		case 0x0000:
			printf("00e0\n");
			if (opcode == 0x00e0) {
				memset(display, 0, 32 * 64 * sizeof(int)); // 00E0
				pc += 2;
			}
			else { // 00EE
				sp--;
				pc = stack[sp];
				pc += 2;
			}
			break;

		case 0x1000: // 1nnn
			pc = nnn;
			break;

		case 0x2000: // 2nnn
			stack[sp] = pc;
			sp++;
			pc = nnn;
			break;

		case 0x3000: // 3xkk
			if (v[x] == kk) pc += 4;
			break;

		case 0x4000: // 4xkk
			if (v[x] != kk) pc +=4;
			break;

		case 0x5000: // 5xy0
			if (v[x] == v[y]) pc +=4;
			break;

		case 0x6000: // 6xkk
			printf("6xkk, 6%x%x\n", x, kk);
			v[x] = kk;
			pc += 2;
			break;

		case 0x7000: // 7xkk
			v[x] += kk;
			pc += 2;
			break;

		case 0x8000:
			switch (opcode & 0x000f) {

				case 0x0000: // 8xy0
					v[x] = v[y];
					pc += 2;
					break;

				case 0x0001: // 8xy1
					v[x] |= v[y];
					pc += 2;
					break;

				case 0x0002: // 8xy2
					v[x] &= v[y];
					pc += 2;
					break;

				case 0x0003: // 8xy3
					v[x] ^= v[y];
					pc += 2;
					break;
					
				case 0x0004: // 8xy4
					sum = v[x] + v[y];
					v[x] = sum & 0x00ff;
					v[0xf] = sum >> 8;
					pc += 2;
					break;

				case 0x0005: // 8xy5
					v[0xf] = v[x] > v[y] ? 1 : 0;
					v[x] = v[x] - v[y];
					pc += 2;
					break;

				case 0x0006: // 8xy6
					v[0xf] = (v[x] & 0x01) == 1 ? 1 : 0;
					v[x] >>= 1;
					pc += 2;
					break;

				case 0x0007: // 8xy7
					v[0xf] = v[y] > v[x] ? 1 : 0;
					v[x] = v[y] - v[x];
					pc += 2;
					break;

				case 0x000e: // 8xyE
					v[0xf] = (v[x] >> 7);
					v[x] <<= 1;
					pc += 2;
					break;
			}

		case 0x9000: // 9xy0
			if (v[x] != v[y]) pc += 2;
			pc += 2;
			break;

		case 0xa000: // Annn
			printf("6nnn, 6%x\n", nnn);
			i = nnn;
			pc += 2;
			break;

		case 0xb000: // Bnnn
			pc = nnn + v[0];
			break;

		case 0xc000: // Cxkk
			v[x] = (rand() % 256) & kk;
			pc += 2;
			break;

		case 0xd000: // Dxyn
			printf("Dxyn, D%x%x%x\n", x, y, n);
			v[0xf] = 0;
			for (int yline = 0; yline < n; yline++) {
				uint8_t row = memory[i + yline];

				for (int xline = 0; xline < 8; xline++) {
					if (row & (1 << (7 - xline))) {
						if (display[(y + yline) % 32][(x + xline) % 64] == 1) {
							v[0xf] = 1;
						}
						display[(y + yline) % 32][(x + xline) % 64] ^= 1;
					}
				}
			}
			pc += 2;
			break;

		case 0xe000:
			switch(opcode & 0x00ff) {
				case 0x009e:
				break;

				case 0x00a1:
				break;
			}

		case 0xf000:
			switch(opcode & 0x00ff) {
			}
	}
}

void draw() {
	uint32_t pixels[32][64] = {0};

	for(int x = 0; x < 64; x++) {
		for (int y = 0; y < 32; y++) {
			if (display[y][x] == 1) {
				pixels[y][x] = 0xFFFFFFFF;
			}
		}
	}

	SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}
