#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <vector.h>
#include <map.h>
#include <string.h>
#include <assert.h>

#define SPACE " \t\v"
#define LNEND "\r\n"
#define ALPHA "abcdefghijklmnopqrstuvwxyz"
#define DIGIT "0123456789"

//Kind of like a relational database mapping names to IDs
map *col_to_id = NULL;
VECTOR_DECL(char const*, id_to_col);

//Whatever, this is simpler
#define MAX_ITEM_TYPES 16
typedef struct bag_rule {
	int container_id;
	int num;
	int allowed[MAX_ITEM_TYPES];
	int allowed_amt[MAX_ITEM_TYPES];
} bag_rule;

void bag_rule_init(bag_rule *r) {
	r->num = 0;
}

int get_col_id(char *str, int len) {
	char saved = str[len];
	str[len] = '\0'; //NUL-terminate so we can do a lookup
	int *id = map_search(col_to_id, str);
	
	if (!id) {
		char *cpy = strdup(str);
		int new_id = id_to_col_len;
		//printf("Registering new colour: [%s] has ID [%d]\n", str, new_id);
		vector_push(id_to_col, cpy);
		map_insert(col_to_id, cpy, 1, &new_id, 0);
		
		str[len] = saved;
		return new_id;
	}
	
	str[len] = saved;
	return *id;
}

//Returns number of characters read
int scan_colour(char *str, int *col_id) {
	//Assume all bag colours have format ADJECTIVE COLOUR
	char *col_begin = str;
	str += strspn(str, ALPHA);
	str += strspn(str, SPACE);
	str += strspn(str, ALPHA);
	int col_len = str - col_begin;
	
	*col_id = get_col_id(col_begin, col_len);
	return col_len;
}

//Returns number of characters to skip alpha then space
int skip_word(char *str) {
	char *str_saved = str;
	str += strspn(str, SPACE);
	str += strspn(str, ALPHA);
	str += strspn(str, SPACE);
	
	/*printf("Skipping word [");
	char *tmp = str_saved;
	for (; tmp != str; tmp++) printf("%c", *tmp);
	puts("]");	*/
	
	return str - str_saved;
}

void print_rule(bag_rule const *r) {
	printf("%s bags contain", id_to_col[r->container_id]);
	char const *delim = " ";
	int i;
	for (i = 0; i < r->num; i++) {
		int amt = r->allowed_amt[i];
		char const *str = id_to_col[r->allowed[i]];
		printf("%s%d %s bag%s", delim, amt, str, (amt == 1) ? "" : "s");
		delim = ", ";
	}
	puts(".");
}

void print_rules(VECTOR_PTR_PARAM(bag_rule, rules)) {
	int i;
	for (i = 0; i < *rules_len; i++) print_rule(&(*rules)[i]);
}

void print_adj_mat(char const *mat, int n) {
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			printf("%c", mat[i*n + j] ? '1' : '0');
		}
		puts("");
	}
}

//Only works with square matrices.
void adj_mat_mul(char *dest, char const *srcA, char const *srcB, int n) {
	//Dest and srcs may not alias, but srcs may alias
	assert(((dest + n*n) <= srcA) || ((srcA + n*n) <= dest));
	assert(((dest + n*n) <= srcB) || ((srcB + n*n) <= dest));
	
	int i, j, k;
	for (i = 0; i < n; i++) {
		char *dest_row = dest + i*n;
		for (j = 0; j < n; j++) {
			char res = 0;
			for (k = 0; k < n; k++) {
				if (srcA[i*n + k] && srcB[k*n + j]) {
					res = 1;
					break;
				}
			}
			dest_row[j] = res;
		}
	}
}

// Technique: transitive closure of an adjacency A of size n by n is equal to:
//   A^n + A^(n-1) + ... + A
// where multiplication (between elements of A) is AND and and addition (between
// elements of A) is OR. In the above expression, A^n means repeated matrix products.
//
// It turns out this is equal to:
//  (A+I)^(n-1) * A
//
// This works because OR is idempotent. If you do the binomial expansion of 
// (A+I)^(n-1), the (scalar) multiplication in front of terms is actually 
// repeated OR'ing.
//
// One more thing. A graph-theoretical argument (that I won't prove here, mostly
// because I don't know how to prove it) shows that actually we can do
//
//  (A+I)^M * A, where M >= (n-1)
//
// Intuititively, this is because this product still find all the paths of length
// up to N, which is all that is needed to compute transitive closure. The reason
// to raise the matrix to a higher power is because we can just use repeated squaring
// and pick M to be the largest power of 2 that is greater than or equal to (n-1).
//
void transitive_closure(char *dest, char const *src, int n) {
	//Matrices may not alias
	//TODO: it should be possible to get around this without sacrificing 
	//performance, because I had to copy src anyway to add the identity
	assert(((dest + n*n) <= src) || ((src + n*n) <= dest));
	
	//Corner cases
	assert(n > 0);
	if (n == 1) {
		*dest = *src;
		return;
	}
	
	//puts("Source: ");
	//print_adj_mat(src, n);
	
	char *tmp = malloc(n*n);
	if (!tmp) FAST_FAIL("out of memory");
	
	char *tmp2 = malloc(n*n);
	if (!tmp2) FAST_FAIL("out of memory");
	
	//Save (A + I) in tmp
	memcpy(tmp, src, n*n);
	int i;
	for (i = 0; i < n; i++) tmp[i*n + i] = 1;
	
	//puts("Source plus identity: ");
	//print_adj_mat(tmp, n);
	
	//Sanity check. We want this loop to run for clog2(n-1) times. Note that
	//n is assumed to be 2 or greater at this point.
	//Suppose n = 2: the loop doesn't run (correct)
	//If n = 3, the loop runs once (correct)
	//If n = 4, the loop runs twice (correct)
	//If n = 5, the loop runs twice (correct)
	//If n = 6, the loop runs three times (correct)
	//Okay, I'm satisfied
	int logerand = n - 2;
	while (logerand > 0) {
		adj_mat_mul(tmp2, tmp, tmp, n);
		char *swap_tmp = tmp;
		tmp = tmp2;
		tmp2 = swap_tmp;
		logerand >>= 1;
		//puts("After squaring: ");
		//print_adj_mat(tmp, n);
	}
	
	adj_mat_mul(dest, src, tmp, n);
	//puts("Result");
	//print_adj_mat(dest, n);
	
	free(tmp2);
	free(tmp);
}

int main() {
	int ret = 0;
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
	data[len] = 0; //NUL-terminate for safety
	
	
	col_to_id = malloc(sizeof(map));
	map_init(col_to_id, char const*, int, STR2VAL);
	
	vector_init(id_to_col);
	
	VECTOR_DECL(bag_rule, rules);
	vector_init(rules);
	
	//Read all the rules
	char *str = data;
	while(1) {
		//Skip all whitespace
		str += strspn(str, "." SPACE LNEND);
		if (!*str) break;
		
		//Read a rule. 
		bag_rule *r = vector_lengthen(rules);
		bag_rule_init(r);
		
		str += scan_colour(str, &r->container_id);
		
		//Skip "bag[s] contain"
		str += skip_word(str);
		str += skip_word(str);
		
		//Read list of bag colours and amounts
		do {
			//Skip comma (if there is one)
			str += strspn(str, "," SPACE);
			
			//This input is by far the most complicated so far from AoC.
			//Sometimes bags contain "no other bags".
			if (*str == 'n') {
				str += strcspn(str, ".");
				break;
			}
			
			int idx = r->num++;
			//For some reason "%n" doesn't work in MinGW
			int rc = sscanf(str, "%d", r->allowed_amt + idx);
			if (rc < 1) {
				fprintf(stderr, "Could not parse number\n");
				ret = -1;
				goto cleanup;
			}
			str += strspn(str, DIGIT); //Too bad %n doesn't work...
			
			//Skip whitespace
			str += strspn(str, SPACE);
		
			//Read colour
			str += scan_colour(str, r->allowed + idx);
			
			//Skip word "bag[s]"
			str += skip_word(str);
			
			//Assume no space before the comma (if there is one)
		} while (*str == ',');
	}
	
	//Okay: our job is to answer the question "how many types of bag may 
	//contain a shiny gold bag?" 
	//By the way, bags can recursively contain any number of other bags
	
	//The solution is to use transitive closure (there may be a simpler way,
	//but whatever)
	
	//Generate the adjacency matrix where M[i][j] means bag_id i can contain
	//bags of id j
	int n = id_to_col_len; //n is the width of the adjacency matrix
	char *adj = calloc(n, n);
	
	int i;
	for (i = 0; i < rules_len; i++) {
		char *row = adj + rules[i].container_id * n;
		int j;
		for (j = 0; j < rules[i].num; j++) {
			row[rules[i].allowed[j]] = 1;
		}
	}
	
	//puts("Original");
	//print_adj_mat(adj, n);
	
	char *closure = malloc(n*n);
	transitive_closure(closure, adj, n);
	free(adj);
	
	//puts("\nClosure");
	//print_adj_mat(closure, n);
	
	//Now the hard work is done. The only thing left is to read the column
	//under "shiny gold" and count the number of ones.
	int *id = map_search(col_to_id, "shiny gold");
	if (!id) {
		puts("Zero bags may contain shiny gold bags");
	} else {
		int i;
		int count = 0;
		for (i = 0; i < n; i++) {
			count += closure[i*n + *id];
		}
		printf("%d bags may contain shiny gold bags\n", count);
	}
	
	free(closure);
	
cleanup:
	free(data);
	fclose(fp);
	
	vector_free(rules);
	vector_free(id_to_col);
	map_free(col_to_id);
	free(col_to_id);
	
	return ret;
}
