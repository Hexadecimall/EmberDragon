// Tiny cooperative scheduler over function pointers + a state array.
// Each task is a small step-function resumed via a saved program-counter in
// its state slot. The scheduler round-robins runnable tasks until all finish.
#include <cstdint>
#include <array>
#include <cassert>

enum class Status : std::uint8_t { Ready, Blocked, Done };

struct TaskState {
    int pc = 0;          // resume point inside the task's step function
    std::int64_t acc = 0;
    std::int64_t scratch = 0;
    Status status = Status::Ready;
    int ticks = 0;       // how many times this task ran
};

struct Scheduler; // fwd

// A task step does a bounded chunk of work, updates its pc, then yields by
// returning. Returning with status==Done removes it from rotation.
using StepFn = void (*)(TaskState&, Scheduler&);

struct Scheduler {
    static constexpr int MAX = 8;
    std::array<StepFn, MAX> fns{};
    std::array<TaskState, MAX> states{};
    int count = 0;
    std::int64_t mailbox = 0; // shared channel between tasks

    int spawn(StepFn fn) {
        assert(count < MAX);
        int id = count++;
        fns[id] = fn;
        states[id] = TaskState{};
        return id;
    }

    // Run round-robin until every task is Done. Returns total ticks executed.
    int run() {
        int total = 0;
        for (;;) {
            bool any = false;
            for (int i = 0; i < count; ++i) {
                if (states[i].status == Status::Done) continue;
                any = true;
                if (states[i].status == Status::Ready) {
                    fns[i](states[i], *this);
                    ++total;
                } else if (states[i].status == Status::Blocked) {
                    // Unblock only once the mailbox has a value to consume.
                    if (mailbox != 0) states[i].status = Status::Ready;
                }
            }
            if (!any) break;
        }
        return total;
    }
};

// Task A: sums 1..5 across multiple resumptions, one add per step.
void taskSum(TaskState& s, Scheduler&) {
    s.ticks++;
    switch (s.pc) {
        case 0: s.scratch = 1; s.acc = 0; s.pc = 1; return;
        case 1:
            s.acc += s.scratch;
            if (s.scratch >= 5) { s.pc = 2; return; }
            s.scratch++;
            return; // yield, resume at case 1
        case 2: s.status = Status::Done; return;
    }
}

// Task B: produces a value into the mailbox after a couple of warm-up ticks.
void taskProducer(TaskState& s, Scheduler& sch) {
    s.ticks++;
    switch (s.pc) {
        case 0: s.pc = 1; return;            // warm up
        case 1: s.pc = 2; return;            // warm up
        case 2: sch.mailbox = 42; s.acc = 42; s.status = Status::Done; return;
    }
}

// Task C: blocks until the producer publishes, then consumes the mailbox.
void taskConsumer(TaskState& s, Scheduler& sch) {
    s.ticks++;
    if (s.pc == 0) {
        if (sch.mailbox == 0) { s.status = Status::Blocked; return; }
        s.acc = sch.mailbox;
        s.pc = 1;
        s.status = Status::Done;
    }
}

int main() {
    Scheduler sch;
    int a = sch.spawn(&taskSum);
    int b = sch.spawn(&taskProducer);
    int c = sch.spawn(&taskConsumer);

    int total = sch.run();

    // Task A computed 1+2+3+4+5 = 15.
    assert(sch.states[a].acc == 15);
    // Producer published 42.
    assert(sch.states[b].acc == 42);
    // Consumer woke from Blocked and read the mailbox.
    assert(sch.states[c].acc == 42);
    // Everyone finished.
    for (int i = 0; i < sch.count; ++i)
        assert(sch.states[i].status == Status::Done);
    // The consumer must have been blocked at least one cycle.
    assert(sch.states[c].ticks >= 2);
    assert(total > 0);

    return (sch.states[a].acc + sch.states[c].acc) == 57 ? 0 : 1;
}
