// 功能
// 1. 给定一个 team ，返回所有成员的共同可用时段
// 2. 给定一个 team ，预定所有成员的某个时段

// 限制：
// 1. 日历不考虑时区
// 2. 时段最小粒度为半个小时
// 3. 时段的起点和终止均需在整点或半整点处
// 4. 时段不可跨天

#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>

struct TimeRange {
    int from;
    int to;

    TimeRange() : from(0), to(0) {}
    TimeRange(int f, int t) : from(f), to(t) {}
};

class Calendar {
    static constexpr int SLOT_CAP = 48;
private:
    std::vector<bool> timeslots;

public:
    Calendar() : timeslots(SLOT_CAP, true) {}

    bool occupy(const TimeRange& tr) {
        if (tr.from < 0 || tr.to > SLOT_CAP || tr.from >= tr.to) {
            return false;
        }

        // chech if all of slots in the give time range available
        for (int i = tr.from; i < tr.to; ++i) {
            if (!timeslots[i]) {
                return false;
            }
        }

        // mark it occpuied
        for (int i = tr.from; i < tr.to; ++i) {
            timeslots[i] = false;
        }

        return true;
    }

    void release(const TimeRange& tr) {
        if (tr.from < 0 || tr.to > SLOT_CAP) {
            return;
        }

        for (int i = tr.from; i < tr.to; ++i) {
            timeslots[i] = true;
        }
    }

    std::vector<TimeRange> available() {
        std::vector<TimeRange> availableRanges;
        int start = -1;

        for (int i = 0; i < SLOT_CAP; ++i) {
            if (timeslots[i] && start == -1) {
                start = i;
            } else if (!timeslots[i] && start != -1) {
                availableRanges.emplace_back(start, i);
                start = -1;
            }
        }

        if (start != -1) {
            availableRanges.emplace_back(start, SLOT_CAP);
        }

        return availableRanges;
    }

    const std::vector<bool>& getTimeslots() const {
        return timeslots;
    }
};

class Team {
private:
    std::unordered_map<std::string, Calendar*> members;

public:
    ~Team() {
        for (auto& [_, calendar] : members) {
            delete calendar;
        }
    }

    void add(const std::string& name) {
        add(name, new Calendar());
    }

    void add(const std::string& name, Calendar* calendar) {
        auto it = members.find(name);
        if (it != members.end()) {
            delete it->second;
            it->second = calendar;
        } else {
            members[name] = calendar;
        }
    }

    void remove(const std::string& name) {
        auto it = members.find(name);
        if (it != members.end()) {
            delete it->second;
            members.erase(it);
        }
    }

    Calendar& getCalendar(const std::string& name) {
        return *members.at(name);
    }

    const std::unordered_map<std::string, Calendar*>& getAllMembers() const {
        return members;
    }
};

class SmartAssist {
public:
    std::vector<TimeRange> queryAvailable(const Team& team) {
        std::vector<bool> commonAvailable(48, true);

        for (const auto& [_, calendar] : team.getAllMembers()) {
            const auto& timeslots = calendar->getTimeslots();
            for (int i = 0; i < 48; ++i) {
                if (!timeslots[i]) {
                    commonAvailable[i] = false;
                }
            }
        }

        std::vector<TimeRange> availableRanges;
        int start = -1;

        for (int i = 0; i < 48; ++i) {
            if (commonAvailable[i] && start == -1) {
                start = i;
            } else if (!commonAvailable[i] && start != -1) {
                availableRanges.emplace_back(start, i);
                start = -1;
            }
        }

        if (start != -1) {
            availableRanges.emplace_back(start, 48);
        }

        return availableRanges;
    }

    bool book(Team& team, const TimeRange& timerange) {
        bool canBook = true;
        std::vector<std::string> bookedMembers;

        for (auto& [name, calendar] : team.getAllMembers()) {
            if (!calendar->occupy(timerange)) {
                canBook = false;
                break;
            }
            bookedMembers.push_back(name);
        }

        if (!canBook) {
            // 回滚已预订的日历
            for (const auto& name : bookedMembers) {
                team.getCalendar(name).release(timerange);
            }
        }

        return canBook;
    }
};

int main() {
    Team myTeam;
    myTeam.add("tom");
    myTeam.add("lily");
    myTeam.add("joe");

    myTeam.getCalendar("joe").occupy({24, 26});
    myTeam.getCalendar("lily").occupy({22, 28});

    SmartAssist smartAssist;
    auto totalAvailable = smartAssist.queryAvailable(myTeam);

    // happy case
    if (!totalAvailable.empty()) {
        assert(smartAssist.book(myTeam, totalAvailable[0]));
    }

    // failure case
    assert(smartAssist.book(myTeam, {20, 23}));

    return 0;
}