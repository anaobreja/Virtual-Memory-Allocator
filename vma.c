#include "vma.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NMAX 100

list_t *create_list(uint64_t data_size)
{
	list_t *list = malloc(sizeof(struct list_t));
	if (!list) {
		fprintf(stderr, "Malloc failed\n");
		exit(-1);
	}

	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;
	return list;
}

void
add_nth_node(list_t *list, unsigned int n)
{
	if (!list)
		return;

	if (n >= list->size)
		n = list->size;

	node_t *ptr = malloc(sizeof(struct node_t));

	if (!ptr) {
		fprintf(stderr, "Malloc failed\n");
		exit(-1);
	}

	ptr->prev = NULL;
	ptr->next = NULL;

	if (n == 0) {
		if (list->size >= 1) {
			ptr->next = list->head;
			list->head->prev = ptr;
			list->head = ptr;
			list->size++;
			return;
		}

		list->head = ptr;
		list->size++;
		return;
	}

	node_t *temp1, *temp2;

	if (n == list->size) {
		temp1 = list->head;
		while (temp1->next)
			temp1 = temp1->next;

		temp1->next = ptr;
		ptr->prev = temp1;
		list->size++;
		return;
	}

	temp1 = list->head;
	temp2 = temp1->next;

	while (n > 1) {
		temp1 = temp1->next;
		temp2 = temp2->next;
		n--;
	}

	ptr->next = temp2;
	ptr->prev = temp1;
	temp1->next = ptr;
	temp2->prev = ptr;

	list->size++;
}

node_t*
get_nth_node(list_t *list, unsigned int n)
{
	if (!list || !list->head)
		exit(-1);

	if (n >= list->size)
		n %= list->size;

	node_t *ptr;

	ptr = list->head;

	while (n >= 1) {
		ptr = ptr->next;
		n--;
	}

	return ptr;
}

node_t*
remove_nth_node(list_t *list, unsigned int n)
{
	if (!list || !list->head)
		exit(-1);

	if (n >= list->size - 1)
		n = list->size - 1;

	node_t *ptr = list->head;

	if (n == 0) {
		if (list->size > 1) {
			ptr = list->head;
			list->head = list->head->next;
			list->head->prev = NULL;
			list->size--;

			return ptr;
		}
		list->head = NULL;
		list->size--;
		return ptr;
	}

	if (n == list->size - 1) {
		while (ptr->next)
			ptr = ptr->next;

		ptr->prev->next = NULL;
		list->size--;
		return ptr;

	} else {
		while (n >= 1) {
			ptr = ptr->next;
			n--;
		}

	ptr->next->prev = ptr->prev;
	ptr->prev->next = ptr->next;
	list->size--;
	return ptr;
	}
}

arena_t*
alloc_arena(const uint64_t size)
{
	arena_t *arena = malloc(sizeof(arena_t));
	arena->arena_size = size;
	arena->alloc_list = create_list(sizeof(block_t));
	return arena;
}

void
dealloc_arena(arena_t *arena)
{
	node_t *node_block = arena->alloc_list->head;

	while (node_block) {
		list_t *mini_list = ((block_t *)node_block->type)->miniblock_list;
		node_t *node_miniblock = mini_list->head;

		while (node_miniblock) {
			node_t *aux = node_miniblock->next;
			char *buffer = ((miniblock_t *)mini_list->head->type)->rw_buffer;
			if (buffer)
				free(((miniblock_t *)mini_list->head->type)->rw_buffer);

			free(mini_list->head->type);

			node_miniblock = remove_nth_node(mini_list, 0);
			free(node_miniblock);

			node_miniblock = aux;
		}

		node_t *aux = node_block->next;

		free(mini_list);

		free(arena->alloc_list->head->type);
		node_block = remove_nth_node(arena->alloc_list, 0);
		free(node_block);
		node_block = aux;
	}

	free(arena->alloc_list);
	free(arena);
}

miniblock_t*
create_miniblock(const uint64_t address, const uint64_t size)
{
	miniblock_t *miniblock = malloc(sizeof(miniblock_t));
	miniblock->start_address = address;
	miniblock->size = size;
	miniblock->rw_buffer = NULL;
	miniblock->perm = 6;
	return miniblock;
}

/*functie care adauga un nod in lista de block-uri, creeaza o lista si adauga
un singur miniblock*/
void
add_block(arena_t *arena, const uint64_t address, const uint64_t size, int n)
{
	int size_block = sizeof(block_t);
	int size_miniblock = sizeof(miniblock_t);

	list_t *mini_list = create_list(size_miniblock);
	add_nth_node(mini_list, 0);

	mini_list->head->type = create_miniblock(address, size);

	block_t *block = malloc(size_block);
	block->start_address = address;
	block->size = size;
	block->miniblock_list = mini_list;

	add_nth_node(arena->alloc_list, n);

	node_t *ptr = get_nth_node(arena->alloc_list, n);

	ptr->type = block;
}

//functie care adauga un miniblock in lista la o pozitie data
void
add_miniblock(const uint64_t address, const uint64_t size, node_t *temp, int n)
{
	miniblock_t *miniblock = create_miniblock(address, size);
	list_t *mini_list = ((block_t *)temp->type)->miniblock_list;

	add_nth_node(mini_list, n);
	node_t *ptr = get_nth_node(mini_list, n);

	ptr->type = miniblock;

	((block_t *)temp->type)->size += size;
	if (n == 0)
		((block_t *)temp->type)->start_address = address;
}

/*functie care indeplineste cazul in care se doreste adaugarea
unui block care se afla exact intre alte 2 block-uri*/
void aux_alloc(arena_t *arena, node_t *temp1, node_t *temp2,
			   uint64_t address, uint64_t size, int i)
{
	list_t *mini_list = ((block_t *)temp1->type)->miniblock_list;

		add_miniblock(address, size, temp1, mini_list->size);

		node_t *ptr = get_nth_node(mini_list, mini_list->size - 1);
		node_t *ptr1 =
		get_nth_node(((block_t *)temp2->type)->miniblock_list, 0);

		ptr->next = ptr1;
		ptr->next->prev = ptr;
		mini_list->size +=
		((list_t *)((block_t *)temp2->type)->miniblock_list)->size;

		((block_t *)temp1->type)->size += ((block_t *)temp2->type)->size;

		((list_t *)((block_t *)temp2->type)->miniblock_list)->head = NULL;

		temp2 = remove_nth_node(arena->alloc_list, i);
		free(((block_t *)temp2->type)->miniblock_list);
		free(temp2->type);
		free(temp2);
}

/*functie care trateaza toate cazurile in care un block este adaugat
si erorile posibile;
am luat in calcul initial cazul in care se doreste adaugarea in arena
libera sau "la stanga" block-ului cand in lista se afla un singur alt
block (si adresa de sfarsit coincide sau nu cu adresa de inceput a
block-ului);
in continuare, am considerat situatia in care block-ul trebuie adaugat
intre minimum 2 block-uri deja existente sau la sfarsitul listei
*/
void
alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return;
	} else if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n");
		return;
	}
	if (arena->alloc_list->size == 0) {
		add_block(arena, address, size, 0);
		return;
	}
	uint64_t condition1, condition2;
	node_t *temp1 = arena->alloc_list->head, *temp2;
	condition2 = ((block_t *)temp1->type)->start_address;
	if (address + size < condition2) {
		add_block(arena, address, size, 0);
		return;
	} else if (address + size == condition2) {
		add_miniblock(address, size, temp1, 0);
		return;
	}
	for (unsigned int i = 1; i <= arena->alloc_list->size; i++) {
		uint64_t b_address = ((block_t *)temp1->type)->start_address;
		uint64_t b_size = ((block_t *)temp1->type)->size;
		if (address == b_address) {
			printf("This zone was already allocated.\n");
			return;
		}
		if (address < b_address && address + size >= b_address + b_size) {
			printf("This zone was already allocated.\n");
			return;
		}
		if (address < b_address && address <= b_address + b_size) {
			printf("This zone was already allocated.\n");
			return;
		}
		if (address > b_address && address < b_address + b_size) {
			printf("This zone was already allocated.\n");
			return;
		}
		if (i == arena->alloc_list->size) {
			temp1 =
			get_nth_node(arena->alloc_list, arena->alloc_list->size - 1);
			condition1 = ((block_t *)temp1->type)->start_address +
			((block_t *)temp1->type)->size;
			condition2 = arena->arena_size;
		} else {
			temp1 = get_nth_node(arena->alloc_list, i - 1);
			temp2 = get_nth_node(arena->alloc_list, i);

			condition1 = ((block_t *)temp1->type)->start_address +
			((block_t *)temp1->type)->size;
			condition2 = ((block_t *)temp2->type)->start_address;
		}
		if (condition1 < address && address + size < condition2) {
			add_block(arena, address, size, i);
			return;
		} else if (condition1 < address && address + size == condition2) {
			if (i == arena->alloc_list->size)
				add_block(arena, address, size, arena->alloc_list->size);
			else
				add_miniblock(address, size, temp2, 0);
			return;
		} else if (condition1 == address && address + size < condition2) {
			add_miniblock(address, size, temp1,
						  ((list_t *)((block_t *)temp1->type)
						  ->miniblock_list)->size);
			return;
		} else if (condition1 == address && address + size == condition2) {
			if (i != arena->alloc_list->size) {
				aux_alloc(arena, temp1, temp2, address, size, i);
			return;
			} else {
				add_miniblock(address, size, temp1,
							  ((list_t *)((block_t *)temp1->type)
							  ->miniblock_list)->size);
			}
		}
	}
}

/*functie care sterge un miniblock din lista si eventual
un block daca lista de miniblock-uri contine un singur nod */
void free_block(arena_t *arena, const uint64_t address)
{
	uint64_t condition1, condition2;
	node_t *temp = arena->alloc_list->head;
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		condition1 = ((block_t *)temp->type)->start_address;
		condition2 = ((block_t *)temp->type)->start_address +
		((block_t *)temp->type)->size;
		if (condition1 <= address && address < condition2) {
			list_t *mini_list = ((block_t *)temp->type)->miniblock_list;
			node_t *temp1 = mini_list->head;
			node_t *temp2;
			for (unsigned int j = 0; j < mini_list->size; j++) {
				uint64_t b_addr = ((miniblock_t *)temp1->type)->start_address;
				if (address == b_addr) {
					if (mini_list->size == 1) {
						free(temp1->type);
						temp1 = remove_nth_node(mini_list, 0);
						free(temp1); free(mini_list);
						free(temp->type);
						temp = remove_nth_node(arena->alloc_list, i);
						free(temp);
						return;
					}
					if (j == 0) {
						temp2 = temp1->next;
						((block_t *)temp->type)->start_address =
						((miniblock_t *)temp2->type)->start_address;
						((block_t *)temp->type)->size -=
						((miniblock_t *)temp1->type)->size;
						free(temp1->type);
						temp1 = remove_nth_node(mini_list, 0);
						free(temp1);
						return;
					} else if (j == mini_list->size - 1) {
						((block_t *)temp->type)->size -=
						((miniblock_t *)temp1->type)->size;
						free(temp1->type);
						temp1 =
						remove_nth_node(((block_t *)temp->type)->miniblock_list,
										((block_t *)temp->type)->size - 1);
						free(temp1);
						return;
					}
					add_nth_node(arena->alloc_list, i + 1);
					node_t *aux_block =
					get_nth_node(arena->alloc_list, i + 1);
					aux_block->type = malloc(sizeof(block_t));
					list_t *mini_list_aux = malloc(sizeof(list_t));
					node_t *temp2 = temp1->next;
					int size = 0, count = 0;
					while (temp2) {
						size += ((miniblock_t *)temp2->type)->size;
						count++;
						temp2 = temp2->next;
					}
					mini_list_aux->head = temp1->next;
					temp1->prev->next = NULL;
					mini_list_aux->head->prev = NULL;
					((block_t *)aux_block->prev->type)->size -=
					(size + ((miniblock_t *)temp1->type)->size);
					mini_list_aux->size = count;
					count++;
					((list_t *)((block_t *)temp->type)->miniblock_list)->
					size -= count;
					((block_t *)aux_block->type)->size = size;
					((block_t *)aux_block->type)->start_address =
					((miniblock_t *)mini_list_aux->head->type)->
					start_address;
					((block_t *)aux_block->type)->miniblock_list =
					mini_list_aux;
					free(temp1->type);
					free(temp1);
					return;
				}
				temp1 = temp1->next;
			}
		}
		temp = temp->next;
	}
	printf("Invalid address for free.\n");
}

/*functie in care se doreste verificare pentru
citirea dintr-o zona in care a fost scrisa informatie continuu,
insa cazul nu a fost necesar in tema*/

// int read_aux(arena_t *arena, uint64_t address, uint64_t size, int i)
// {
// unsigned int count = 0;

// node_t* temp = get_nth_node(arena->alloc_list, i);
// list_t* mini_list = ((block_t *)temp->type)->miniblock_list;
// node_t* temp1 = mini_list->head;

// for (unsigned int i = 0; i < mini_list->size; i++) {
// for (unsigned int j = address -
//		((miniblock_t *)temp1->type)->start_address;
//		j < ((miniblock_t *)temp1->type)->size; j++) {

// if (((char *)((miniblock_t *)temp1->type)->rw_buffer)[j]) {
// count++;
// } else {

// if (count >= size) {
// return count;
// } else {
// return -1;
// }
// }

// }
// if (temp1->next) {
// temp1 = temp1->next;
// if (temp1)
// address = ((miniblock_t *)temp1->type)->start_address;
// }
// }

// if (count >= size) {
// return count;
// } else {
// return -1;
// }
// }

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	uint64_t condition1, condition2;
	int ok = 0;

	node_t *temp = arena->alloc_list->head;

	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		condition1 = ((block_t *)temp->type)->start_address;
		condition2 = condition1 + ((block_t *)temp->type)->size;

		if (condition1 <= address && address < condition2) {
			uint64_t new_size = size;
			uint64_t new_address = address;
			//citirea pana la finalul block-ului daca lungimea data
			//il depaseste
			if (address + size > condition2) {
				new_size = condition2 - address;
				printf("Warning: size was bigger than the block size. ");
				printf("Reading %ld characters.\n", new_size);
			}

//if (read_aux(arena, address, new_size, i) < 0) {
//printf("error\n");
//return;
//}

			list_t *mini_list = ((block_t *)temp->type)->miniblock_list;
			node_t *temp1 = mini_list->head;

			for (unsigned int k = 0; k < mini_list->size; k++) {
				uint8_t perm;
				perm = ((miniblock_t *)temp1->type)->perm;
				//comditie mprotect
				if ((perm >> 2) == 0) {
					printf("Invalid permissions for read.\n");
					return;
				}
				temp1 = temp1->next;
			}

			temp1 = mini_list->head;

			for (unsigned int k = 0; k < mini_list->size; k++) {
				condition1 = ((miniblock_t *)temp1->type)->start_address;
				condition2 = condition1 + ((miniblock_t *)temp1->type)->size;
				//citire din miniblock si din urmatoarele, daca lungimea data
				//il depaseste
				if (condition1 <= new_address && new_address <= condition2) {
					ok = 1;
				for (unsigned int j = new_address -
					((miniblock_t *)temp1->type)->start_address;
				j <  ((miniblock_t *)temp1->type)->size; j++) {
					printf("%c", ((char *)((miniblock_t *)temp1->type)->
					rw_buffer)[j]);
					new_size--;
					if (new_size == 0) {
						printf("\n");
						return;
					}
				}
			}   //redimiensionare date
				if (new_size) {
					if (temp1->next && ok == 1) {
						new_address =
						((miniblock_t *)temp1->next->type)->start_address;
					}
				}
				temp1 = temp1->next;
			}
		temp = temp->next;
		}
	}
	printf("Invalid address for read.\n");
}

void write(arena_t *arena, const uint64_t address,
		   const uint64_t size, char *data)
{
	uint64_t condition1, condition2;
	node_t *temp = arena->alloc_list->head;
	int ok = 0;
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		condition1 = ((block_t *)temp->type)->start_address;
		condition2 = condition1 + ((block_t *)temp->type)->size;
		if (condition1 <= address && address < condition2) {
			uint64_t new_size = size, new_address = address;
			int add = 0;
			if (address + size > condition2) {
				new_size = condition2 - address;
				printf("Warning: size was bigger than the block size. ");
				printf("Writing %ld characters.\n", new_size);
			}
			if (condition1 <= address && address < condition2)
				ok = 1;
			list_t *mini_list = ((block_t *)temp->type)->miniblock_list;
			node_t *temp1 = mini_list->head;
			for (unsigned int k = 0; k < mini_list->size; k++) {
				uint8_t perm;
			perm = ((miniblock_t *)temp1->type)->perm;
			if (perm == 1 || perm == 4 || perm == 5 || perm == 0) {
				printf("Invalid permissions for write.\n");
				return;
			}
			temp1 = temp1->next;
			}
			temp1 = mini_list->head;
			for (unsigned int j = 0; j < mini_list->size; j++) {
				int count = 0, ok = 0;
				char *buffer = ((miniblock_t *)temp1->type)->rw_buffer;
				if (!buffer)
					((miniblock_t *)temp1->type)->rw_buffer =
					calloc(sizeof(char), ((miniblock_t *)temp1->type)->size);
				if (address >
				((miniblock_t *)temp1->type)->start_address && ok) {
					ok = 1;
					int aux = address -
					((miniblock_t *)temp1->type)->start_address;
					for (int i = aux; i <
					(int)((miniblock_t *)temp1->type)->size; i++) {
						((char *)((miniblock_t *)temp1->type)->rw_buffer)[i] =
						((char *)data)[i - aux]; count++;
					}
					if (new_size + new_address ==
					((miniblock_t *)temp1->type)->size +
					((miniblock_t *)temp1->type)->start_address)
						return;
				} else {
					uint64_t final;
					if (new_size > ((miniblock_t *)temp1->type)->size)
						final = ((miniblock_t *)temp1->type)->size;
					else
						final = new_size;
					for (int i = 0; i < (int)final; i++) {
						((char *)((miniblock_t *)temp1->type)->rw_buffer)[i] =
						((char *)data)[i + add];
					}
					if (new_size <= ((miniblock_t *)temp1->type)->size)
						return;
					count = ((miniblock_t *)temp1->type)->size;
				}
			if (((miniblock_t *)temp1->type)->start_address +
				((miniblock_t *)temp1->type)->size
				 < new_size + new_address) {
				add += count;
				new_size -= count;
				new_address = ((miniblock_t *)temp1->next->type)->
				start_address;
				}
			temp1 = temp1->next;
		}
	}
	temp = temp->next;
	}
	if (!ok)
		printf("Invalid address for write.\n");
}

void pmap(const arena_t *arena)
{
	node_t *node_block = arena->alloc_list->head;

	int alocated_memory = 0, blocks, miniblocks = 0;

	blocks = arena->alloc_list->size;

	node_block = arena->alloc_list->head;

	printf("Total memory: 0x%lX bytes\n", arena->arena_size);

	while (node_block) {
		alocated_memory += ((block_t *)node_block->type)->size;

		list_t *mini_list =
		((block_t *)node_block->type)->miniblock_list;

		miniblocks += mini_list->size;

		node_block = node_block->next;
	}
	//afisare date generale
	printf("Free memory: 0x%lX bytes\n", arena->arena_size - alocated_memory);

	printf("Number of allocated blocks: %d\n", blocks);

	printf("Number of allocated miniblocks: %d\n", miniblocks);

	node_block = arena->alloc_list->head;

	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		printf("\nBlock %d begin\n", i + 1);
		printf("Zone: 0x%lX - 0x%lX\n",
			   ((block_t *)node_block->type)->start_address,
			  ((block_t *)node_block->type)->size +
			  ((block_t *)node_block->type)->start_address);

		list_t *mini_list = ((block_t *)node_block->type)->miniblock_list;
		node_t *node_miniblock = mini_list->head;

		for (unsigned int j = 0; j < mini_list->size; j++) {
			printf("Miniblock %d:\t\t0x%lX\t\t-\t\t0x%lX\t\t| ",
				   j + 1, ((miniblock_t *)node_miniblock->type)->start_address,
				   ((miniblock_t *)node_miniblock->type)->start_address +
					((miniblock_t *)node_miniblock->type)->size);

			//echivalarea numarului in binar cu permisiunile respective
			int8_t perm = ((miniblock_t *)node_miniblock->type)->perm;

			if ((perm >> 2) == 1) {
				printf("R");
				perm -= 4;
			} else {
				printf("-");
			}
			if ((perm >> 1) == 1) {
				printf("W");
				perm -= 2;
			} else {
				printf("-");
			}
			if (perm == 1)
				printf("X\n");
			else
				printf("-\n");

			node_miniblock = node_miniblock->next;
		}
		printf("Block %d end\n", i + 1);
		node_block = node_block->next;
	}
}

void mprotect(arena_t *arena, uint64_t address, char *permission)
{
	char *comanda = malloc(sizeof(char) * NMAX);
	fgets(comanda, NMAX, stdin);
	free(permission);

	node_t *node_block = arena->alloc_list->head;
	int ok = 0;
	node_t *node_aux;

		for (int i = 0; i < (int)arena->alloc_list->size; i++) {
			list_t *mini_list = ((block_t *)node_block->type)->
								miniblock_list;
			node_t *node_miniblock = mini_list->head;

			for (int j = 0; j < (int)mini_list->size; j++) {
				if (((miniblock_t *)node_miniblock->type)->perm == 0) {
					printf("Invalid address for write\n");
					free(comanda);
					return;
				}
				if (address ==
					((miniblock_t *)node_miniblock->type)->start_address) {
					ok = 1;
					node_aux = node_miniblock;
					break;
				}
				node_miniblock = node_miniblock->next;
			}

			if (ok == 1)
				break;

			node_block = node_block->next;
	}

	if (ok == 0) {
		printf("Invalid address for mprotect.\n");
		free(comanda);
		return;
	}
	//adunam pentru permisiuni numarul care in binar
	//are 1 pe pozitia caracterului ce ofera
	//respectiva permisiune si 0 in rest
	permission = strtok(comanda, " |\n");
	((miniblock_t *)node_aux->type)->perm = 0;
	while (permission) {
		if (strncmp(permission, "PROT_NONE", 9) == 0)
			((miniblock_t *)node_aux->type)->perm = 0;
		else if (strncmp(permission, "PROT_EXEC", 9) == 0)
			((miniblock_t *)node_aux->type)->perm += 1;
		else if (strcmp(permission, "PROT_WRITE") == 0)
			((miniblock_t *)node_aux->type)->perm += 2;
		else if (strncmp(permission, "PROT_READ", 9) == 0)
			((miniblock_t *)node_aux->type)->perm += 4;

		permission = strtok(NULL, " |\n");
	}

	free(comanda);
}

