/*
 * deferred_command_buffer.c
 *
 * A record-and-replay command buffer of the kind a renderer or game engine uses
 * to capture work on one pass and execute it later in a single flush. Commands
 * carry a small fixed-size argument payload, are appended sequentially, and are
 * dispatched in recording order through a function table keyed by opcode.
 */

#include <stdint.h>
#include <string.h>

/* Capacity of the command buffer, in commands. */
#define CMD_BUFFER_SIZE 128

/* Opcodes for the recordable commands. Each maps to one executor function. */
typedef enum {
    OP_NOP = 0,
    OP_SET_COLOR,  /* arg0 = packed RGBA */
    OP_MOVE_TO,    /* arg0 = x, arg1 = y */
    OP_DRAW_LINE,  /* arg0..arg3 = x0,y0,x1,y1 */
    OP_COUNT
} OpCode;

/* A recorded command: an opcode plus up to four integer operands. Keeping the
 * payload inline (no pointers) makes the whole buffer trivially copyable. */
typedef struct {
    OpCode  op;
    int32_t args[4];
} Command;

/* The buffer accumulates commands until flushed. `count` is how many are
 * recorded; recording past capacity is reported to the caller. */
typedef struct {
    Command  commands[CMD_BUFFER_SIZE];
    int      count;
} CommandBuffer;

/* Mutable execution context updated as commands run. A real backend would hold
 * GPU handles; here we keep an interpreter-style cursor and pen color plus a
 * tally of drawn primitives for verification. */
typedef struct {
    int32_t  cursor_x;
    int32_t  cursor_y;
    uint32_t pen_color;
    uint32_t lines_drawn;
} RenderContext;

/* Executor signature: applies one command's effect to the render context. */
typedef void (*CommandExecutor)(RenderContext *ctx, const Command *cmd);

/*
 * Reset a command buffer to empty.
 * @param buf  buffer to clear
 */
void command_buffer_reset(CommandBuffer *buf) {
    buf->count = 0;
}

/*
 * Append a command with up to four operands.
 * @param buf  buffer to record into
 * @param op   opcode
 * @param a0..a3  integer operands (unused ones may be 0)
 * @return 1 on success, 0 if the buffer is full (command dropped).
 * O(1).
 */
int command_buffer_record(CommandBuffer *buf, OpCode op,
                          int32_t a0, int32_t a1, int32_t a2, int32_t a3) {
    if (buf->count >= CMD_BUFFER_SIZE) {
        return 0;
    }
    Command *c = &buf->commands[buf->count++];
    c->op      = op;
    c->args[0] = a0;
    c->args[1] = a1;
    c->args[2] = a2;
    c->args[3] = a3;
    return 1;
}

/* ---- Executors: one per opcode ---- */

/* OP_NOP: do nothing. Present so the table has no NULL holes. */
static void exec_nop(RenderContext *ctx, const Command *cmd) {
    (void)ctx;
    (void)cmd;
}

/* OP_SET_COLOR: latch the pen color from arg0. */
static void exec_set_color(RenderContext *ctx, const Command *cmd) {
    ctx->pen_color = (uint32_t)cmd->args[0];
}

/* OP_MOVE_TO: move the drawing cursor without drawing. */
static void exec_move_to(RenderContext *ctx, const Command *cmd) {
    ctx->cursor_x = cmd->args[0];
    ctx->cursor_y = cmd->args[1];
}

/* OP_DRAW_LINE: draw from (x0,y0) to (x1,y1), then leave the cursor at the
 * endpoint so a following draw can chain from it. */
static void exec_draw_line(RenderContext *ctx, const Command *cmd) {
    ctx->lines_drawn++;
    ctx->cursor_x = cmd->args[2];
    ctx->cursor_y = cmd->args[3];
}

/*
 * The dispatch table, indexed by opcode. Order MUST match the OpCode enum so a
 * command's opcode is a direct index into this array.
 */
static const CommandExecutor EXECUTORS[OP_COUNT] = {
    exec_nop,        /* OP_NOP */
    exec_set_color,  /* OP_SET_COLOR */
    exec_move_to,    /* OP_MOVE_TO */
    exec_draw_line   /* OP_DRAW_LINE */
};

/*
 * Initialize a render context to a known default state.
 * @param ctx  context to reset
 */
void render_context_init(RenderContext *ctx) {
    memset(ctx, 0, sizeof(*ctx));
}

/*
 * Execute every recorded command against a context, in order.
 * @param buf  buffer to replay
 * @param ctx  mutable context the commands act on
 * @return number of commands executed.
 * Each command's opcode indexes the executor table; out-of-range opcodes are
 * skipped defensively. O(n) in the number of commands.
 */
int command_buffer_flush(const CommandBuffer *buf, RenderContext *ctx) {
    int executed = 0;
    for (int i = 0; i < buf->count; i++) {
        const Command *c = &buf->commands[i];
        if (c->op >= 0 && c->op < OP_COUNT) {
            EXECUTORS[c->op](ctx, c);
            executed++;
        }
    }
    return executed;
}
