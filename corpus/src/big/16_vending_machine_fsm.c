/*
 * vending_machine_fsm.c
 *
 * A state-machine controller for a simple coin-operated vending machine. Unlike
 * a pure table-driven FSM, each state is handled by its own function so that
 * stateful arithmetic (accumulating credit, computing change) lives next to the
 * transition logic. The dispatcher routes the current input to the right state
 * handler and returns the machine's resulting output.
 */

#include <stdint.h>

/* The price of the single product this machine sells, in cents. */
#define PRODUCT_PRICE 75

/* Operating states of the controller. */
typedef enum {
    VM_IDLE = 0,    /* waiting for the first coin */
    VM_COLLECTING,  /* has some credit, not yet enough */
    VM_DISPENSING   /* enough credit; product is being released */
} VendState;

/* Input signals the machine can receive. */
typedef enum {
    IN_COIN_5   = 0, /* a 5-cent coin */
    IN_COIN_25,      /* a 25-cent coin */
    IN_CANCEL,       /* refund request */
    IN_TICK          /* internal clock tick (drives the dispense step) */
} VendInput;

/* Observable outputs produced by a step. */
typedef enum {
    OUT_NONE = 0,
    OUT_REFUND,    /* credit was returned to the customer */
    OUT_DISPENSE,  /* the product was released */
    OUT_CHANGE     /* change was returned after a sale */
} VendOutput;

/* Controller state: which mode it is in and how much credit it holds. */
typedef struct {
    VendState state;
    int32_t   credit;      /* accumulated credit in cents */
    int32_t   last_change; /* change returned by the most recent sale */
} VendingMachine;

/*
 * Initialize the machine to idle with no credit.
 * @param vm  controller to reset
 */
void vending_init(VendingMachine *vm) {
    vm->state       = VM_IDLE;
    vm->credit      = 0;
    vm->last_change = 0;
}

/* Map a coin input to its value in cents; non-coin inputs yield 0. Internal. */
static int32_t coin_value(VendInput in) {
    switch (in) {
        case IN_COIN_5:  return 5;
        case IN_COIN_25: return 25;
        default:         return 0;
    }
}

/*
 * Handle input while idle or collecting credit.
 * @param vm  controller to advance
 * @param in  input signal
 * @return the resulting output.
 * Adds coin value to credit and, once the price is met, transitions to the
 * dispensing state. A cancel refunds all accumulated credit and returns to idle.
 */
static VendOutput step_collecting(VendingMachine *vm, VendInput in) {
    if (in == IN_CANCEL) {
        vm->credit = 0;
        vm->state  = VM_IDLE;
        return OUT_REFUND;
    }

    int32_t v = coin_value(in);
    if (v > 0) {
        vm->credit += v;
        if (vm->credit >= PRODUCT_PRICE) {
            /* Enough money: arm dispensing. Change is computed when the product
             * actually drops on the next tick, not here. */
            vm->state = VM_DISPENSING;
        } else {
            vm->state = VM_COLLECTING;
        }
    }
    /* A TICK while still collecting has no effect. */
    return OUT_NONE;
}

/*
 * Handle the dispense step.
 * @param vm  controller to advance
 * @param in  input signal
 * @return OUT_DISPENSE when the product drops, OUT_CHANGE if change is owed.
 * The product is released on a TICK; any credit beyond the price is recorded as
 * change and the machine returns to idle. Coins inserted during dispensing are
 * ignored to avoid double-charging.
 */
static VendOutput step_dispensing(VendingMachine *vm, VendInput in) {
    if (in != IN_TICK) {
        return OUT_NONE; /* ignore late coins / cancels mid-dispense */
    }
    vm->last_change = vm->credit - PRODUCT_PRICE; /* may be zero */
    vm->credit      = 0;
    vm->state       = VM_IDLE;
    /* Distinguish an exact-change sale from one that owes coins back. */
    return (vm->last_change > 0) ? OUT_CHANGE : OUT_DISPENSE;
}

/*
 * Feed one input to the machine and advance its state.
 * @param vm  controller to drive
 * @param in  input signal
 * @return the output produced by this step.
 * Dispatches to the handler for the current state; O(1).
 */
VendOutput vending_step(VendingMachine *vm, VendInput in) {
    switch (vm->state) {
        case VM_IDLE:
        case VM_COLLECTING:
            return step_collecting(vm, in);
        case VM_DISPENSING:
            return step_dispensing(vm, in);
        default:
            return OUT_NONE;
    }
}

/*
 * Read the change owed by the most recent completed sale.
 * @param vm  controller to query
 * @return change in cents from the last sale (0 if none / not yet sold).
 */
int32_t vending_last_change(const VendingMachine *vm) {
    return vm->last_change;
}
