// KCTF-zhuceji.cpp : 开发者写出的注册机
//

#include "stdafx.h"
#include <stdint.h>
#include "md5.h"
#include <windows.h>
#include <math.h>


unsigned char ROL1(unsigned char x, unsigned char n) {
	return (x >> (8 - (n % 8))) | (x << (n % 8));
}


void threeBaseOperation(uint8_t* user_md5_part2_md5, uint8_t* step2_1) {
	// 三种基础运算

	// python: random.sample(range(1,32+1), 32)
	uint8_t H[] = { 18, 27, 32, 14, 8, 5, 17, 13, 26, 22, 25, 10, 1, 12, 30, 31, 23, 24, 21, 15, 19, 11, 16, 6, 7, 2, 4, 29, 28, 3, 9, 20 };
	uint8_t O[] = { 26, 28, 17, 24, 8, 12, 25, 1, 32, 11, 7, 16, 23, 2, 29, 21, 20, 27, 22, 18, 5, 30, 10, 0, 9, 3, 19, 13, 4, 6, 14, 31 };
	uint8_t K[] = { 21, 23, 22, 20, 19, 12, 4, 18, 3, 6, 16, 14, 10, 24, 28, 15, 31, 0, 11, 5, 8, 26, 13, 32, 30, 29, 17, 9, 25, 2, 1, 27 };

	for (int i = 0; i < 16; i++) {
		step2_1[i] = ROL1((user_md5_part2_md5[i] + H[i]), O[i]) ^ K[i];
	}
}

void multiplyUnderModule(uint8_t* step2_1, uint8_t* step2_2) {
	// 取模运算下的叠乘
	uint8_t key[] = { 233, 136, 189, 132, 157, 100, 196, 185, 138, 222, 90, 101, 115, 229, 161, 97 };
	for (int i = 0; i < 16; i++) {
		int temp = 0;
		for (int j = 0; j < 16; j++)
		{
			temp = (temp * key[i] + step2_1[j]) % 65423;
		}
		printf("%d, ", temp);
		step2_2[i * 2] = temp & 0xff;
		step2_2[i * 2 + 1] = (temp >> 8) & 0xff;
	}
	printf("\n\n\n");
}


unsigned int getBitCDM(const uint8_t n, int i) {
	unsigned int t = pow(2, i);
	return ((t & n) >> i) ? 1 : -1;
}

void CodeDivisionMultiplexingEncode(uint8_t* step2_2, uint8_t* step2_3) {
	int seq1[] = { -1, -1, -1, +1, +1, -1, +1, +1 };
	int seq2[] = { -1, -1, +1, -1, +1, +1, +1, -1 };
	int seq3[] = { -1, +1, -1, +1, +1, +1, -1, -1 };
	int seq4[] = { -1, +1, -1, -1, -1, -1, +1, -1 };

	int i, j, k;
	int8_t encoded[8][8][8];

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			for (k = 0; k < 8; k++) {
				encoded[i][j][k] =
					getBitCDM(*(uint8_t*)(step2_2 + i), 7 - j) * seq1[k] +
					getBitCDM(*(uint8_t*)(step2_2 + 8 + i), 7 - j) * seq2[k] +
					getBitCDM(*(uint8_t*)(step2_2 + 16 + i), 7 - j) * seq3[k] +
					getBitCDM(*(uint8_t*)(step2_2 + 24 + i), 7 - j) * seq4[k];
				//printf("%d ", encoded[i][j][k]);
			}
			//printf("\n");
		}
		//printf("\n\n");
	}

	// -4 -2 0 2 4
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			*(uint8_t*)(step2_3 + 24 * i + 3 * j + 0) = (((encoded[i][j][0] + 4) / 2) << 5) + (((encoded[i][j][1] + 4) / 2) << 2) + ((((encoded[i][j][2] + 4) / 2) >> 1) & 0x3);
			*(uint8_t*)(step2_3 + 24 * i + 3 * j + 1) = (((encoded[i][j][2] + 4) / 2) << 7) + (((encoded[i][j][3] + 4) / 2) << 4) + (((encoded[i][j][4] + 4) / 2) << 1) + ((((encoded[i][j][5] + 4) / 2) >> 2) & 0x1);
			*(uint8_t*)(step2_3 + 24 * i + 3 * j + 2) = (((encoded[i][j][5] + 4) / 2) << 6) + (((encoded[i][j][6] + 4) / 2) << 3) + (((encoded[i][j][7] + 4) / 2) << 0);
			//printf("%u %u %u\n", *(uint8_t*)(step2_3 + 24 * i + 3 * j + 0), *(uint8_t*)(step2_3 + 24 * i + 3 * j + 1), *(uint8_t*)(step2_3 + 24 * i + 3 * j + 2));
		}
		//printf("\n");
	}
}

int main()
{
	uint8_t user[] = "iyzyi";
	uint8_t user_md5[16];
	GetMD5(user, user_md5, 5);
	


	// 第二部分
	uint8_t user_md5_part2_md5[16];
	GetMD5(user_md5 + 8, user_md5_part2_md5, 8);

	uint8_t step2_1[16];
	threeBaseOperation(user_md5_part2_md5, step2_1);

	uint8_t step2_2[32];
	multiplyUnderModule(step2_1, step2_2);

	uint8_t step2_3[192];
	CodeDivisionMultiplexingEncode(step2_2, step2_3);

	for (int i = 0; i < 192; i++) {
		printf("%d, ", step2_3[i]);
	}
	printf("\n\n\n");

	system("pause");
	return 0;
}