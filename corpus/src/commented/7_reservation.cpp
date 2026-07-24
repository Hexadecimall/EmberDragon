/*
 * reservation.cpp
 *
 * A room-booking system that manages time-slot reservations for a small set
 * of rooms. Bookings are kept sorted by start time per room, which lets the
 * module detect overlaps, find the next free window, and report utilization
 * without any dynamic allocation or floating-point math.
 */

#include <cstdint>
#include <cstring>

static const int MAX_ROOMS    = 16;
static const int MAX_BOOKINGS = 32;   /* per room */

/* A half-open time interval [start, end) measured in minutes from midnight,
 * tagged with the id of the party that holds it. */
struct Booking {
    uint32_t start;
    uint32_t end;
    uint32_t holder_id;
};

/* One bookable room. Bookings are maintained in ascending order of `start`,
 * an invariant every mutating function preserves. */
struct Room {
    Booking bookings[MAX_BOOKINGS];
    int count;
};

/* The whole facility. */
struct Facility {
    Room rooms[MAX_ROOMS];
    int room_count;
};

/*
 * Initialize a facility with a given number of empty rooms.
 * Parameters: f - facility; rooms - room count (clamped to [0, MAX_ROOMS]).
 * Returns nothing.
 */
void facility_init(Facility *f, int rooms) {
    if (rooms < 0) rooms = 0;
    if (rooms > MAX_ROOMS) rooms = MAX_ROOMS;
    f->room_count = rooms;
    for (int i = 0; i < rooms; i++)
        f->rooms[i].count = 0;
}

/*
 * Test whether two half-open intervals overlap.
 * Parameters: a_start,a_end,b_start,b_end - the two intervals.
 * Returns 1 if they share any instant, 0 otherwise. Touching end-to-start
 * (a_end == b_start) does NOT count as an overlap. O(1).
 */
static int intervals_overlap(uint32_t a_start, uint32_t a_end,
                             uint32_t b_start, uint32_t b_end) {
    return a_start < b_end && b_start < a_end;
}

/*
 * Attempt to book a time slot in a room.
 * Parameters: f, room (index), start, end, holder (party id).
 * Returns 1 on success. Returns 0 if the room index is invalid, the interval
 * is empty/inverted (start >= end), the room is full, or the slot conflicts
 * with an existing booking. On success the booking is inserted so the room's
 * list stays sorted by start time. O(n) for the conflict scan and insert.
 */
int facility_book(Facility *f, int room, uint32_t start, uint32_t end,
                  uint32_t holder) {
    if (room < 0 || room >= f->room_count)
        return 0;
    if (start >= end)
        return 0;                       /* reject empty or inverted slots */
    Room *r = &f->rooms[room];
    if (r->count >= MAX_BOOKINGS)
        return 0;
    /* Find the insertion point and reject any conflict along the way. */
    int pos = r->count;
    for (int i = 0; i < r->count; i++) {
        if (intervals_overlap(start, end,
                              r->bookings[i].start, r->bookings[i].end))
            return 0;                   /* double-booking not allowed */
        if (r->bookings[i].start > start && pos == r->count)
            pos = i;                    /* first booking that comes after us */
    }
    /* Shift later bookings right by one to open a hole at `pos`. */
    for (int i = r->count; i > pos; i--)
        r->bookings[i] = r->bookings[i - 1];
    r->bookings[pos].start = start;
    r->bookings[pos].end = end;
    r->bookings[pos].holder_id = holder;
    r->count++;
    return 1;
}

/*
 * Cancel the first booking in a room held by a given party.
 * Parameters: f, room (index), holder (party id).
 * Returns 1 if a booking was removed, 0 if none matched or the room is bad.
 * Remaining bookings are compacted to preserve the sorted invariant. O(n).
 */
int facility_cancel(Facility *f, int room, uint32_t holder) {
    if (room < 0 || room >= f->room_count)
        return 0;
    Room *r = &f->rooms[room];
    for (int i = 0; i < r->count; i++) {
        if (r->bookings[i].holder_id == holder) {
            /* Close the gap by shifting everything after i left by one. */
            for (int j = i; j < r->count - 1; j++)
                r->bookings[j] = r->bookings[j + 1];
            r->count--;
            return 1;
        }
    }
    return 0;
}

/*
 * Find the earliest start time at or after `from` at which a slot of the
 * given duration fits in a room without conflict.
 * Parameters: f, room (index), from (earliest acceptable start),
 *             duration (minutes, must be > 0).
 * Returns the chosen start time, or UINT32_MAX if the room is invalid, the
 * duration is non-positive, or no gap before the day's bookings is found.
 * Because bookings are sorted, a single forward sweep suffices. O(n).
 */
uint32_t facility_next_free(const Facility *f, int room, uint32_t from,
                            uint32_t duration) {
    if (room < 0 || room >= f->room_count || duration == 0)
        return UINT32_MAX;
    const Room *r = &f->rooms[room];
    uint32_t candidate = from;
    for (int i = 0; i < r->count; i++) {
        /* A booking entirely before our candidate cannot block us. */
        if (r->bookings[i].end <= candidate)
            continue;
        /* If the slot fits before this booking starts, we're done. */
        if (candidate + duration <= r->bookings[i].start)
            return candidate;
        /* Otherwise jump past this booking and try again. */
        candidate = r->bookings[i].end;
    }
    return candidate;                   /* fits after the last booking */
}

/*
 * Sum the total minutes reserved in a room.
 * Parameters: f, room (index).
 * Returns booked minutes, or 0 for an invalid room. O(n).
 */
uint64_t facility_booked_minutes(const Facility *f, int room) {
    if (room < 0 || room >= f->room_count)
        return 0;
    const Room *r = &f->rooms[room];
    uint64_t total = 0;
    for (int i = 0; i < r->count; i++)
        total += (uint64_t)(r->bookings[i].end - r->bookings[i].start);
    return total;
}
