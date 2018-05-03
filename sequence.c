#include "cs136-tracing.h"
#include <stdlib.h>
#include <stdio.h>
#include "sequence.h"

struct sequence {
  int *data;
  int len;
  int maxlen;
};


struct sequence *sequence_create(void) {
  struct sequence *s = malloc(sizeof(struct sequence));
  s->data = malloc(sizeof(int));
  s->len = 0;
  s->maxlen = 1;
  return s;
}


void sequence_destroy(struct sequence *seq) {
  free(seq->data);
  free(seq);
}


int sequence_length(const struct sequence *seq) {
  return seq->len;
}


int sequence_item_at(const struct sequence *seq, int pos) {
  return seq->data[pos];
}

void enlarge(struct sequence *seq) {
  int len = seq->len;
  int *newseq = malloc(sizeof(int) * len * 2);
  for(int i = 0; i < len; i++) {
    newseq[i] = seq->data[i];
  }
  int *backup_pointer = seq->data;
  seq->data = newseq;
  free(backup_pointer);
  if (1 > len * 2) {
    seq->maxlen = 1;
  } else {
    seq->maxlen = len * 2;
  }
}

void shrink(struct sequence *seq) {
  int len = seq->len;
  int *newseq = malloc(sizeof(int) * len * 2);
  for(int i = 0; i < len; i ++) {
    newseq[i] = seq->data[i];
  }
  int *backup_pointer = seq->data;
  seq->data = newseq;
  free(backup_pointer);
  seq->maxlen = seq->len * 2;
}

void sequence_insert_at(struct sequence *seq, int pos, int val) {
  int len = seq->len;
  if (len != 0) {
    if (len == seq->maxlen) enlarge(seq);
    //shift over every value after pos
    for(int i = len - 1; i >= pos; i--) {
      seq->data[i + 1] = seq->data[i];
    }
    // data[pos] has been duplicated to data[pos + 1] so it can be overwritten
    //   by the new value
  }
  seq->data[pos] = val;
  seq->len ++;
}


int sequence_remove_at(struct sequence *seq, int pos) {
  int ret = seq->data[pos];
  int len = seq->len;
  for (int i = pos; i < len - 1; i++) {
    seq->data[i] = seq->data[i + 1];
  }
  seq->len --;
  if (seq->len * 4 < seq->maxlen && seq->len != 0) {
    shrink(seq);
  }
  return ret;
}


void sequence_print(const struct sequence *seq) {
  int len = seq->len;
  if (!len) {
    printf("[empty]\n");
    return;
  }
  printf("[%d", seq->data[0]);
  for (int i = 1; i < len; i++) {
    printf(",%d", seq->data[i]);
  }
  printf("]\n");
}