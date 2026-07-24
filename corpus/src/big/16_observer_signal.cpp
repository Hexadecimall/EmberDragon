/*
 * observer_signal.cpp
 *
 * A lightweight publish/subscribe signal that lets multiple listeners react to
 * a value-changed notification, modeling the observer pattern without dynamic
 * allocation. Listeners are stored in a fixed array of callback+context pairs;
 * each can be added, removed by token, and invoked when the signal emits.
 */

#include <cstdint>

/* Maximum simultaneous subscribers. Exceeding this is reported, not silently
 * dropped, so callers can size the signal appropriately. */
#define MAX_LISTENERS 16

/* Listener callback: receives the emitted integer payload plus the opaque
 * `context` pointer that was supplied at subscription time. */
typedef void (*SignalCallback)(int32_t value, void *context);

/* One subscription slot. `active` distinguishes a live slot from a freed one,
 * which lets us reuse holes left by unsubscribe without compacting the array. */
struct Listener {
    SignalCallback callback;
    void          *context;
    int            token;  /* unique handle returned to the subscriber */
    bool           active;
};

/*
 * The signal owns its listener table and a monotonically increasing token
 * counter used to mint unique unsubscribe handles.
 */
struct Signal {
    Listener listeners[MAX_LISTENERS];
    int      next_token;
};

/*
 * Initialize a signal with no subscribers.
 * @param s  signal to reset
 */
void signal_init(Signal *s) {
    s->next_token = 1; /* 0 is reserved to mean "invalid token" */
    for (int i = 0; i < MAX_LISTENERS; i++) {
        s->listeners[i].active = false;
    }
}

/*
 * Subscribe a callback to the signal.
 * @param s   signal to attach to
 * @param cb  callback to invoke on emit
 * @param ctx opaque pointer passed back to the callback (may be null)
 * @return a positive token used later to unsubscribe, or 0 if the signal is
 *         full.
 * Reuses the first inactive slot, so repeated subscribe/unsubscribe cycles do
 * not exhaust capacity. O(n) in MAX_LISTENERS.
 */
int signal_connect(Signal *s, SignalCallback cb, void *ctx) {
    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (!s->listeners[i].active) {
            Listener *l = &s->listeners[i];
            l->callback = cb;
            l->context  = ctx;
            l->token    = s->next_token++;
            l->active   = true;
            return l->token;
        }
    }
    return 0; /* no free slot */
}

/*
 * Remove a previously registered listener by its token.
 * @param s      signal to detach from
 * @param token  handle returned by signal_connect
 * @return true if a matching listener was found and removed, false otherwise.
 * The slot is marked inactive for later reuse rather than physically removed.
 */
bool signal_disconnect(Signal *s, int token) {
    if (token == 0) {
        return false; /* invalid token can never match */
    }
    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (s->listeners[i].active && s->listeners[i].token == token) {
            s->listeners[i].active = false;
            return true;
        }
    }
    return false;
}

/*
 * Count the currently active subscribers.
 * @param s  signal to inspect
 * @return number of live listeners.
 */
int signal_listener_count(const Signal *s) {
    int n = 0;
    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (s->listeners[i].active) {
            n++;
        }
    }
    return n;
}

/*
 * Notify every active listener with a value.
 * @param s      signal to fire
 * @param value  payload delivered to each callback
 * @return number of listeners invoked.
 * Listeners are called in slot order. A null callback in an active slot is
 * skipped defensively so a partially configured slot cannot crash the emit.
 */
int signal_emit(Signal *s, int32_t value) {
    int invoked = 0;
    for (int i = 0; i < MAX_LISTENERS; i++) {
        Listener *l = &s->listeners[i];
        if (l->active && l->callback != nullptr) {
            l->callback(value, l->context);
            invoked++;
        }
    }
    return invoked;
}
