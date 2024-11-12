#ifndef GOAL_H
#define GOAL_H

#include <string>
#include <istream>
#include <ostream>
#include <expected>
#include <vector>
#include <span>

struct Goal {
    std::string title, description, category;
    double progress, goal;
};

struct GoalInfo {
    std::vector<Goal> current, completed;
};

std::istream& operator>>(std::istream& is, Goal& g);
std::ostream& operator<<(std::ostream& os, const Goal& g);

std::expected<GoalInfo, std::string_view> LoadGoals(std::string_view filepath);
bool WriteGoals(GoalInfo, std::string_view filepath);

#endif // GOAL_H
