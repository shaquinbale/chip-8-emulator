#ifndef MAIN_H
#define MAIN_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>


#define MEMORY_SIZE 4096
#define MEMORY_START 0x200


void init_sdl();
void init_chip8();
void execute();
void draw();



#endif
