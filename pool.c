#include "pool.h"
#include "sequence.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct pool {
  int size;
  char *data;
  struct sequence *available; //stores available as positive ints and 
  //                               malloc-ed as negative ints
};

struct pool *pool_create(int size) {
  assert(size > 0);
  struct pool *p = malloc(sizeof(struct pool));
  p->size = size;
  p->data = malloc(sizeof(char) * size);
  p->available = sequence_create();
  sequence_insert_at(p->available, 0, size);
  return p;
}

bool pool_destroy(struct pool *p) {
  assert(p);
  if (sequence_item_at(p->available, 0) == p->size && 
      sequence_length(p->available) == 1) { 
    // there is only one block of memory, and it is available to be malloced to
    free(p->data);
    sequence_destroy(p->available);
    free(p);
    return true;
  }
  return false;
}

// abs(x) returns |x|
// time: O(1)
int abs(int x) {
  if (x < 0) return (-1) * x;
  return x;
}

char *pool_alloc(struct pool *p, int size) {
  assert(p);
  assert(size > 0);
  int index = 0; //keeps track of how much space is passed
  for(int i = 0; i < sequence_length(p->available); i++) {
    int space = sequence_item_at(p->available, i);
    if (space >= size) {// available space
      sequence_remove_at(p->available, i); //remove the old allocation
      if (space > size) {  //re-add available if necessary
        sequence_insert_at(p->available, i, space - size);
      }
      //add malloced space before the available space
      sequence_insert_at(p->available, i, -1 * size); 
      return p->data + index; // return the address
    }
    index += abs(space);
  }
  return NULL;
}

bool pool_free(struct pool *p, char *addr) {
  assert(p);
  assert(addr);
  int index = addr - p->data; //since addr = p->data + index
  int i = 0;
  int space = abs(sequence_item_at(p->available, i));
  //iterate i until it finds the appropriate location
  while (index > 0) {
    space = abs(sequence_item_at(p->available, i));
    index -= space;
    i++;
  }
  space = abs(sequence_item_at(p->available, i));
  int next_space = 0;
  if (i + 1 < sequence_length(p->available)) {
    next_space = sequence_item_at(p->available, i + 1);
  }
  int prev_space = 0;
  if (i - 1 >= 0) {
    prev_space = sequence_item_at(p->available, i - 1);
  }
  if (prev_space <= 0 && next_space <= 0) { //chunks on either side are occupied
    sequence_remove_at(p->available, i);
    sequence_insert_at(p->available, i, space);
  } else if (next_space > 0 && prev_space > 0) {
    sequence_remove_at(p->available, i - 1);
    sequence_remove_at(p->available, i - 1);
    sequence_remove_at(p->available, i - 1);
    sequence_insert_at(p->available, i - 1, space + prev_space + next_space);
  } else if (next_space > 0) {
    sequence_remove_at(p->available, i);
    sequence_remove_at(p->available, i);
    sequence_insert_at(p->available, i, space + next_space);
  } else if (prev_space > 0) {
    sequence_remove_at(p->available, i - 1);
    sequence_remove_at(p->available, i - 1);
    sequence_insert_at(p->available, i - 1, space + prev_space);
  }
  return true;
}

char *pool_realloc(struct pool *p, char *addr, int new_space) {
  assert(p);
  assert(addr);
  assert(new_space > 0);
  int index = addr - p->data; //since addr = p->data + index
  int i = 0;
  int space = abs(sequence_item_at(p->available, i));
  //iterate i until it finds the appropriate location
  while (index > 0) {
    space = abs(sequence_item_at(p->available, i));
    index -= space;
    i++;
  }
  space = abs(sequence_item_at(p->available, i));
  int next_space = 0;
  if (i + 1 < sequence_length(p->available)) {
    next_space = sequence_item_at(p->available, i + 1);
  }
  if (new_space < space) {
    sequence_remove_at(p->available, i);
    if (next_space < 0) { //next chunk is occupied
      sequence_insert_at(p->available, i, space - new_space); //alloc available
    } else { // next chunk is available
      sequence_remove_at(p->available, i);
      sequence_insert_at(p->available, i, space - new_space + next_space);
    }
    sequence_insert_at(p->available, i, (-1) * new_space); //alloc newspace
    return addr;
  } else if (new_space == space) { //unchanged addr and sizes
    return addr;
  } else { // space > newspace
    if (next_space < 0) { //next chunk is occupied
      char *new_addr = pool_alloc(p, new_space);
      if (new_addr == NULL) {
        return NULL; //can't fit this bad boy anywhere so fail
      }
      // else, delete the old address
      sequence_remove_at(p->available, i);
      sequence_insert_at(p->available, i, space);
      return new_addr;
    } else { //next chunk is available
      if (space + next_space < new_space) { //not enough space at cur location
        char *new_addr = pool_alloc(p, new_space);
        if (new_addr == NULL) {
          return NULL;
        }
        // else, delete the old address
        sequence_remove_at(p->available, i);
        sequence_remove_at(p->available, i);
        int prev_space = 0;
        if (i - 1 >= 0 && sequence_item_at(p->available, i - 1) > 0) {
          prev_space = sequence_item_at(p->available, i - 1);
          sequence_remove_at(p->available, i - 1);
          sequence_insert_at(p->available, i - 1, 
                             space + next_space + prev_space);
        } else {
          sequence_insert_at(p->available, i, space + next_space + prev_space);
        }
        return new_addr;
      } else if (space + next_space == new_space) {
        //the exact right amount in curblock and nextblock
        sequence_remove_at(p->available, i);
        sequence_remove_at(p->available, i);
        sequence_insert_at(p->available, i, (-1) * space - next_space);
      } else { // more than enough space at cur location
        sequence_remove_at(p->available, i);
        sequence_remove_at(p->available, i);
        sequence_insert_at(p->available, i, (space + next_space) - new_space);
        sequence_insert_at(p->available, i, (-1) * new_space);
      }
    }
    return addr;
  }
}

void pool_print_active(struct pool *p) {
  assert(p);
  printf("active:");
  bool first = true;
  int index = 0;
  int length = sequence_length(p->available);
  if (length == 1 && sequence_item_at(p->available, 0) > 0) {
    printf(" none\n");
    return;
  } 
  for(int i = 0; i < length; i++) {
    int space = sequence_item_at(p->available, i);
    if (space < 0) {
      if (first) {
        printf(" %d [%d]", index, abs(space));
        first = false;
      } else {
        printf(", %d [%d]", index, abs(space));
      }
    }
    index += abs(space);
  }
  printf("\n");
}

void pool_print_available(struct pool *p) {
  assert(p);
  printf("available:");
  bool first = true;
  int index = 0;
  int length = sequence_length(p->available);
  int at_least_one_available = false;
  for(int i = 0; i < length; i++) {
    int space = sequence_item_at(p->available, i);
    if (space > 0) {
      at_least_one_available = true;
      if (first) {
        printf(" %d [%d]", index, space);
        first = false;
      } else {
        printf(", %d [%d]", index, space);
      }
    }
    index += abs(space);
  }
  if (!at_least_one_available) {
    printf(" none");
  }
  printf("\n");
}
