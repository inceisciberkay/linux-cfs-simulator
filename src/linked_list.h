#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include "PCB.h"

typedef struct llnode {
    PCB *ptr_pcb;
    struct llnode *next;
} llnode;

void ll_insert(llnode **head, PCB *ptr_pcb);

void ll_remove(llnode **head, PCB *ptr_pcb);

void ll_clean(llnode *head);

void ll_print(llnode *head);

#endif