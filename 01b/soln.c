#include <stdio.h>
#include <stdlib.h>
#include <vector.h>
#include <fast_fail.h>
#include <util.h>

//Returns product on success, -1 if no solution
//Expects input to be sorted
int findsum(int desired, int const *nums, int nums_len) {
	int l = 0, r = nums_len - 1;
	
	while (l < r) {
		int sum = nums[l] + nums[r];
		if (sum < desired) {
			l++;
		} else if (sum == desired) {
			printf("First number: %d\n", nums[l]);
			printf("Second number: %d\n", nums[r]);
			return nums[l]*nums[r];
		} else {
			r--;
		}
	}
	
	return -1;
}

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
	
	int i;
	int prod = -1;
	for (i = 0; i < nums_len - 2; i++) { //1-indexing a little more natural?
		int first = nums[i];
		prod = findsum(2020 - nums[i], nums + i + 1, nums_len - (i + 1));
		if (prod < 0) {
			//No solution with nums[i] as the first number
			continue;
		} else {
			printf("Third number: %d\n", first);
			prod *= first;
			break;
		}
	}
	
	if (prod < 0) {
		puts("Sadly, no solution was found");
	} else {
		printf("Product: %d\n", prod);
	}
	
	vector_free(nums);
	fclose(fp);
}
