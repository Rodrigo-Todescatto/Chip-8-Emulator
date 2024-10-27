#pragma once
#pragma warning(disable : 4996)
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <iostream>

using namespace std;

const unsigned int FONTSET_SIZE = 80;

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

//Variables
uint16_t stack[16];                 // Stack
uint16_t sp;                        // Stack pointer

uint8_t memory[4096];               // Memory (4k)
uint8_t V[16];                      // V registers (V0-VF)

uint16_t pc;                        // Program counter
uint16_t opcode;                    // Current op code
uint16_t I;                         // Index register

uint8_t delay_timer;                // Delay timer
uint8_t sound_timer;

uint8_t  gfx[64 * 32];              // Graphics buffer
uint8_t  key[16];

bool drawFlag;

void init();

//Initialize.
void init() {
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	for (int i = 0; i < 2048; ++i) {
		gfx[i] = 0;
	}

	for (int i = 0; i < 16; ++i) {
		stack[i] = 0;
		key[i] = 0;
		V[i] = 0;
	}

	for (int i = 0; i < 4096; ++i) {
		memory[i] = 0;
	}

	for (int i = 0; i < 80; ++i) {
		memory[i] = fontset[i];
	}

	delay_timer = 0;
	sound_timer = 0;

	srand(time(NULL));
}

//Loads ROM.
bool load(const char* file_path)
{
	init();

	FILE* ROM = fopen(file_path, "rb");

	if (ROM == NULL)
	{
		return false;
	}

	fseek(ROM, 0, SEEK_END);
	long ROM_SIZE = ftell(ROM);
	rewind(ROM);

	char* ROM_BUFFER = (char*)malloc(sizeof(char) * ROM_SIZE);
	if (ROM_BUFFER == NULL)
	{
		return false;
	}

	size_t result = fread(ROM_BUFFER, sizeof(char), (size_t)ROM_SIZE, ROM);
	if (result != ROM_SIZE)
	{
		return false;
	}

	if ((4096 - 512) > ROM_SIZE) {
		for (int i = 0; i < ROM_SIZE; ++i) {
			memory[i + 512] = (uint8_t)ROM_BUFFER[i];
		}
	}
	else {
		return false;
	}

	fclose(ROM);
	free(ROM_BUFFER);

	return true;
}

//Emulates the CPU.
void emulation_cycle()
{
	opcode = memory[pc] << 8 | memory[pc + 1];

	if (opcode & 0x0000)
	{
		if (0x000E)
		{
			--sp;
			pc = stack[sp];
			pc += 2;
		}

		for (int i = 0; i < 2048; ++i) {
			gfx[i] = 0;
		}
		drawFlag = true;
		pc += 2;
	}

	if (opcode & 0x1000)
	{
		pc = opcode & 0x0FFF;
	}

	if (opcode & 0x2000)
	{
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
	}

	if (opcode & 0x3000)
	{
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
	}

	if (opcode & 0x4000)
	{
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
	}

	if (opcode & 0x5000)
	{
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
	}

	if (opcode & 0x6000)
	{
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
	}

	if (opcode & 0x7000)
	{
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
	}

	if (opcode & 0x8000)
	{
		switch (opcode & 0x000F) {

		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001:
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002:
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003:
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004:
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
				V[0xF] = 1;
			else
				V[0xF] = 0;
			pc += 2;
			break;

		case 0x0005:
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
				V[0xF] = 0;
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0006:
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

		case 0x0007:
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
				V[0xF] = 0;
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x000E:
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;

		default:
			exit(3);
		}

	}

	if (opcode & 0x9000)
	{
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
	}

	if (opcode & 0xA000)
	{
		I = opcode & 0x0FFF;
		pc += 2;
	}

	if (opcode & 0xB000)
	{
		pc = (opcode & 0x0FFF) + V[0];
	}

	if (opcode & 0xC000)
	{
		V[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
		pc += 2;
	}

	if (opcode & 0xD000)
	{
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
					{
						V[0xF] = 1;
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
	}

	if (opcode & 0xE000)
	{
		switch (opcode & 0x00FF) {

			case 0x009E:
				if (key[V[(opcode & 0x0F00) >> 8]] != 0)
					pc += 4;
				else
					pc += 2;
				break;

			case 0x00A1:
				if (key[V[(opcode & 0x0F00) >> 8]] == 0)
					pc += 4;
				else
					pc += 2;
				break;

			default:
				exit(3);
		}
	}

	if (opcode & 0xF000)
	{
		switch (opcode & 0x00FF)
		{
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A:
		{
			bool key_pressed = false;

			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
				{
					V[(opcode & 0x0F00) >> 8] = i;
					key_pressed = true;
				}
			}

			if (!key_pressed)
				return;

			pc += 2;
		}
		break;

		case 0x0015:
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018:
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E:

			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
				V[0xF] = 1;
			else
				V[0xF] = 0;
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029:
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;

		case 0x0033:
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
			pc += 2;
			break;

		case 0x0055:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[I + i] = V[i];

			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		case 0x0065:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				V[i] = memory[I + i];

			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		default:
			printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
		}
	}

	if (delay_timer > 0)
		--delay_timer;

	if (sound_timer > 0)
		if (sound_timer == 1);
	// TODO: Implement sound
	--sound_timer;
}
