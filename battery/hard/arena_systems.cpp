// Fixed-size arena / bump allocator handing out typed structs.
// No heap after construction; alignment-correct; supports rewind to a marker.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <cassert>

template <std::size_t Capacity>
class Arena {
public:
    Arena() noexcept : offset_(0) {}

    using Marker = std::size_t;
    Marker mark() const noexcept { return offset_; }
    void rewind(Marker m) noexcept { offset_ = m; }
    void reset() noexcept { offset_ = 0; }
    std::size_t used() const noexcept { return offset_; }
    std::size_t remaining() const noexcept { return Capacity - offset_; }

    // Allocate raw aligned bytes, or nullptr if it would overflow.
    void* allocate(std::size_t bytes, std::size_t align) noexcept {
        std::size_t cur = reinterpret_cast<std::uintptr_t>(storage_) + offset_;
        std::size_t aligned = (cur + (align - 1)) & ~(align - 1);
        std::size_t pad = aligned - cur;
        if (offset_ + pad + bytes > Capacity) return nullptr;
        offset_ += pad + bytes;
        return reinterpret_cast<void*>(aligned);
    }

    // Construct one T in place.
    template <typename T, typename... Args>
    T* make(Args&&... args) noexcept {
        void* p = allocate(sizeof(T), alignof(T));
        if (!p) return nullptr;
        return ::new (p) T(static_cast<Args&&>(args)...);
    }

    // Construct an array of N default Ts.
    template <typename T>
    T* makeArray(std::size_t n) noexcept {
        void* p = allocate(sizeof(T) * n, alignof(T));
        if (!p) return nullptr;
        T* arr = reinterpret_cast<T*>(p);
        for (std::size_t i = 0; i < n; ++i) ::new (&arr[i]) T();
        return arr;
    }

private:
    alignas(std::max_align_t) unsigned char storage_[Capacity];
    std::size_t offset_;
};

struct Particle {
    double x, y, vx, vy;
    std::uint32_t id;
    Particle() : x(0), y(0), vx(0), vy(0), id(0) {}
    Particle(double px, double py, std::uint32_t i)
        : x(px), y(py), vx(1.0), vy(-1.0), id(i) {}
};

int main() {
    Arena<4096> arena;

    // Hand out a batch of structs.
    Particle* a = arena.make<Particle>(1.0, 2.0, 7u);
    Particle* b = arena.make<Particle>(3.0, 4.0, 9u);
    assert(a && b);
    assert(reinterpret_cast<std::uintptr_t>(a) % alignof(Particle) == 0);
    assert(reinterpret_cast<std::uintptr_t>(b) % alignof(Particle) == 0);

    // Rewind: allocate a scratch array, then roll it back.
    Arena<4096>::Marker m = arena.mark();
    Particle* scratch = arena.makeArray<Particle>(64);
    assert(scratch != nullptr);
    double sum = 0;
    for (int i = 0; i < 64; ++i) { scratch[i].id = static_cast<std::uint32_t>(i); sum += i; }
    arena.rewind(m);
    assert(arena.used() == m);

    // After rewind the space is reusable.
    Particle* c = arena.make<Particle>(5.0, 6.0, 11u);
    assert(c == reinterpret_cast<Particle*>(scratch)); // same address reused
    (void)c; (void)sum;

    // Exhaust the arena to test overflow handling.
    int count = 0;
    while (arena.make<Particle>(0.0, 0.0, 0u)) ++count;
    assert(count > 0);
    assert(arena.make<Particle>() == nullptr);

    return static_cast<int>(a->id + b->id) == 16 ? 0 : 1;
}
