/*
 * event_router.cpp
 *
 * A topic-based event router that delivers messages to subscribers filtered by
 * channel. Producers publish an event tagged with a channel id; the router
 * walks its subscription list and invokes only those handlers registered for
 * that channel, while keeping per-channel delivery statistics. This is the
 * dispatch core of a simple in-process message bus.
 */

#include <cstdint>

/* Channels an event can be published on. CHANNEL_COUNT bounds the stats array. */
enum Channel {
    CH_SYSTEM = 0,
    CH_INPUT,
    CH_NETWORK,
    CH_AUDIO,
    CHANNEL_COUNT
};

/* Maximum number of registered subscriptions. */
#define MAX_SUBSCRIPTIONS 32

/* A delivered event: the channel it arrived on plus an integer payload (an
 * event code, a key, a byte count, etc., interpreted per channel). */
struct RoutedEvent {
    Channel channel;
    int32_t payload;
};

/* Subscriber callback. Returns true if it "consumed" the event, which the
 * router records but does not use to stop further delivery (events fan out to
 * all matching subscribers). */
typedef bool (*EventHandler)(const RoutedEvent *event, void *user);

/* One subscription: a handler bound to a specific channel, with user context. */
struct Subscription {
    Channel      channel;
    EventHandler handler;
    void        *user;
    bool         active;
};

/*
 * The router holds the subscription table and a per-channel count of events
 * published, which is handy for diagnostics and back-pressure decisions.
 */
struct EventRouter {
    Subscription subscriptions[MAX_SUBSCRIPTIONS];
    uint32_t     published[CHANNEL_COUNT];
};

/*
 * Initialize a router with no subscriptions and zeroed statistics.
 * @param r  router to reset
 */
void router_init(EventRouter *r) {
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        r->subscriptions[i].active = false;
    }
    for (int c = 0; c < CHANNEL_COUNT; c++) {
        r->published[c] = 0;
    }
}

/*
 * Register a handler for one channel.
 * @param r        router to subscribe on
 * @param channel  channel to listen to
 * @param handler  callback invoked for matching events
 * @param user     opaque context forwarded to the handler
 * @return the slot index used as a subscription id (>= 0), or -1 if the table
 *         is full or the channel is out of range.
 * Reuses the first inactive slot. O(n) in MAX_SUBSCRIPTIONS.
 */
int router_subscribe(EventRouter *r, Channel channel,
                     EventHandler handler, void *user) {
    if (channel < 0 || channel >= CHANNEL_COUNT) {
        return -1; /* reject events that could never be routed */
    }
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (!r->subscriptions[i].active) {
            r->subscriptions[i].channel = channel;
            r->subscriptions[i].handler = handler;
            r->subscriptions[i].user    = user;
            r->subscriptions[i].active  = true;
            return i;
        }
    }
    return -1; /* no free slot */
}

/*
 * Cancel a subscription by its id.
 * @param r   router to modify
 * @param id  subscription id returned by router_subscribe
 * @return true if the id referred to an active subscription, false otherwise.
 */
bool router_unsubscribe(EventRouter *r, int id) {
    if (id < 0 || id >= MAX_SUBSCRIPTIONS) {
        return false;
    }
    if (!r->subscriptions[id].active) {
        return false; /* already inactive or never used */
    }
    r->subscriptions[id].active = false;
    return true;
}

/*
 * Publish an event to all subscribers of its channel.
 * @param r      router to publish through
 * @param event  event to deliver (its channel selects the recipients)
 * @return number of handlers that reported consuming the event.
 * Updates the per-channel published counter, then fans the event out to every
 * active subscription whose channel matches. O(n) in MAX_SUBSCRIPTIONS.
 */
int router_publish(EventRouter *r, const RoutedEvent *event) {
    if (event->channel < 0 || event->channel >= CHANNEL_COUNT) {
        return 0; /* unknown channel: nothing to do */
    }
    r->published[event->channel]++;

    int consumed = 0;
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        Subscription *s = &r->subscriptions[i];
        if (s->active && s->channel == event->channel && s->handler != nullptr) {
            if (s->handler(event, s->user)) {
                consumed++;
            }
        }
    }
    return consumed;
}

/*
 * Read how many events have been published on a channel.
 * @param r        router to query
 * @param channel  channel of interest
 * @return the published count, or 0 if the channel id is out of range.
 */
uint32_t router_published_count(const EventRouter *r, Channel channel) {
    if (channel < 0 || channel >= CHANNEL_COUNT) {
        return 0;
    }
    return r->published[channel];
}
