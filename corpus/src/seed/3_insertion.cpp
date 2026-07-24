#include <cstdint>

// Insertion sort of a player leaderboard, descending by score.

struct PlayerEntry {
    int playerId;
    int score;
};

class Leaderboard {
public:
    Leaderboard(PlayerEntry *entries, int capacity)
        : entries_(entries), capacity_(capacity), size_(0) {}

    bool addPlayer(int playerId, int score) {
        if (size_ >= capacity_) {
            return false;
        }
        entries_[size_].playerId = playerId;
        entries_[size_].score = score;
        size_ = size_ + 1;
        return true;
    }

    void sortDescending() {
        for (int i = 1; i < size_; i++) {
            PlayerEntry key = entries_[i];
            int j = i - 1;
            while (j >= 0 && entries_[j].score < key.score) {
                entries_[j + 1] = entries_[j];
                j = j - 1;
            }
            entries_[j + 1] = key;
        }
    }

    int rankOfPlayer(int playerId) const {
        for (int i = 0; i < size_; i++) {
            if (entries_[i].playerId == playerId) {
                return i + 1;
            }
        }
        return -1;
    }

    int topScore() const {
        if (size_ == 0) {
            return 0;
        }
        return entries_[0].score;
    }

    int size() const { return size_; }

private:
    PlayerEntry *entries_;
    int capacity_;
    int size_;
};

int scoreGap(const Leaderboard &board, int rankA, int rankB) {
    int gap = rankA - rankB;
    if (gap < 0) {
        gap = -gap;
    }
    return gap;
}
