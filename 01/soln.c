#include <stdio.h>
#include <stdlib.h>
#include <vector.h>
#include <fast_fail.h>
#include <util.h>

int main() {
	VECTOR_DECL(int, nums);
	vector_init(nums);
	
	FILE *fp = fopen("input.txt", "rb");
	if (!fp) {
		perror("Could not open input.txt");
		FAST_FAIL("Cannot continue");
	}
	
	while (!feof(fp)) {
		int *x = vector_lengthen(nums);
		int rc = fscanf(fp, "%d", x);
		if (rc < 1) {
			if (feof(fp)) {
				vector_pop(nums);
			} else if (ferror(fp)) {
				perror("Error reading input.txt");
				FAST_FAIL("Cannot continue");
			} else {
				FAST_FAIL("Bizarre error");
			}
		}
	}
	
	qsort(nums, nums_len, sizeof(*nums), intcomp);
	
	int l = 0, r = nums_len - 1;
	
	while (l < r) {
		int sum = nums[l] + nums[r];
		if (sum < 2020) {
			l++;
		} else if (sum == 2020) {
			printf("Product: %d\n", nums[l]*nums[r]);
			break;
		} else {
			r--;
		}
	}
	
	if (l >= r) {
		puts("Sadly, there were no two numbers that summed to 2020");
	}
	
	vector_free(nums);
	fclose(fp);
}
