/*
 * undo_redo_stack.c
 *
 * A bounded undo/redo history for an editor-style application. Each recorded
 * action is an opaque, reversible "delta" (here, an integer add/subtract on a
 * document value). Pushing a new action clears any redo history, mirroring the
 * familiar behavior of every text editor.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Maximum number of actions retained in history. When this is exceeded the
 * oldest action is forgotten so the undo depth stays bounded. */
#define HISTORY_LIMIT 128

/* A reversible edit. `delta` is added to the document value when the action is
 * applied (redo) and subtracted when it is reverted (undo). `label` names the
 * action for an "Undo Rename"-style menu entry. */
typedef struct {
    int32_t delta;
    char    label[24];
} Action;

/*
 * The history maintains a single array of actions and a cursor.
 *  - actions[0 .. cursor-1]    have been applied and can be undone
 *  - actions[cursor .. count-1] have been undone and can be redone
 * `value` is the current document state the actions operate on.
 */
typedef struct {
    Action  actions[HISTORY_LIMIT];
    int     count;   /* total actions retained (applied + undone) */
    int     cursor;  /* boundary between applied and undone actions */
    int32_t value;   /* current document value */
} History;

/*
 * Initialize a history with a starting document value.
 * @param h      history to initialize
 * @param start  initial document value
 */
void history_init(History *h, int32_t start) {
    h->count  = 0;
    h->cursor = 0;
    h->value  = start;
}

/*
 * Record and apply a new action, discarding any redoable future.
 * @param h      history to mutate
 * @param delta  amount to add to the document value
 * @param label  short human-readable name (truncated to 23 chars)
 * Applying a fresh action invalidates the redo stack: actions at or past the
 * cursor are dropped before the new one is appended. If the history is full the
 * oldest action is evicted, keeping recent edits at the cost of deep undo.
 */
void history_do(History *h, int32_t delta, const char *label) {
    /* Truncate the redo branch: anything after the cursor is now unreachable. */
    h->count = h->cursor;

    if (h->count == HISTORY_LIMIT) {
        /* Evict the oldest action by shifting the array down one slot. The
         * evicted delta stays folded into `value`, so document state is intact;
         * we simply lose the ability to undo that far back. O(n) shift. */
        memmove(&h->actions[0], &h->actions[1],
                (HISTORY_LIMIT - 1) * sizeof(Action));
        h->count--;
        h->cursor--;
    }

    Action *a = &h->actions[h->count];
    a->delta = delta;
    /* Copy the label defensively and guarantee NUL termination. */
    strncpy(a->label, label, sizeof(a->label) - 1);
    a->label[sizeof(a->label) - 1] = '\0';

    h->value += delta; /* apply immediately */
    h->count++;
    h->cursor = h->count;
}

/*
 * Whether an undo is currently possible.
 * @return 1 if there is at least one applied action, else 0.
 */
int history_can_undo(const History *h) {
    return h->cursor > 0;
}

/*
 * Whether a redo is currently possible.
 * @return 1 if at least one action has been undone, else 0.
 */
int history_can_redo(const History *h) {
    return h->cursor < h->count;
}

/*
 * Revert the most recently applied action.
 * @param h  history to mutate
 * @return 1 if an action was undone, 0 if there was nothing to undo.
 * Moves the cursor back one and subtracts that action's delta from the value.
 */
int history_undo(History *h) {
    if (!history_can_undo(h)) {
        return 0;
    }
    h->cursor--;
    h->value -= h->actions[h->cursor].delta;
    return 1;
}

/*
 * Re-apply the most recently undone action.
 * @param h  history to mutate
 * @return 1 if an action was redone, 0 if there was nothing to redo.
 * Re-adds the action's delta and advances the cursor.
 */
int history_redo(History *h) {
    if (!history_can_redo(h)) {
        return 0;
    }
    h->value += h->actions[h->cursor].delta;
    h->cursor++;
    return 1;
}

/*
 * Return the label of the action that an undo would revert, for menu display.
 * @param h  history to inspect
 * @return the label string, or NULL when nothing can be undone.
 */
const char *history_undo_label(const History *h) {
    if (!history_can_undo(h)) {
        return NULL;
    }
    return h->actions[h->cursor - 1].label;
}
