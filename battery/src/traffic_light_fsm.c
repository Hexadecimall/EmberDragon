/* enum + struct state machine: a traffic light advancing through phases. */
#include <stdio.h>

enum Light {
    RED = 0,
    GREEN,
    YELLOW
};

struct TrafficLight {
    enum Light state;
    int        ticks_in_state;
    int        cycles;
};

static int duration_for(enum Light s) {
    switch (s) {
        case RED:    return 3;
        case GREEN:  return 2;
        case YELLOW: return 1;
        default:     return 1;
    }
}

static const char *name_for(enum Light s) {
    switch (s) {
        case RED:    return "RED";
        case GREEN:  return "GREEN";
        case YELLOW: return "YELLOW";
        default:     return "?";
    }
}

static void advance(struct TrafficLight *tl) {
    tl->ticks_in_state++;
    if (tl->ticks_in_state >= duration_for(tl->state)) {
        tl->ticks_in_state = 0;
        if (tl->state == YELLOW)
            tl->cycles++;
        tl->state = (tl->state + 1) % 3;
    }
}

int main(void) {
    struct TrafficLight tl = { RED, 0, 0 };

    for (int t = 0; t < 12; ++t) {
        printf("t=%2d  %s\n", t, name_for(tl.state));
        advance(&tl);
    }
    printf("full cycles completed: %d\n", tl.cycles);
    return 0;
}
