#include "main.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>


uint8_t memory[MEMORY_SIZE] = {0};
uint8_t v[16] = {0};
uint16_t stack[16] = {0};
int display[32][64] = {0};
bool keypad[16] = {false};

uint16_t opcode;
uint8_t delay = 0;
uint8_t sound = 0;

uint16_t i = 0;
uint16_t pc = MEMORY_START;
uint8_t sp = 0;

SDL_Window *window;
SDL_Event event;
SDL_Texture *texture;
SDL_Renderer *renderer;

bool quit = false;


int main(void) {
	init_sdl();
	init_chip8();


	while (!quit) {
		execute();
		draw();
		handle_input();

		//SDL_Delay(1);
	}


	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void init_sdl (){
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL_Init failed %s", SDL_GetError());
		exit(1);
	}

	window = SDL_CreateWindow("Chip 8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, 0);
	if (!window) {
	fprintf(stderr, "SDL_CreateWindow error: %s", SDL_GetError());
	exit(1);
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "SDL_CreateRenderer error: %s", SDL_GetError());
		exit(1);
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
	if (!texture) {
		fprintf(stderr, "SDL_CreateTexture error: %s", SDL_GetError());
		exit(1);
	}

	printf("SDL Succesfully Initialized\n");
}

void init_chip8() {
	FILE *rom = fopen("../roms/5-quirks.ch8", "rb");
	fread(&memory[MEMORY_START], sizeof(uint8_t), MEMORY_SIZE - MEMORY_START, rom);

	printf("ROM succesfully initialized\n");
}

void execute() {
	// printf("opcode is 0x%x\n", opcode);
	uint8_t x, y, kk, n;
	uint16_t nnn;
	uint16_t sum; // Used for logging the carry
	uint8_t carry;

	opcode = (memory[pc] << 8 | memory[pc+1]);	

	nnn = opcode & 0x0fff;
	n = opcode & 0x000f;
	x = (opcode & 0x0f00) >> 8;
	y = (opcode & 0x00f0) >> 4;
	kk = opcode & 0x00ff;


	switch(opcode & 0xf000) {

		case 0x0000:
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
			pc += (v[x] == kk) ? 4 : 2;
			break;

		case 0x4000: // 4xkk
			pc += (v[x] != kk) ? 4 : 2;
			break;

		case 0x5000: // 5xy0
			pc += (v[x] == v[y]) ? 4 : 2;
			break;

		case 0x6000: // 6xkk
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
					carry = v[x] >= v[y] ? 1 : 0;
					v[x] -= v[y];
					v[0xf] = carry;
					pc += 2;
					break;

				case 0x0006: // 8xy6
					carry = (v[x] & 0x01) == 1 ? 1 : 0;
					v[x] >>= 1;
					v[0xf] = carry;
					pc += 2;
					break;

				case 0x0007: // 8xy7
					carry = v[y] >= v[x] ? 1 : 0;
					v[x] = v[y] - v[x];
					v[0xf] = carry;
					pc += 2;
					break;

				case 0x000e: // 8xyE1
					v[x] = v[y];
					carry = (v[x] >> 7);
					v[x] <<= 1;
					v[0xf] = carry;
					pc += 2;
					break;
			}
			break;

		case 0x9000: // 9xy0
			if (v[x] != v[y]) pc += 2;
			pc += 2;
			break;

		case 0xa000: // Annn
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
			v[0xf] = 0;
			for (int yline = 0; yline < n; yline++) {
				uint8_t row = memory[i + yline];

				for (int xline = 0; xline < 8; xline++) {
					if (row & (1 << (7 - xline))) {
						if (display[(v[y] + yline) % 32][(v[x] + xline) % 64] == 1) {
							v[0xf] = 1;
						}
						display[(v[y] + yline) % 32][(v[x] + xline) % 64] ^= 1;
					}
				}
			}
			pc += 2;
			break;

		case 0xe000:
			switch(opcode & 0x00ff) {
				case 0x009e:
					if (keypad[v[x]] == 1) pc += 2;
					pc += 2;
					break;

				case 0x00a1:
					if (keypad[v[x]] == 0) pc +=2;
					pc += 2;
				break;
			}

		case 0xf000:
			switch(opcode & 0x00ff) {
				case 0x0007:
					v[x] = delay;
					pc += 2;
					break;

				case 0x000a:
					for (int key = 0; key < 16; key++) {
						if (keypad[key]) {
							v[x] = key;
							return;
						}
					}
					
					pc -= 2;
					break;

				case 0x015:
					delay = v[x];
					pc += 2;
					break;

				case 0x018:
					sound = v[x];
					pc += 2;
					break;

				case 0x01e:
					i += v[x];
					pc += 2;
					break;

				case 0x029:
					// Needs implemented
					break;

				case 0x033: // Does not pass test 3
					memory[i] = v[x] / 100;
					memory[i + 1] = (v[x] / 10) & 10;
					memory[i + 2] = v[x] & 10;
					pc += 2;
					break;

				case 0x055:
					for (int reg = 0; reg <= x; reg++) {
						memory[i + reg] = v[reg];
					}
					pc += 2;
					break;

				case 0x0065:
					for (int reg = 0; reg <= x; reg++) {
						v[reg] = memory[i + reg];
					}
					pc += 2;
					break;
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

void handle_input() {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			quit = true;
		}

		if (event.type == SDL_KEYDOWN) {
			switch(event.key.keysym.sym) {
				case SDLK_1: keypad[0x1] = 1; break;
				case SDLK_2: keypad[0x2] = 1; break;
				case SDLK_3: keypad[0x3] = 1; break;
				case SDLK_4: keypad[0xc] = 1; break;
				case SDLK_q: keypad[0x4] = 1; break;
				case SDLK_w: keypad[0x5] = 1; break;
				case SDLK_e: keypad[0x6] = 1; break;
				case SDLK_r: keypad[0xd] = 1; break;
				case SDLK_a: keypad[0x7] = 1; break;
				case SDLK_s: keypad[0x8] = 1; break;
				case SDLK_d: keypad[0x9] = 1; break;
				case SDLK_f: keypad[0xe] = 1; break;
				case SDLK_z: keypad[0xa] = 1; break;
				case SDLK_x: keypad[0x0] = 1; break;
				case SDLK_c: keypad[0xb] = 1; break;
				case SDLK_v: keypad[0xf] = 1; break;
			}
		}
		if (event.type == SDL_KEYUP) {
			switch(event.key.keysym.sym) {
				case SDLK_1: keypad[0x1] = 0; break;
				case SDLK_2: keypad[0x2] = 0; break;
				case SDLK_3: keypad[0x3] = 0; break;
				case SDLK_4: keypad[0xc] = 0; break;
				case SDLK_q: keypad[0x4] = 0; break;
				case SDLK_w: keypad[0x5] = 0; break;
				case SDLK_e: keypad[0x6] = 0; break;
				case SDLK_r: keypad[0xd] = 0; break;
				case SDLK_a: keypad[0x7] = 0; break;
				case SDLK_s: keypad[0x8] = 0; break;
				case SDLK_d: keypad[0x9] = 0; break;
				case SDLK_f: keypad[0xe] = 0; break;
				case SDLK_z: keypad[0xa] = 0; break;
				case SDLK_x: keypad[0x0] = 0; break;
				case SDLK_c: keypad[0xb] = 0; break;
				case SDLK_v: keypad[0xf] = 0; break;
			}
		}
	}
}
