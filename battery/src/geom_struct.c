/* C struct + functions taking T*: Point distance and Rectangle area. */
#include <stdio.h>
#include <math.h>

struct Point {
    double x;
    double y;
};

struct Rect {
    struct Point origin;
    double width;
    double height;
};

static double point_distance(const struct Point *a, const struct Point *b) {
    double dx = a->x - b->x;
    double dy = a->y - b->y;
    return sqrt(dx * dx + dy * dy);
}

static double rect_area(const struct Rect *r) {
    return r->width * r->height;
}

static void rect_translate(struct Rect *r, double dx, double dy) {
    r->origin.x += dx;
    r->origin.y += dy;
}

int main(void) {
    struct Point p = { 0.0, 0.0 };
    struct Point q = { 3.0, 4.0 };
    struct Rect r = { { 1.0, 1.0 }, 5.0, 2.0 };

    rect_translate(&r, 2.0, -0.5);

    printf("distance = %.2f\n", point_distance(&p, &q));
    printf("area = %.2f\n", rect_area(&r));
    printf("origin = (%.2f, %.2f)\n", r.origin.x, r.origin.y);
    return 0;
}
