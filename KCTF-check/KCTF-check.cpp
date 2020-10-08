// KCTF-check.cpp : 验证用户名和注册码
//

#include "stdafx.h"
#include <stdint.h>
#include "md5.h"
#include <windows.h>
#include <math.h>


void CodeDivisionMultiplexingDecode(uint8_t* step2_3, uint8_t* step2_2) {		//码分复用解码
	memset(step2_2, 0, 32);					//这里把我坑死了。。。

	int seq1[] = { -1, -1, -1, +1, +1, -1, +1, +1 };
	int seq2[] = { -1, -1, +1, -1, +1, +1, +1, -1 };
	int seq3[] = { -1, +1, -1, +1, +1, +1, -1, -1 };
	int seq4[] = { -1, +1, -1, -1, -1, -1, +1, -1 };

	int i, j, k;
	int8_t decoded2[8][8][8];

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			decoded2[i][j][0] = ((*(uint8_t*)(step2_3 + 24 * i + 3 * j + 0) >> 5) & 0x7) * 2 - 4;
			decoded2[i][j][1] = ((*(uint8_t*)(step2_3 + 24 * i + 3 * j + 0) >> 2) & 0x7) * 2 - 4;
			decoded2[i][j][2] = (((*(uint8_t*)(step2_3 + 24 * i + 3 * j + 0) << 1) & 0x7) + ((*(uint8_t*)(step2_3 + 24 * i + 3 * j + 1) >> 7) & 0x1)) * 2 - 4;
			decoded2[i][j][3] = ((*(uint8_t*)(step2_3 + 24 * i + 3 * j + 1) >> 4) & 0x7) * 2 - 4;
			decoded2[i][j][4] = ((*(uint8_t*)(step2_3 + 24 * i + 3 * j + 1) >> 1) & 0x7) * 2 - 4;
			decoded2[i][j][5] = (((*(uint8_t*)(step2_3 + 24 * i + 3 * j + 1) << 2) & 0x7) + ((*(uint8_t*)(step2_3 + 24 * i + 3 * j + 2) >> 6) & 0x3)) * 2 - 4;
			decoded2[i][j][6] = ((*(uint8_t*)(step2_3 + 24 * i + 3 * j + 2) >> 3) & 0x7) * 2 - 4;
			decoded2[i][j][7] = ((*(uint8_t*)(step2_3 + 24 * i + 3 * j + 2)) & 0x7) * 2 - 4;
			for (k = 0; k < 8; k++) {
				//printf("%d ", decoded2[i][j][k]);
			}
			//printf("\n");
		}
		//printf("\n\n");
	}

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			int t1 = 0, t2 = 0, t3 = 0, t4 = 0;
			for (k = 0; k < 8; k++) {
				t1 += seq1[k] * decoded2[i][j][k];
				t2 += seq2[k] * decoded2[i][j][k];
				t3 += seq3[k] * decoded2[i][j][k];
				t4 += seq4[k] * decoded2[i][j][k];
			}
			t1 /= 8; t2 /= 8; t3 /= 8; t4 /= 8;
			int bit, power = pow(2, 7 - j);
			bit = (t1 == -1) ? 0 : 1;
			step2_2[i] |= bit * power;
			bit = (t2 == -1) ? 0 : 1;
			step2_2[i + 8] |= bit * power;
			bit = (t3 == -1) ? 0 : 1;
			step2_2[i + 16] |= bit * power;
			bit = (t4 == -1) ? 0 : 1;
			step2_2[i + 24] |= bit * power;
		}
	}
}


long long ksm(register long long x, register int y)                    //快速幂算法 
{
	int p = 65423;
	if (!y) return 1;
	register long long ret = ksm(x, y >> 1);
	if (y & 1) return ret*ret%p*x%p;
	return ret*ret%p;
}

void GaussianEliminationUnderMod(uint8_t* step2_2, uint8_t* step2_1)		//模意义下的高斯消元
{
	uint16_t sum[16];
	for (int i = 0; i < 16; i++) {
		sum[i] = step2_2[2 * i] + (step2_2[2 * i + 1] << 8);
		//printf("%d, ", sum[i]);
	}
	//printf("\n\n");

	int n = 16, p = 65423, maxi;
	long long tmp, ans[110], a[110][110];
	int key[16] = { 233, 136, 189, 132, 157, 100, 196, 185, 138, 222, 90, 101, 115, 229, 161, 97 };

	for (register int i = 1; i <= n; i++)
		for (register int j = 1; j <= n; j++)
			a[i][j] = ksm(key[i - 1], 15 - j + 1);
	for (int i = 1; i <= 16; i++) {
		a[i][17] = sum[i - 1];
	}

	for (register int i = 1; i <= n; i++)
	{
		if (!a[i][i])//主元不能为0
		{
			maxi = 0;
			for (register int j = i + 1; j <= n && !maxi; j++)
				if (a[j][i]) maxi = j;
			if (!maxi) continue;//如果一整列都为0，不需要消元
			for (register int j = i; j <= n + 1; j++)
				tmp = a[maxi][j], a[maxi][j] = a[i][j], a[i][j] = tmp;
		}
		for (register int j = i + 1; j <= n; j++)
		{
			tmp = a[j][i];
			if (!tmp) continue;//已经为0，不需要消元
			for (register int k = i; k <= n + 1; k++)
				a[j][k] = ((a[j][k] * a[i][i] - a[i][k] * tmp) % p + p) % p;
		}
	}
	for (register int i = n; i; i--)
	{
		for (register int j = i + 1; j <= n; j++)
			a[i][n + 1] = ((a[i][n + 1] - ans[j] * a[i][j]) % p + p) % p;
		ans[i] = a[i][n + 1] * ksm(a[i][i], p - 2) % p;
	}

	for (int i = 0; i < 16; i++) {
		step2_1[i] = (uint8_t)(ans[i + 1]);
	}
	//for(register int i=0;i<n;i++) printf("%d ",step2_1[i]);
}


unsigned char ROR1(unsigned char x, unsigned char n) {
	return (x << (8 - (n % 8))) | (x >> (n % 8));
}

bool threeBaseOperationCheck(uint8_t* step2_1, uint8_t* user_md5_part2_md5) {
	unsigned char O[] = { 26, 28, 17, 24, 8, 12, 25, 1, 32, 11, 7, 16, 23, 2, 29, 21, 20, 27, 22, 18, 5, 30, 10, 0, 9, 3, 19, 13, 4, 6, 14, 31 };
	unsigned char K[] = { 21, 23, 22, 20, 19, 12, 4, 18, 3, 6, 16, 14, 10, 24, 28, 15, 31, 0, 11, 5, 8, 26, 13, 32, 30, 29, 17, 9, 25, 2, 1, 27 };
	//unsigned char A[] = { 21, 9, 12, 31, 28, 19, 22, 25, 29, 3, 16, 15, 1, 2, 30, 6, 23, 0, 4, 26, 8, 14, 20, 5, 7, 24, 10, 17, 11, 27, 18, 13};
	unsigned char B[] = { 3, 238, 236, 17, 20, 14, 5, 12, 3, 237, 247, 5, 0, 246, 0, 231, 0, 232, 239, 11, 245, 3, 4, 255, 0, 22, 6, 244, 239, 24, 9, 249 };
	unsigned char C[] = { 23, 38, 241, 179, 134, 72, 110, 16, 154, 191, 181, 79, 233, 101, 15, 62 };
	/*for (int i = 0; i < 32; i++) {
	B[i] = A[i] - H[i];
	printf("%d, ", B[i]);
	}*/

	uint8_t Temp[16];
	for (int i = 0; i < 16; i++) {
		Temp[i] = ROR1((step2_1[i] ^ K[i]), O[i]) + B[i] - user_md5_part2_md5[i];
	}
	
	uint8_t FinalCheckMD5[16];
	GetMD5(Temp, FinalCheckMD5, 16);
	//for (int i = 0; i < 16; i++) {
	//	printf("%d, ", FinalCheckMD5[i]);
	//}

	for (int i = 0; i < 16; i++) {
		if (C[i] != FinalCheckMD5[i]) {
			return false;
		}
	}
	return true;
}



int main()
{
	uint8_t user[] = "iyzyi";
	uint8_t user_md5[16];
	GetMD5(user, user_md5, 5);


	// 第二部分
	uint8_t user_md5_part2_md5[16];
	GetMD5(user_md5 + 8, user_md5_part2_md5, 8);
	
	uint8_t step2_3[32 * 6] = { 109, 16, 82, 101, 52, 84, 72, 162, 27, 109, 52, 194, 109, 16, 82, 73, 130, 153, 8, 166, 153, 73, 167, 9, 44, 52, 82, 101, 52, 84, 73, 167, 9, 101, 52, 84, 72, 198, 139, 44, 52, 82, 109, 16, 82, 8, 166, 153, 8, 166, 153, 109, 16, 82, 80, 162, 137, 45, 20, 208, 72, 198, 139, 44, 52, 82, 65, 166, 155, 73, 130, 153, 45, 20, 208, 109, 16, 82, 65, 166, 155, 101, 52, 84, 137, 162, 139, 109, 52, 194, 8, 166, 153, 80, 162, 137, 45, 20, 208, 109, 16, 82, 72, 198, 139, 72, 162, 27, 65, 166, 155, 137, 162, 139, 137, 162, 139, 137, 162, 139, 101, 52, 84, 37, 56, 210, 37, 56, 210, 45, 20, 208, 8, 166, 153, 109, 16, 82, 102, 20, 210, 65, 166, 155, 72, 162, 27, 44, 52, 82, 45, 20, 208, 72, 162, 27, 72, 198, 139, 72, 162, 27, 137, 162, 139, 72, 198, 139, 137, 162, 139, 80, 162, 137, 109, 52, 194, 101, 52, 84, 109, 52, 194, 73, 130, 153, 72, 162, 27, 80, 162, 137 };

	uint8_t step2_2[32];
	CodeDivisionMultiplexingDecode(step2_3, step2_2);

	uint8_t step2_1[16];
	GaussianEliminationUnderMod(step2_2, step2_1);

	bool check2 = threeBaseOperationCheck(step2_1, user_md5_part2_md5);
	printf("%d", check2);

	system("pause");
    return 0;
}