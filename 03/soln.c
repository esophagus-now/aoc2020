#include <stdio.h>
#include <stdlib.h>
#include <vector.h>
#include <string.h>
#include <fast_fail.h>
#include <util.h>

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
	
	int x = 0, y = 0;
	
	int encounters = 0;
	
	while (height --> 0) {
		//Speed is of absolutely no concern
		#define IS_TREE(map, x, y) ((map)[(y)*stride + ((x)%period)] == '#')
		if (IS_TREE(data, x, y)) encounters++;
		x += 3;
		y += 1;
	}
	
	printf("You will hit %d trees\n", encounters);
	
	fclose(fp);
}
