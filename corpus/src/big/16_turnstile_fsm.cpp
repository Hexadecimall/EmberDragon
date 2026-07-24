/*
 * turnstile_fsm.cpp
 *
 * A finite-state-machine dispatcher modeling a coin-operated turnstile. The
 * machine is driven by a transition table indexed by (current state, input
 * event); each cell names the next state and an optional side-effect action.
 * This is the classic textbook FSM, implemented as a real dispatchable table.
 */

#include <cstdint>

/* The turnstile is either LOCKED (won't let you through) or UNLOCKED (will,
 * once). NUM_STATES bounds the transition table's first dimension. */
enum State {
    STATE_LOCKED = 0,
    STATE_UNLOCKED,
    NUM_STATES
};

/* Inputs the turnstile reacts to. NUM_EVENTS bounds the second dimension. */
enum Event {
    EVENT_COIN = 0, /* a coin was inserted */
    EVENT_PUSH,     /* someone pushed the arm */
    NUM_EVENTS
};

/* Side effects a transition can trigger. The mechanism is abstract here; a real
 * driver would energize a solenoid or sound an alarm. */
enum Action {
    ACTION_NONE = 0,
    ACTION_UNLOCK, /* disengage the latch */
    ACTION_LOCK,   /* re-engage the latch */
    ACTION_ALARM   /* illegal/ignored input */
};

/* A single transition-table cell: where to go and what to do on the way. */
struct Transition {
    State  next;
    Action action;
};

/*
 * The machine bundles its current state with running counters that a real
 * turnstile would expose for auditing (coins taken, people admitted, and
 * rejected pushes that tripped the alarm).
 */
struct Turnstile {
    State    state;
    uint32_t coins;
    uint32_t admitted;
    uint32_t alarms;
};

/*
 * The transition table: TABLE[state][event] gives the resulting transition.
 *  - LOCKED + COIN  -> UNLOCKED, unlatch
 *  - LOCKED + PUSH  -> LOCKED,   alarm (can't push through a locked arm)
 *  - UNLOCKED + COIN-> UNLOCKED, none  (extra coin, no change)
 *  - UNLOCKED + PUSH-> LOCKED,   re-latch behind the person
 */
static const Transition TABLE[NUM_STATES][NUM_EVENTS] = {
    /* STATE_LOCKED   */ { { STATE_UNLOCKED, ACTION_UNLOCK },
                           { STATE_LOCKED,   ACTION_ALARM  } },
    /* STATE_UNLOCKED */ { { STATE_UNLOCKED, ACTION_NONE   },
                           { STATE_LOCKED,   ACTION_LOCK   } }
};

/*
 * Initialize a turnstile in the locked state with zeroed counters.
 * @param t  machine to initialize
 */
void turnstile_init(Turnstile *t) {
    t->state    = STATE_LOCKED;
    t->coins    = 0;
    t->admitted = 0;
    t->alarms   = 0;
}

/*
 * Apply the side effect of a transition to the machine's counters.
 * @param t       machine whose counters to update
 * @param action  effect to apply
 * Kept separate from state advancement so the dispatch logic stays readable.
 */
static void turnstile_apply_action(Turnstile *t, Action action) {
    switch (action) {
        case ACTION_UNLOCK:
            t->coins++;    /* a coin caused the unlock */
            break;
        case ACTION_LOCK:
            t->admitted++; /* a person passed through and re-locked it */
            break;
        case ACTION_ALARM:
            t->alarms++;   /* an illegal push */
            break;
        case ACTION_NONE:
        default:
            break;
    }
}

/*
 * Feed one event to the machine and advance it.
 * @param t   machine to drive
 * @param ev  input event
 * @return the Action that fired (so a caller can render UI/audio), or
 *         ACTION_NONE if the event was out of range and ignored.
 * O(1): a direct two-dimensional table index plus a counter update.
 */
Action turnstile_dispatch(Turnstile *t, Event ev) {
    if (t->state < 0 || t->state >= NUM_STATES ||
        ev < 0 || ev >= NUM_EVENTS) {
        return ACTION_NONE; /* defensive guard against corrupt input */
    }
    const Transition *tr = &TABLE[t->state][ev];
    turnstile_apply_action(t, tr->action);
    t->state = tr->next; /* commit the state change */
    return tr->action;
}

/*
 * Replay a sequence of events and report how many people were admitted.
 * @param t       machine to drive (mutated in place)
 * @param events  array of events to feed in order
 * @param n       number of events
 * @return the admitted count after processing all events.
 */
uint32_t turnstile_run(Turnstile *t, const Event *events, int n) {
    for (int i = 0; i < n; i++) {
        turnstile_dispatch(t, events[i]);
    }
    return t->admitted;
}
