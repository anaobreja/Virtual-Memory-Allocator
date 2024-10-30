#include "vma.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	char *data;
	char *comanda;
	uint64_t start_address, address;
	size_t size;
	uint64_t arena_size;
	arena_t *arena;

	comanda = (char *)malloc(NMAX * sizeof(char));

	int ok = 1;

	while (ok != 0) {
		scanf("%s", comanda);
		if (strncmp(comanda, "ALLOC_ARENA", 11) == 0) {
			scanf("%lu", &arena_size);
			arena = alloc_arena(arena_size);
		} else if (strncmp(comanda, "ALLOC_BLOCK", 11) == 0) {
			scanf("%lu%lu", &start_address, &size);
			alloc_block(arena, start_address, size);
		} else if (strncmp(comanda, "PMAP", 4) == 0) {
			if (strlen(comanda) > 4)
				printf("%s\n", "Invalid command. Please try again.");
			else
				pmap(arena);
		} else if (strncmp(comanda, "DEALLOC_ARENA", 13) == 0) {
			dealloc_arena(arena);
			ok = 0;
		} else if (strncmp(comanda, "FREE_BLOCK", 10) == 0) {
			if (strlen(comanda) > 10)
				printf("%s\n", "Invalid command. Please try again.");
			else
				scanf("%lu", &address);
			free_block(arena, address);
		} else if (strncmp(comanda, "WRITE", 5) == 0) {
			if (strlen(comanda) > 5) {
				printf("%s\n", "Invalid command. Please try again.");
			} else {
				scanf("%lu%lu", &address, &size);
				data = malloc(sizeof(char) * 2 * (size + 1));
				data[0] = getchar();
				for (int i = 0; i < (int)size; i++)
					data[i] = getchar();
				write(arena, address, size, data);
				free(data);
			}
		} else if (strncmp(comanda, "READ_BLOCK", 10) == 0) {
			scanf("%lu%lu", &address, &size);
		} else if (strncmp(comanda, "READ", 4) == 0) {
			if (strlen(comanda) > 4) {
				printf("%s\n", "Invalid command. Please try again.");
			} else {
				scanf("%lu%lu", &address, &size);
				read(arena, address, size);
			}
		} else if (strncmp(comanda, "MPROTECT", 8) == 0) {
			char *perm = (char *)malloc(NMAX * sizeof(char));
			scanf("%lu", &address);
			mprotect(arena, address, perm);
		} else {
			printf("%s\n", "Invalid command. Please try again.");
		}
	}

	free(comanda);

	return 0;
}
