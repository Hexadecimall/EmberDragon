/*
 * trafficfsm.c — A turnstile and traffic-light finite state machine driver.
 *
 * Implements two small deterministic finite state machines: a coin-operated
 * turnstile and a four-phase traffic light. Each is modelled as an explicit
 * state enum plus a transition function that maps (state, event) to the next
 * state, demonstrating table-free FSM dispatch with clear, auditable rules.
 */

#include <stdint.h>

/* ---- Turnstile FSM --------------------------------------------------- */

/* The turnstile is either locked (blocks passage) or unlocked (admits one). */
typedef enum TurnstileState {
    TS_LOCKED,
    TS_UNLOCKED
} TurnstileState;

/* External stimuli the turnstile reacts to. */
typedef enum TurnstileEvent {
    TS_COIN,   /* a coin is inserted */
    TS_PUSH    /* someone pushes the arm */
} TurnstileEvent;

/*
 * Compute the turnstile's next state for a given event.
 * Parameters: state — current state; event — the stimulus. Returns the
 * resulting state. A coin unlocks; a push through an unlocked turnstile relocks
 * it; pushing while locked or inserting a coin while unlocked are no-ops that
 * leave the state unchanged. O(1).
 */
TurnstileState turnstileStep(TurnstileState state, TurnstileEvent event) {
    if (state == TS_LOCKED) {
        /* Only a coin makes progress; an idle push is ignored. */
        return (event == TS_COIN) ? TS_UNLOCKED : TS_LOCKED;
    } else { /* TS_UNLOCKED */
        /* A push consumes the entry and relocks; extra coins do nothing. */
        return (event == TS_PUSH) ? TS_LOCKED : TS_UNLOCKED;
    }
}

/*
 * Count how many people successfully pass through the turnstile for a sequence
 * of events.
 * Parameters: events — array of events; count — its length. Returns the number
 * of admitted passages (pushes that occurred while unlocked). Starts from the
 * locked state. O(count).
 */
int turnstilePassages(const TurnstileEvent *events, int count) {
    TurnstileState state = TS_LOCKED;
    int passages = 0;
    for (int i = 0; i < count; i++) {
        /* A passage happens exactly when a push fires in the unlocked state. */
        if (state == TS_UNLOCKED && events[i] == TS_PUSH)
            passages++;
        state = turnstileStep(state, events[i]);
    }
    return passages;
}

/* ---- Traffic light FSM ----------------------------------------------- */

/*
 * Four-phase cycle: GREEN -> YELLOW -> RED -> RED_YELLOW -> GREEN ...
 * RED_YELLOW is the brief "get ready" phase used in some countries before
 * green, which makes the cycle four states rather than three.
 */
typedef enum LightState {
    LIGHT_GREEN,
    LIGHT_YELLOW,
    LIGHT_RED,
    LIGHT_RED_YELLOW
} LightState;

/*
 * Advance the traffic light by one phase.
 * Parameters: state — current phase. Returns the next phase in the fixed cycle.
 * The cycle is closed, so RED_YELLOW wraps back to GREEN. O(1).
 */
LightState lightAdvance(LightState state) {
    switch (state) {
    case LIGHT_GREEN:      return LIGHT_YELLOW;
    case LIGHT_YELLOW:     return LIGHT_RED;
    case LIGHT_RED:        return LIGHT_RED_YELLOW;
    case LIGHT_RED_YELLOW: return LIGHT_GREEN;   /* wrap around */
    default:               return LIGHT_RED;     /* fail safe to RED */
    }
}

/*
 * Report whether traffic may proceed in a given phase.
 * Parameters: state — the phase. Returns 1 if vehicles may move (GREEN only),
 * 0 otherwise. YELLOW counts as "stop if safe", so it is treated as not-go. O(1).
 */
int lightIsGo(LightState state) {
    return state == LIGHT_GREEN ? 1 : 0;
}

/*
 * Simulate the light for a number of phase transitions and return its final
 * phase.
 * Parameters: start — initial phase; steps — number of advances to apply.
 * Returns the phase after that many transitions. Negative steps are treated as
 * zero. O(steps); equivalently start advanced (steps mod 4) times.
 */
LightState lightSimulate(LightState start, int steps) {
    LightState state = start;
    for (int i = 0; i < steps; i++)
        state = lightAdvance(state);
    return state;
}
