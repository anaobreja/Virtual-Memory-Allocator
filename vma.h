#ifndef FUNCTII
#define FUNCTII

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NMAX 100

typedef struct node_t node_t;
typedef struct list_t list_t;
typedef struct block_t block_t;

struct node_t {
	node_t *prev, *next;
	void *type; //nodul "contine" block sau miniblock
};

struct list_t {
	node_t *head;
	unsigned int data_size, size;
};

struct block_t {
	uint64_t start_address;
	size_t size;
	void *miniblock_list;
};

typedef struct {
	uint64_t start_address;
	size_t size;
	uint8_t perm;
	void *rw_buffer;
} miniblock_t;

typedef struct {
	uint64_t arena_size;
	list_t *alloc_list;
} arena_t;

arena_t *alloc_arena(const uint64_t size);
void dealloc_arena(arena_t *arena);

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);
void free_block(arena_t *arena, const uint64_t address);

void read(arena_t *arena, uint64_t address, uint64_t size);
void
write(arena_t *arena, const uint64_t address,  const uint64_t size, char *data);
void pmap(const arena_t *arena);
void mprotect(arena_t *arena, uint64_t address, char *permission);

list_t *create_list(uint64_t data_size);
void add_nth_node(list_t *list, unsigned int n);
node_t *get_nth_node(list_t *list, unsigned int n);
node_t *remove_nth_node(list_t *list, unsigned int n);

#endif
