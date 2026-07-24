#include <cstdint>

class MeetingScheduler {
public:
    struct Slot {
        int32_t start_minute;
        int32_t end_minute;
        int32_t room_id;
        int32_t booked;
    };

    static const int MAX_SLOTS = 96;

    MeetingScheduler() : slot_count(0) {}

    int add_slot(int32_t start, int32_t end, int32_t room) {
        if (slot_count >= MAX_SLOTS || end <= start) {
            return -1;
        }
        Slot *s = &slots[slot_count];
        s->start_minute = start;
        s->end_minute = end;
        s->room_id = room;
        s->booked = 0;
        slot_count++;
        return slot_count - 1;
    }

    static int overlaps(const Slot *a, int32_t start, int32_t end, int32_t room) {
        if (a->room_id != room) {
            return 0;
        }
        if (end <= a->start_minute || start >= a->end_minute) {
            return 0;
        }
        return 1;
    }

    int book(int32_t start, int32_t end, int32_t room) {
        if (end <= start) {
            return -1;
        }
        for (int i = 0; i < slot_count; i++) {
            if (slots[i].booked && overlaps(&slots[i], start, end, room)) {
                return -2;
            }
        }
        return add_slot_booked(start, end, room);
    }

    int cancel(int32_t start, int32_t room) {
        for (int i = 0; i < slot_count; i++) {
            if (slots[i].booked && slots[i].start_minute == start && slots[i].room_id == room) {
                slots[i].booked = 0;
                return 0;
            }
        }
        return -1;
    }

    int32_t total_booked_minutes(int32_t room) {
        int32_t sum = 0;
        for (int i = 0; i < slot_count; i++) {
            if (slots[i].booked && slots[i].room_id == room) {
                sum += slots[i].end_minute - slots[i].start_minute;
            }
        }
        return sum;
    }

    int find_free_room(int32_t start, int32_t end, int32_t room_max) {
        for (int32_t room = 0; room < room_max; room++) {
            int conflict = 0;
            for (int i = 0; i < slot_count; i++) {
                if (slots[i].booked && overlaps(&slots[i], start, end, room)) {
                    conflict = 1;
                    break;
                }
            }
            if (!conflict) {
                return room;
            }
        }
        return -1;
    }

private:
    int add_slot_booked(int32_t start, int32_t end, int32_t room) {
        int idx = add_slot(start, end, room);
        if (idx >= 0) {
            slots[idx].booked = 1;
        }
        return idx;
    }

    Slot slots[MAX_SLOTS];
    int slot_count;
};
