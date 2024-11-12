#include <fstream>
#include <sstream>
#include "Goal.h"

static constexpr std::string_view NO_CATEGORY = "NOCAT";

static std::expected<std::vector<Goal>, std::string_view> LoadGoalVec(std::istream& is, size_t n)
{
    std::vector<Goal> goals;
    goals.reserve(n);

    Goal goal;
    for (size_t i = 0; i < n; ++i) {
        is >> goal;
        goals.push_back(goal);
    }

    if (is.fail() && !is.eof())
        return std::unexpected("Failed to read goal");

    return goals;
}

std::istream& operator>>(std::istream& is, Goal& g)
{
    std::getline(is, g.title);
    std::getline(is, g.description);
    std::getline(is, g.category);
    is >> g.progress >> g.goal;
    is.seekg(is.tellg() + std::streampos(1));

    return is;
}

std::ostream& operator<<(std::ostream& os, const Goal& g)
{
    return os << g.title << '\n' << g.description << '\n' << g.category << '\n' << g.progress << '\n' << g.goal << '\n';
}

std::expected<GoalInfo, std::string_view> LoadGoals(std::string_view filepath)
{
    GoalInfo info;

    std::stringstream ss;
    {
    std::ifstream file(filepath.data());
    if (!file) return std::unexpected("Failed to open file");
    ss << file.rdbuf();
    }

    std::string token;
    ss >> token;
    if (token == "CURRENT") {
        size_t total;
        ss >> total;
        if (ss.fail())
            return std::unexpected("Invalid token. Number of current goals expected.");
        ss.seekg(ss.tellg() + std::streampos(1));

        auto current = LoadGoalVec(ss, total);
        if (current.has_value())
            info.current = std::move(*current);
        else
            return std::unexpected(current.error());
    }
    else {
        return std::unexpected("Invalid token. CURRENT expected.");
    }

    ss >> token;
    if (token == "COMPLETED") {
        size_t total;
        ss >> total;
        if (ss.fail())
            return std::unexpected("Invalid token. Number of completed goals expected.");
        ss.seekg(ss.tellg() + std::streampos(1));

        auto completed = LoadGoalVec(ss, total);
        if (completed.has_value())
            info.completed = std::move(*completed);
        else
            return std::unexpected(completed.error());
    }
    else {
        return std::unexpected("Invalid token. COMPLETED expected.");
    }

    return info;
}

bool WriteGoals(GoalInfo goals, std::string_view filepath)
{
    std::ofstream file(filepath.data());
    if (!file) return false;

    std::stringstream ss;

    ss << "CURRENT " << goals.current.size() << '\n';
    for (const auto& g : goals.current)
        ss << g;

    ss << "\nCOMPLETED " << goals.completed.size() << '\n';
    for (const auto& g : goals.completed)
        ss << g;

    if (ss.fail()) return false;
    file << ss.rdbuf();
    if (file.fail()) return false;

    return true;
}
