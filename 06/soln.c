#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <vector.h>
#include <string.h>
#include <fast_fail.h>
#include <util.h>

#define SPACE " \t\v"
#define LNEND "\r\n"

typedef struct {
	VECTOR_DECL(char, ans_cat); //Concatenation of all the group's answers
	VECTOR_DECL(short, people); //Offsets into the answer array for each person
} group;

void group_init(group *g) {
	vector_init(g->ans_cat);
	vector_init(g->people);
}

void group_free(group *g) {
	if (!g) return;
	vector_free(g->people);
	vector_free(g->ans_cat);
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
	
	VECTOR_DECL(group, grps);
	vector_init(grps);
	
	char *str = data;
	group cur;
	group_init(&cur);
	short cur_person = 0;
	
	while(*str) {
		//Skip all whitespace
		str += strspn(str, SPACE);
		
		//If this is a blank line, then we push the current group to the
		//list of groups
		if ((*str == '\r' || *str == '\n') && cur_person != 0) {
			vector_extend_if_full(cur.ans_cat);
			cur.ans_cat[cur.ans_cat_len] = '\0'; //NUL-terminate answers without 
			                                     //counting NUL as part of length
			vector_push(grps, cur);
			group_init(&cur); //Subtle: don't free cur, because we "std::move"d it
			cur_person = 0;
			
			//Skip past all space and line terminators
			str += strspn(str, SPACE LNEND);
		}
		
		//Append this answer to the catted answers
		short anslen = 0;
		while ('a' <= *str && *str <= 'z') {
			vector_push(cur.ans_cat, *str++);
			anslen++;
		}
		
		//Push the current person offset to the list of people offsets
		vector_push(cur.people, cur_person);
		cur_person += anslen; //Move current offset past end of this answer
		
		//Skip all whitespace and at most one newline
		str += strspn(str, SPACE);
		if (*str == '\r') str++;
		if (*str == '\n') str++;
	}
	
	//Annoying corner case: if the end of the file does not have a blank line,
	//need to push the current group if it is valid
	if (cur_person != 0) {
		vector_push(grps, cur);
	}
	
	//For each group, get sum of cardinality of union set of all people
	int i;
	int count = 0;
	for (i = 0; i < grps_len; i++) {
		char *copy = strdup(grps[i].ans_cat);
		if (!copy) FAST_FAIL("out of memory");
		qsort(copy, grps[i].ans_cat_len, sizeof(char), charcomp); 
		
		int uniq_count = 0;
		char *tmp = copy;
		while(*tmp++) {
			uniq_count++;
			while(*tmp == *(tmp-1)) tmp++;
		}
		
		count += uniq_count;
		
		free(copy);
	}
	
	printf("The total count of the cardinality of the union of all people (for each group) is %d\n", count);
	
	for (i = 0; i < grps_len; i++) group_free(grps + i);
	vector_free(grps);
	free(data);
	fclose(fp);
	
	return 0;
}
