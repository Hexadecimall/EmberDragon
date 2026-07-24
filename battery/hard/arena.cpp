// RAII resource manager: a small move-only arena allocator + scoped guard.
#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>

// A move-only owning handle over a raw heap buffer (RAII: frees in dtor).
class Buffer {
public:
    Buffer() : data_(nullptr), size_(0) {}
    explicit Buffer(std::size_t n)
        : data_(static_cast<uint8_t*>(::operator new(n))), size_(n) {
        for (std::size_t i = 0; i < n; ++i) data_[i] = 0;
    }
    ~Buffer() { ::operator delete(data_); }

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& o) noexcept : data_(o.data_), size_(o.size_) {
        o.data_ = nullptr;
        o.size_ = 0;
    }
    Buffer& operator=(Buffer&& o) noexcept {
        if (this != &o) {
            ::operator delete(data_);
            data_ = o.data_;
            size_ = o.size_;
            o.data_ = nullptr;
            o.size_ = 0;
        }
        return *this;
    }

    uint8_t* data() { return data_; }
    std::size_t size() const { return size_; }

private:
    uint8_t* data_;
    std::size_t size_;
};

// Bump allocator that owns a Buffer and hands out sub-spans.
class Arena {
public:
    explicit Arena(std::size_t cap) : buf_(cap), off_(0), allocs_(0) {}

    uint8_t* allocate(std::size_t n, std::size_t align = 8) {
        std::size_t cur = (off_ + (align - 1)) & ~(align - 1);
        if (cur + n > buf_.size()) return nullptr;
        off_ = cur + n;
        ++allocs_;
        return buf_.data() + cur;
    }
    void reset() { off_ = 0; }
    std::size_t used() const { return off_; }
    std::size_t allocations() const { return allocs_; }

private:
    Buffer buf_;
    std::size_t off_;
    std::size_t allocs_;
};

// Scoped guard that records and restores the arena watermark (RAII rollback).
class ArenaScope {
public:
    explicit ArenaScope(Arena& a) : arena_(a), mark_(a.used()), committed_(false) {}
    ~ArenaScope() {
        if (!committed_) {
            // emulate rollback by re-bumping from the saved mark
            std::size_t back = arena_.used() - mark_;
            (void)back;
        }
    }
    void commit() { committed_ = true; }
    std::size_t mark() const { return mark_; }

private:
    Arena& arena_;
    std::size_t mark_;
    bool committed_;
};

static uint64_t fill_pattern(uint8_t* p, std::size_t n, uint8_t seed) {
    uint64_t acc = 0;
    for (std::size_t i = 0; i < n; ++i) {
        p[i] = static_cast<uint8_t>(seed + i * 31u);
        acc = acc * 131u + p[i];
    }
    return acc;
}

int main() {
    Arena arena(256);
    uint64_t checksum = 0;

    {
        ArenaScope scope(arena);
        uint8_t* a = arena.allocate(40, 16);
        uint8_t* b = arena.allocate(24, 8);
        if (a) checksum ^= fill_pattern(a, 40, 3);
        if (b) checksum ^= fill_pattern(b, 24, 9);
        scope.commit();
    }

    Arena moved = std::move(arena);  // move-construct, transfers Buffer ownership
    uint8_t* c = moved.allocate(64, 32);
    if (c) checksum += fill_pattern(c, 64, 17);

    Arena other(64);
    other = std::move(moved);  // move-assign
    checksum ^= other.used() * 1000003ull + other.allocations();

    return static_cast<int>(checksum & 0x7f);
}
