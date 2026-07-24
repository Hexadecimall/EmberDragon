// Single-thread SPSC-style ring buffer with atomic head/tail indices.
// Power-of-two capacity, lock-free-style index arithmetic, no modulo on hot path.
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <array>
#include <cassert>

template <typename T, std::size_t N>
class RingBuffer {
    static_assert((N & (N - 1)) == 0, "capacity must be a power of two");
public:
    RingBuffer() : head_(0), tail_(0) {}

    bool push(const T& v) noexcept {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t next = head + 1;
        if (next - tail_.load(std::memory_order_acquire) > N) return false;
        buf_[head & MASK] = v;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& out) noexcept {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) return false;
        out = buf_[tail & MASK];
        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

    bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }
    std::size_t size() const noexcept {
        return head_.load(std::memory_order_acquire) -
               tail_.load(std::memory_order_acquire);
    }
    static constexpr std::size_t capacity() noexcept { return N; }

private:
    static constexpr std::size_t MASK = N - 1;
    std::array<T, N> buf_;
    std::atomic<std::size_t> head_; // producer writes
    std::atomic<std::size_t> tail_; // consumer writes
};

int main() {
    RingBuffer<std::uint64_t, 8> rb;
    assert(rb.empty());
    assert(rb.capacity() == 8);

    // Fill to capacity.
    for (std::uint64_t i = 0; i < 8; ++i) assert(rb.push(i));
    assert(rb.size() == 8);
    assert(!rb.push(999)); // full

    // Drain and verify FIFO order.
    std::uint64_t checksum = 0;
    for (int i = 0; i < 8; ++i) {
        std::uint64_t v;
        bool ok = rb.pop(v);
        assert(ok);
        assert(v == static_cast<std::uint64_t>(i));
        checksum += v;
    }
    assert(rb.empty());
    assert(checksum == 28); // 0+1+...+7

    // Interleaved wrap-around stress: push/pop repeatedly so indices exceed N.
    std::uint64_t produced = 0, consumed = 0, acc = 0;
    for (int round = 0; round < 1000; ++round) {
        // burst push up to 5
        for (int k = 0; k < 5; ++k) {
            if (rb.push(produced)) ++produced;
        }
        // burst pop up to 3
        for (int k = 0; k < 3; ++k) {
            std::uint64_t v;
            if (rb.pop(v)) { acc += v; ++consumed; }
        }
    }
    // Drain the rest.
    std::uint64_t v;
    while (rb.pop(v)) { acc += v; ++consumed; }
    assert(produced == consumed);
    // Sum of 0..produced-1 must equal acc.
    std::uint64_t expect = produced * (produced - 1) / 2;
    assert(acc == expect);

    return acc == expect ? 0 : 1;
}
