/*
 * Fixed-point particle system stepped on an integer grid.
 *
 * Particles carry sub-cell position and velocity in 16.16 fixed point so the
 * simulation stays smooth without any floating-point math. Each step applies a
 * constant downward acceleration (gravity), integrates motion, and bounces
 * particles off the rectangular world bounds with a fractional energy loss.
 */

#include <stdint.h>

#define FP_SHIFT 16                 /* 16 fractional bits */
#define FP_ONE   (1 << FP_SHIFT)    /* 1.0 in 16.16 fixed point */
#define MAX_PARTICLES 256

/*
 * A single particle. Position and velocity are 16.16 fixed point; 'alive'
 * marks whether the slot is in use. Units of position are grid cells scaled
 * by FP_ONE, so x >> FP_SHIFT is the integer cell column.
 */
typedef struct {
    int32_t x;   /* horizontal position, fixed point */
    int32_t y;   /* vertical position, fixed point */
    int32_t vx;  /* horizontal velocity per step, fixed point */
    int32_t vy;  /* vertical velocity per step, fixed point */
    uint8_t alive;
} Particle;

/*
 * The particle world: a pool of particle slots and the integer cell bounds the
 * particles are confined to. 'gravity' is added to each particle's vy every
 * step. 'restitution' is the fraction (in 16.16) of speed retained after a
 * wall bounce, e.g. FP_ONE/2 keeps half the speed.
 */
typedef struct {
    Particle particles[MAX_PARTICLES];
    int      count;        /* number of slots currently in use */
    int32_t  bound_x;      /* exclusive right wall, in fixed point */
    int32_t  bound_y;      /* exclusive bottom wall, in fixed point */
    int32_t  gravity;      /* downward acceleration per step, fixed point */
    int32_t  restitution;  /* bounce energy retention, 16.16 fraction */
} ParticleSystem;

/*
 * Convert an integer cell value to 16.16 fixed point. O(1).
 */
static int32_t fp_from_int(int v) {
    return (int32_t)v << FP_SHIFT;
}

/*
 * Multiply two 16.16 fixed-point numbers, returning a 16.16 result.
 * Uses a 64-bit intermediate so the product does not overflow before the
 * shift. O(1).
 */
static int32_t fp_mul(int32_t a, int32_t b) {
    int64_t product = (int64_t)a * (int64_t)b;
    return (int32_t)(product >> FP_SHIFT);
}

/*
 * Initialize an empty particle system bounded by a width x height integer
 * grid, with the given per-step gravity and bounce restitution (both 16.16).
 * The pool starts with zero live particles.
 */
void psys_init(ParticleSystem *sys, int width, int height,
               int32_t gravity, int32_t restitution) {
    sys->count = 0;
    sys->bound_x = fp_from_int(width);
    sys->bound_y = fp_from_int(height);
    sys->gravity = gravity;
    sys->restitution = restitution;
}

/*
 * Spawn a particle at integer cell (cx, cy) with the given fixed-point
 * velocity. Returns the index of the new particle, or -1 if the pool is full.
 * O(1).
 */
int psys_spawn(ParticleSystem *sys, int cx, int cy, int32_t vx, int32_t vy) {
    if (sys->count >= MAX_PARTICLES) {
        return -1;  /* pool exhausted; caller should reuse or drop */
    }
    int idx = sys->count++;
    Particle *p = &sys->particles[idx];
    /* Offset by half a cell so spawned particles sit at the cell center. */
    p->x = fp_from_int(cx) + FP_ONE / 2;
    p->y = fp_from_int(cy) + FP_ONE / 2;
    p->vx = vx;
    p->vy = vy;
    p->alive = 1;
    return idx;
}

/*
 * Integrate one particle forward by a single step: apply gravity, move, and
 * reflect off the four walls, scaling the rebound velocity by restitution.
 * The particle is clamped to stay strictly inside [0, bound) on each axis so
 * it can never tunnel out of the world. O(1).
 */
static void particle_step(ParticleSystem *sys, Particle *p) {
    /* Gravity accelerates the particle downward (increasing y). */
    p->vy += sys->gravity;

    /* Semi-implicit integration: update position using the new velocity. */
    p->x += p->vx;
    p->y += p->vy;

    /* Left wall: reflect position back in and invert/dampen x velocity. */
    if (p->x < 0) {
        p->x = -p->x;
        p->vx = -fp_mul(p->vx, sys->restitution);
    }
    /* Right wall: bound is exclusive, so the last valid coordinate is bound-1. */
    else if (p->x >= sys->bound_x) {
        int32_t over = p->x - (sys->bound_x - FP_ONE);
        p->x = (sys->bound_x - FP_ONE) - over;
        p->vx = -fp_mul(p->vx, sys->restitution);
    }

    /* Top wall. */
    if (p->y < 0) {
        p->y = -p->y;
        p->vy = -fp_mul(p->vy, sys->restitution);
    }
    /* Floor: the most common bounce; same exclusive-bound handling as right. */
    else if (p->y >= sys->bound_y) {
        int32_t over = p->y - (sys->bound_y - FP_ONE);
        p->y = (sys->bound_y - FP_ONE) - over;
        p->vy = -fp_mul(p->vy, sys->restitution);
    }
}

/*
 * Advance the entire system by one step, integrating every live particle.
 * Dead slots are skipped. Returns the number of particles updated.
 * Runs in O(count).
 */
int psys_step(ParticleSystem *sys) {
    int updated = 0;
    for (int i = 0; i < sys->count; ++i) {
        Particle *p = &sys->particles[i];
        if (!p->alive) {
            continue;
        }
        particle_step(sys, p);
        updated++;
    }
    return updated;
}

/*
 * Return the integer cell column a particle currently occupies by truncating
 * its fixed-point x position. O(1).
 */
int psys_cell_x(const ParticleSystem *sys, int idx) {
    return sys->particles[idx].x >> FP_SHIFT;
}

/*
 * Return the integer cell row a particle currently occupies. O(1).
 */
int psys_cell_y(const ParticleSystem *sys, int idx) {
    return sys->particles[idx].y >> FP_SHIFT;
}
