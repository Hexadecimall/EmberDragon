/*
 * command_dispatch.c
 *
 * A small command-dispatch table that maps short string command names (the
 * kind you would type into a debug console or REPL) to handler functions.
 * Lookup is a linear scan over a registration array, and each handler receives
 * an argument count plus an integer argument vector and returns a status code.
 */

#include <stdint.h>
#include <string.h>

/* Maximum number of distinct commands that can be registered. */
#define MAX_COMMANDS 32

/* Status codes returned by handlers and by the dispatcher itself. */
typedef enum {
    CMD_OK            = 0,
    CMD_ERR_NOT_FOUND = -1, /* no command matched the given name */
    CMD_ERR_ARITY     = -2, /* wrong number of arguments supplied */
    CMD_ERR_FULL      = -3, /* dispatch table has no free slots */
    CMD_ERR_RANGE     = -4  /* a handler rejected an argument value */
} CmdStatus;

/* Handler signature: receives the argument count and an array of integer
 * arguments, returns CMD_OK or a negative CmdStatus on failure. */
typedef int (*CommandHandler)(int argc, const int32_t *argv);

/* One entry in the dispatch table. `min_argc`/`max_argc` bound the accepted
 * argument count so the dispatcher can validate arity before invoking. */
typedef struct {
    const char     *name;
    CommandHandler  handler;
    int             min_argc;
    int             max_argc;
} CommandEntry;

/* The dispatch table itself. `count` is the number of live entries. */
typedef struct {
    CommandEntry entries[MAX_COMMANDS];
    int          count;
} CommandTable;

/*
 * Initialize an empty dispatch table.
 * @param table  table to reset (must be non-NULL)
 */
void command_table_init(CommandTable *table) {
    table->count = 0;
}

/*
 * Register a command by name.
 * @param table     table to add to
 * @param name      command name (not copied; must outlive the table)
 * @param handler   function to invoke on a match
 * @param min_argc  smallest acceptable argument count
 * @param max_argc  largest acceptable argument count
 * @return CMD_OK, or CMD_ERR_FULL if the table is at capacity.
 * Registering a duplicate name is allowed; the first match wins at lookup.
 */
int command_register(CommandTable *table, const char *name,
                     CommandHandler handler, int min_argc, int max_argc) {
    if (table->count >= MAX_COMMANDS) {
        return CMD_ERR_FULL;
    }
    CommandEntry *e = &table->entries[table->count];
    e->name     = name;
    e->handler  = handler;
    e->min_argc = min_argc;
    e->max_argc = max_argc;
    table->count++;
    return CMD_OK;
}

/*
 * Find a command entry by name.
 * @param table  table to search
 * @param name   name to look for
 * @return pointer to the matching entry, or NULL if none.
 * O(n) linear scan; n is the number of registered commands.
 */
const CommandEntry *command_find(const CommandTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].name, name) == 0) {
            return &table->entries[i];
        }
    }
    return NULL;
}

/*
 * Look up a command by name and invoke its handler with validation.
 * @param table  table to dispatch through
 * @param name   command name to run
 * @param argc   number of arguments in argv
 * @param argv   integer argument vector (may be NULL if argc == 0)
 * @return the handler's return value, or a negative CmdStatus if the command
 *         was not found or the argument count fell outside the entry's bounds.
 */
int command_dispatch(const CommandTable *table, const char *name,
                     int argc, const int32_t *argv) {
    const CommandEntry *e = command_find(table, name);
    if (e == NULL) {
        return CMD_ERR_NOT_FOUND;
    }
    /* Reject the call early if the caller violated the declared arity, so each
     * handler can assume argc is already within range. */
    if (argc < e->min_argc || argc > e->max_argc) {
        return CMD_ERR_ARITY;
    }
    return e->handler(argc, argv);
}

/* ---- Example handlers, to show the table in use ---- */

/*
 * "add": sum every argument.
 * @return CMD_OK; the computed sum is not surfaced here (a real console would
 *         print it) but the loop demonstrates argument traversal.
 */
static int handle_add(int argc, const int32_t *argv) {
    int32_t sum = 0;
    for (int i = 0; i < argc; i++) {
        sum += argv[i];
    }
    (void)sum;
    return CMD_OK;
}

/*
 * "clamp": verify a value lies within [lo, hi].
 * Expects exactly three args: value, lo, hi.
 * @return CMD_OK if lo <= value <= hi, otherwise CMD_ERR_RANGE.
 */
static int handle_clamp(int argc, const int32_t *argv) {
    (void)argc; /* arity already validated by the dispatcher */
    int32_t value = argv[0];
    int32_t lo    = argv[1];
    int32_t hi    = argv[2];
    if (value < lo || value > hi) {
        return CMD_ERR_RANGE;
    }
    return CMD_OK;
}

/*
 * Populate a table with the built-in example commands.
 * @param table  table to fill (should be freshly initialized)
 * @return number of commands successfully registered.
 */
int command_register_builtins(CommandTable *table) {
    int registered = 0;
    if (command_register(table, "add",   handle_add,   1, 8) == CMD_OK) registered++;
    if (command_register(table, "clamp", handle_clamp, 3, 3) == CMD_OK) registered++;
    return registered;
}
