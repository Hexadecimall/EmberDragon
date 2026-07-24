/*
 * reservation.cpp — A room booking system that prevents double-booking via
 * interval-overlap checks.
 *
 * Each reservation occupies a half-open time interval [start, end) on a given
 * room, measured in integer minutes since some epoch. A new booking is accepted
 * only if it does not overlap any existing booking for the same room. The
 * module supports booking, cancellation, and querying availability.
 */

#include <cstdint>

namespace booking {

constexpr int MAX_RESERVATIONS = 64;

/* One confirmed reservation on a single room. */
struct Reservation {
    int32_t id;       /* unique booking id, assigned by the system   */
    int32_t room;     /* room number being reserved                  */
    int64_t start;    /* inclusive start time, minutes since epoch   */
    int64_t end;      /* exclusive end time; must satisfy end > start */
};

class ReservationBook {
public:
    ReservationBook() : count_(0), next_id_(1) {}

    /*
     * Test whether two half-open intervals overlap. [a0,a1) and [b0,b1) overlap
     * iff a0 < b1 and b0 < a1; touching endpoints (a1 == b0) do NOT overlap,
     * which is the desired behavior for back-to-back bookings.
     */
    static bool overlaps(int64_t a0, int64_t a1, int64_t b0, int64_t b1) {
        return a0 < b1 && b0 < a1;
    }

    /*
     * Determine whether `room` is free for the whole interval [start, end).
     * Returns true if no existing reservation on that room overlaps the
     * requested window. O(count).
     */
    bool is_available(int32_t room, int64_t start, int64_t end) const {
        for (int i = 0; i < count_; i++) {
            const Reservation &r = res_[i];
            if (r.room != room)
                continue;                /* different room, cannot conflict */
            if (overlaps(start, end, r.start, r.end))
                return false;
        }
        return true;
    }

    /*
     * Attempt to book `room` for [start, end). Returns the new reservation id on
     * success, or -1 if the interval is invalid (end <= start), the book is
     * full, or the room is already taken for any overlapping time. O(count).
     */
    int32_t book(int32_t room, int64_t start, int64_t end) {
        if (end <= start)
            return -1;                   /* zero-length or inverted interval */
        if (count_ >= MAX_RESERVATIONS)
            return -1;
        if (!is_available(room, start, end))
            return -1;

        Reservation &r = res_[count_++];
        r.id    = next_id_++;
        r.room  = room;
        r.start = start;
        r.end   = end;
        return r.id;
    }

    /*
     * Cancel the reservation with the given id. Returns true if it existed and
     * was removed. The freed slot is backfilled with the last entry, so the
     * stored order is not preserved. O(count).
     */
    bool cancel(int32_t id) {
        for (int i = 0; i < count_; i++) {
            if (res_[i].id == id) {
                res_[i] = res_[--count_];
                return true;
            }
        }
        return false;
    }

    /*
     * Count how many reservations a given room currently holds. Useful for
     * utilization reporting. O(count).
     */
    int reservations_for_room(int32_t room) const {
        int n = 0;
        for (int i = 0; i < count_; i++)
            if (res_[i].room == room)
                n++;
        return n;
    }

    /* Total number of active reservations across all rooms. */
    int total() const { return count_; }

private:
    Reservation res_[MAX_RESERVATIONS];
    int         count_;     /* number of active reservations */
    int32_t     next_id_;   /* next id to hand out           */
};

} // namespace booking
