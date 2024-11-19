#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"
extern int rq_cur_size;

void ll_insert(llnode **head, PCB *ptr_pcb) {
  if (*head == NULL) {
    *head = (llnode *)malloc(sizeof(llnode));
    (*head)->ptr_pcb = ptr_pcb;
    (*head)->next = NULL;
  } else {
    llnode *prev_head = *head;
    *head = (llnode *)malloc(sizeof(llnode));
    (*head)->ptr_pcb = ptr_pcb;
    (*head)->next = prev_head;
  }
  rq_cur_size++;
}

void ll_remove(llnode **head, PCB *ptr_pcb) {
  if (*head == NULL) {
    return;
  }
  // head is to be removed
  else if ((*head)->ptr_pcb->pid == ptr_pcb->pid) {
    llnode *delptr = *head;
    (*head) = (*head)->next;
    free(delptr);
  } else {
    llnode *prev = *head;
    for (llnode *cur = prev->next; cur != NULL;) {
      if (cur->ptr_pcb->pid == ptr_pcb->pid) {
        llnode *delptr = cur;
        prev->next = cur->next;
        free(delptr);
        cur = prev;
      } else {
        prev = cur;
        cur = cur->next;
      }
    }
  }
  rq_cur_size--;
}

void ll_clean(llnode *head) {
  while (head) {
    llnode *cur = head;
    head = head->next;
    free(cur);
  }
}

void ll_print(llnode *head) {
  while (head) {
    printf("pid: %d     ", head->ptr_pcb->pid);
    fflush(stdout);
    head = head->next;
  }

  printf("\n");
  fflush(stdout);
}
