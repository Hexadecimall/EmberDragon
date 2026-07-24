#include <stdint.h>

struct Vec2 {
    int x;
    int y;
};

class Particle {
public:
    Vec2 position;
    Vec2 velocity;
    int life;
    bool active;

    void spawn(int px, int py, int vx, int vy, int ttl) {
        position.x = px;
        position.y = py;
        velocity.x = vx;
        velocity.y = vy;
        life = ttl;
        active = true;
    }

    void advance(int gravity) {
        if (!active) return;
        velocity.y += gravity;
        position.x += velocity.x;
        position.y += velocity.y;
        life--;
        if (life <= 0) active = false;
    }
};

class ParticleField {
public:
    static const int kCapacity = 64;
    static const int kFloor = 480;

    ParticleField() : count_(0) {}

    bool emit(int px, int py, int vx, int vy, int ttl) {
        if (count_ >= kCapacity) return false;
        pool_[count_].spawn(px, py, vx, vy, ttl);
        count_++;
        return true;
    }

    void bounce(Particle *p) {
        if (p->position.y >= kFloor) {
            p->position.y = kFloor;
            p->velocity.y = -(p->velocity.y / 2);
            p->velocity.x = (p->velocity.x * 3) / 4;
        }
    }

    int update(int gravity) {
        int alive = 0;
        for (int i = 0; i < count_; i++) {
            Particle *p = &pool_[i];
            if (!p->active) continue;
            p->advance(gravity);
            bounce(p);
            if (p->active) alive++;
        }
        return alive;
    }

    void compact() {
        int dst = 0;
        for (int i = 0; i < count_; i++) {
            if (pool_[i].active) {
                pool_[dst] = pool_[i];
                dst++;
            }
        }
        count_ = dst;
    }

    int activeCount() const {
        int total = 0;
        for (int i = 0; i < count_; i++)
            if (pool_[i].active) total++;
        return total;
    }

private:
    Particle pool_[kCapacity];
    int count_;
};

int simulate_burst(ParticleField *field, int frames, int gravity) {
    int peak = 0;
    for (int f = 0; f < frames; f++) {
        int alive = field->update(gravity);
        if (alive > peak) peak = alive;
        if ((f & 7) == 0) field->compact();
    }
    return peak;
}
