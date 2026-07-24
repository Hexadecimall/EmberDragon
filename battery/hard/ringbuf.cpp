// Class template RingBuffer<T, N> instantiated for two distinct types.
#include <cstddef>
#include <cstdint>

template <typename T, std::size_t N>
class RingBuffer {
public:
    RingBuffer() : head_(0), tail_(0), count_(0) {}

    bool push(const T& v) {
        if (count_ == N) {
            // overwrite oldest
            data_[tail_] = v;
            tail_ = next(tail_);
            head_ = next(head_);
            return false;
        }
        data_[tail_] = v;
        tail_ = next(tail_);
        ++count_;
        return true;
    }

    bool pop(T& out) {
        if (count_ == 0) return false;
        out = data_[head_];
        head_ = next(head_);
        --count_;
        return true;
    }

    std::size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }
    bool full() const { return count_ == N; }

    // fold over live elements oldest-to-newest
    template <typename F, typename Acc>
    Acc fold(Acc init, F f) const {
        std::size_t idx = head_;
        for (std::size_t i = 0; i < count_; ++i) {
            init = f(init, data_[idx]);
            idx = next(idx);
        }
        return init;
    }

private:
    static std::size_t next(std::size_t i) { return (i + 1) % N; }

    T data_[N];
    std::size_t head_, tail_, count_;
};

struct Vec2 {
    int x, y;
    Vec2(int xx = 0, int yy = 0) : x(xx), y(yy) {}
};

int main() {
    // Instantiation #1: RingBuffer<int, 4>
    RingBuffer<int, 4> ints;
    for (int i = 1; i <= 7; ++i) ints.push(i * i);  // overflows, overwrites
    int popped = 0, popsum = 0;
    int tmp;
    while (ints.pop(tmp)) popsum += tmp, ++popped;

    RingBuffer<int, 4> ints2;
    for (int i = 0; i < 4; ++i) ints2.push(i + 10);
    long long isum = ints2.fold<long long (*)(long long, const int&), long long>(
        0, [](long long a, const int& v) -> long long { return a + v; });

    // Instantiation #2: RingBuffer<Vec2, 3>
    RingBuffer<Vec2, 3> vecs;
    vecs.push(Vec2(1, 2));
    vecs.push(Vec2(3, 4));
    vecs.push(Vec2(5, 6));
    vecs.push(Vec2(7, 8));  // overwrites the (1,2)
    long long vdot = vecs.fold<long long (*)(long long, const Vec2&), long long>(
        0, [](long long a, const Vec2& p) -> long long {
            return a + (long long)p.x * p.y;
        });
    Vec2 vout;
    int vpop = 0;
    while (vecs.pop(vout)) vpop += vout.x + vout.y;

    uint64_t mix = (uint64_t)popsum * 1000000007ull;
    mix ^= (uint64_t)isum << 13;
    mix += (uint64_t)vdot * 31u + (uint64_t)vpop;
    mix ^= (uint64_t)popped + ((uint64_t)ints2.size() << 8);

    return (int)(mix & 0x7f);
}
