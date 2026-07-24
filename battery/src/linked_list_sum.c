/* Singly linked list: push-front, length, sum. Heap nodes + pointer chasing. */
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

static Node *push_front(Node *head, int v) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->value = v;
    n->next = head;
    return n;
}

static int length(const Node *head) {
    int count = 0;
    for (const Node *p = head; p != NULL; p = p->next)
        count++;
    return count;
}

static long sum(const Node *head) {
    long total = 0;
    for (const Node *p = head; p != NULL; p = p->next)
        total += p->value;
    return total;
}

int main(void) {
    Node *head = NULL;
    for (int i = 1; i <= 6; i++)
        head = push_front(head, i * 10);
    printf("len=%d sum=%ld\n", length(head), sum(head));
    while (head != NULL) {
        Node *next = head->next;
        free(head);
        head = next;
    }
    return 0;
}
