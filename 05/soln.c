#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <vector.h>
#include <string.h>
#include <fast_fail.h>
#include <util.h>

//Assumes input is valid
int str_to_seat_id(char const *str) {
	int res = 0;
	
	while (*str) {
		res <<= 1;
		if (*str == 'B' || *str == 'R') res |= 1;
		str++;
	}
	
	return res;
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
	
	char *data = malloc(len+1);
	if (!data) FAST_FAIL("out of memory");
	fread(data, len, 1, fp);
	data[len] = 0; //NUL-temrinate for safety
	
	//Assume all seat codes have same length
	#define WS " \t\v\r\n"
	int width = strcspn(data, WS);
	int stride = width + strspn(data + width, WS);
	int num_tix = (len + stride - 1) / stride;
	
	//NUL-terminate all the strings. Not really necessary, but easy enough
	//to do and might make it easier to use standard library routines later
	int i;
	int max_id = -1;
	for (i = 0; i < num_tix; i++) {
		data[stride*i + width] = 0;
		int id = str_to_seat_id(data + stride*i);
		if (id > max_id) max_id = id;
	}
	
	printf("The largest seat ID is %d\n", max_id);
	
	free(data);
	fclose(fp);
	
	return 0;
}
