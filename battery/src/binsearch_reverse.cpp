#include <iostream>
#include <vector>

// Binary search on a sorted vector; returns index or -1.
static int binary_search(const std::vector<int> &a, int target) {
    int lo = 0, hi = static_cast<int>(a.size()) - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] == target) return mid;
        if (a[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

// In-place two-pointer reverse.
static void reverse_inplace(std::vector<int> &a) {
    int i = 0, j = static_cast<int>(a.size()) - 1;
    while (i < j) {
        int t = a[i]; a[i] = a[j]; a[j] = t;
        i++; j--;
    }
}

int main() {
    std::vector<int> data = {1, 3, 5, 7, 9, 11, 13, 15};
    for (int needle : {7, 1, 15, 8}) {
        int idx = binary_search(data, needle);
        std::cout << "find " << needle << " -> " << idx << "\n";
    }
    reverse_inplace(data);
    std::cout << "reversed:";
    for (int v : data) std::cout << " " << v;
    std::cout << "\n";
    return 0;
}
