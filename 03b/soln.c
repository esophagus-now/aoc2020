#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <vector.h>
#include <string.h>
#include <fast_fail.h>
#include <util.h>

int num_encounters(char *map, int height, int period, int stride, int xstep, int ystep) {
	int x = 0, y = 0;
	
	int encounters = 0;
	
	while (y < height) {
		//Speed is of absolutely no concern
		#define IS_TREE(map, x, y) ((map)[(y)*stride + ((x)%period)] == '#')
		if (IS_TREE(map, x, y)) encounters++;
		x += xstep;
		y += ystep;
	}
	
	return encounters;
}

int main() {
	
	FILE *fp = fopen("input.txt", "rb");
	if (!fp) {
		perror("Could not open input.txt");
		FAST_FAIL("Cannot continue");
	}
	
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	rewind(fp);
	
	char *data = malloc(len);
	if (!data) FAST_FAIL("out of memory");
	fread(data, len, 1, fp);
	
	int period = strcspn(data, "\r\n");
	int stride = period  + strspn(data + period, "\r\n");
	
	int height = (len + stride - 1) / stride;
	
	int slopes[][2] = {
		{1,1},
		{3,1},
		{5,1},
		{7,1},
		{1,2}
	};
	
	uint64_t prod = 1;
	int i;
	for (i = 0; i < sizeof(slopes)/sizeof(*slopes); i++) {
		int xstep = slopes[i][0];
		int ystep = slopes[i][1];
		int encounters = num_encounters(data, height, period, stride, xstep, ystep);
		
		printf("For xstep = %d and ystep = %d, you will hit %d trees\n", xstep, ystep, encounters);
		prod *= (uint64_t) encounters;		
	}
	
	//I guess MinGW doesn't like PRIu64, so I need to do this:
	//printf("Product: " PRIu64 "\n", prod);
	printf("Product: %u%08u\n", (unsigned) (prod/100000000), (unsigned) (prod%100000000));
	
	fclose(fp);
	
	return 0;
}
