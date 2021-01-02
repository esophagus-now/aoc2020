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
	
	int *all_ids = malloc(num_tix * sizeof(int));
	if (!all_ids) FAST_FAIL("out of memory");
	
	//NUL-terminate all the strings. Not really necessary, but easy enough
	//to do and might make it easier to use standard library routines later
	int i;
	for (i = 0; i < num_tix; i++) {
		data[stride*i + width] = 0;
		all_ids[i] = str_to_seat_id(data + stride*i);
	}
	
	qsort(all_ids, num_tix, sizeof(int), intcomp);
	
	int my_ticket = -1;
	for (i = 0; i < num_tix - 1; i++) {
		if (all_ids[i + 1] > all_ids[i] + 1) {
			my_ticket = all_ids[i] + 1;
			break;
		}
	}
	
	if (my_ticket < 0) {
		printf("Sadly, my ticket could not be found\n");
	} else {
		printf("My ticket number is %d\n", my_ticket);
	}
	
	free(all_ids);
	free(data);
	fclose(fp);
	
	return 0;
}
