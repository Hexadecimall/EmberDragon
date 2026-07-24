#include <stdint.h>

#define MAX_EMPLOYEES 100

typedef struct {
    int32_t employee_id;
    int32_t hourly_rate_cents;
    int32_t hours_worked;
    int32_t overtime_hours;
    int32_t department;
} Employee;

typedef struct {
    Employee staff[MAX_EMPLOYEES];
    int32_t headcount;
} Payroll;

void payroll_init(Payroll *payroll) {
    payroll->headcount = 0;
}

int hire(Payroll *payroll, int32_t id, int32_t rate, int32_t dept) {
    if (payroll->headcount >= MAX_EMPLOYEES) {
        return -1;
    }
    Employee *e = &payroll->staff[payroll->headcount];
    e->employee_id = id;
    e->hourly_rate_cents = rate;
    e->hours_worked = 0;
    e->overtime_hours = 0;
    e->department = dept;
    payroll->headcount++;
    return 0;
}

Employee *get_employee(Payroll *payroll, int32_t id) {
    for (int i = 0; i < payroll->headcount; i++) {
        if (payroll->staff[i].employee_id == id) {
            return &payroll->staff[i];
        }
    }
    return 0;
}

int log_hours(Payroll *payroll, int32_t id, int32_t hours) {
    Employee *e = get_employee(payroll, id);
    if (e == 0 || hours < 0) {
        return -1;
    }
    int32_t total = e->hours_worked + hours;
    if (total > 40) {
        int32_t regular = 40 - e->hours_worked;
        if (regular < 0) {
            regular = 0;
        }
        int32_t extra = hours - regular;
        e->hours_worked += regular;
        e->overtime_hours += extra;
    } else {
        e->hours_worked += hours;
    }
    return 0;
}

int64_t compute_gross_pay(const Employee *e) {
    int64_t base = (int64_t)e->hours_worked * e->hourly_rate_cents;
    int64_t ot_rate = (int64_t)e->hourly_rate_cents * 3 / 2;
    int64_t overtime = (int64_t)e->overtime_hours * ot_rate;
    return base + overtime;
}

int64_t department_payroll(Payroll *payroll, int32_t dept) {
    int64_t total = 0;
    for (int i = 0; i < payroll->headcount; i++) {
        if (payroll->staff[i].department == dept) {
            total += compute_gross_pay(&payroll->staff[i]);
        }
    }
    return total;
}

int32_t highest_paid_id(Payroll *payroll) {
    int32_t best_id = -1;
    int64_t best_pay = -1;
    for (int i = 0; i < payroll->headcount; i++) {
        int64_t pay = compute_gross_pay(&payroll->staff[i]);
        if (pay > best_pay) {
            best_pay = pay;
            best_id = payroll->staff[i].employee_id;
        }
    }
    return best_id;
}
