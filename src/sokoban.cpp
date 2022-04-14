#include "sokoban.hpp"

#include <iostream>
#include <queue>
#include <stack>
#include <stdexcept>
#include <unordered_map>

Sokoban::Sokoban(std::vector<std::vector<std::string>> levels) {
    this->levels = levels;
    current_level = 0;
    _board = levels.at(current_level);
    locate_player();
    history.push_back(_board);
}

void Sokoban::print_board() {
    for (const std::string &row : board()) {
        std::cout << row << '\n';
    }
}

unsigned int Sokoban::level() {
    return current_level;
}

bool Sokoban::solved() {
    for (const std::string &row : board()) {
        for (const char cell : row) {
            if (cell == Cell::GOAL ||
                cell == Cell::PLAYER_ON_GOAL) {
                return false;
            }
        }
    }
    return true;
}

std::vector<std::string> Sokoban::board() {
    return _board;
}

void Sokoban::locate_player() {
    for (unsigned int y = 0; y < _board.size(); y++) {
        for (unsigned int x = 0; x < _board[y].size(); x++) {
            if (_board[y][x] == Cell::PLAYER ||
                _board[y][x] == Cell::PLAYER_ON_GOAL) {
                px = x;
                py = y;
                return;
            }
        }
    }
    throw std::invalid_argument("Player not found...");
}

void Sokoban::move_player(int dy, int dx) {
    // Set the cell state when player leaves the current cell
    _board[py][px] = (_board[py][px] == Cell::PLAYER_ON_GOAL) ? 
        Cell::GOAL : Cell::EMPTY;

    // Set the cell state when player arrives to the new cell
    _board[py+dy][px+dx] = (_board[py+dy][px+dx] == Cell::GOAL) ? 
        Cell::PLAYER_ON_GOAL : Cell::PLAYER;
    py += dy;
    px += dx;
}

void Sokoban::push_box(int dy, int dx) {
    // Set the cell state when box leaves the current cell
    _board[py+dy][px+dx] = (_board[py+dy][px+dx] == Cell::BOX_ON_GOAL) ? 
        Cell::GOAL : Cell::EMPTY;

    // Set the cell state when box arrives to the new cell
    _board[py+dy+dy][px+dx+dx] = (_board[py+dy+dy][px+dx+dx] == Cell::GOAL) ? 
        Cell::BOX_ON_GOAL : Cell::BOX;
}

/* Update move with associated board state */
void Sokoban::update(Direction direction) {
    moves.push_back(direction);
    history.push_back(_board);
}

bool Sokoban::make_move(Direction direction) {
    auto [dy, dx] = dir_offsets[direction];

    // Player moves to a goal or empty cell
    if (_board[py+dy][px+dx] == Cell::GOAL || 
        _board[py+dy][px+dx] == Cell::EMPTY) {

        move_player(dy, dx);
        update(direction);

        return true;
    }

    // Player encounters a box or box on goal
    if (_board[py+dy][px+dx] == Cell::BOX ||
        _board[py+dy][px+dx] == Cell::BOX_ON_GOAL) {
        
        // If the cell next to the box is a goal or empty cell,
        // then the player can push box to that cell
        if (_board[py+dy+dy][px+dx+dx] == Cell::EMPTY ||
            _board[py+dy+dy][px+dx+dx] == Cell::GOAL) {

            push_box(dy, dx);
            move_player(dy, dx);
            update(direction);

            return true;
        }
    }
    return false;
}

bool Sokoban::move(Direction direction) {
    undone.clear();
    return make_move(direction);
}

bool Sokoban::move(unsigned int y, unsigned int x) {

    auto origin = std::make_pair(py, px);
    auto destination = std::make_pair(y, x);

    // If the specified destination is the same, don't do anything
    if (origin == destination) {
        return false;
    }

    // If the specified destination is next to the player,
    // try to move to that destination
    for (const auto &[dir, offset] : dir_offsets) {
        auto delta = std::make_pair(origin.first + offset.first, origin.second + offset.second);
        if (delta == destination) {
            return move(dir);
        }
    }

    std::queue<std::pair<unsigned int, unsigned int>> queue;
    std::unordered_map<std::pair<unsigned int, unsigned int>, 
        std::pair<unsigned int, unsigned int>, PairHash, PairEqual> visited;
    bool destination_found = false;

    // Add origin to the queue and visited map
    queue.push(origin);
    visited[origin] = std::make_pair(0, 0);

    // Visit all paths to a destination if possible
    while (!queue.empty()) {
        // Get the first node from the queue
        auto current = queue.front();
        queue.pop();

        // Get the cardinal adjacent neighbors if they are a valid path.
        // A valid path is a path that has not yet been visited, and
        // a goal or empty cell.
        std::vector<std::pair<unsigned int, unsigned int>> neighbors;
        for (const auto &[key, value] : dir_offsets) {

            auto adj = std::make_pair(current.first + value.first, current.second + value.second);
            
            if ((visited.find(adj) == visited.end()) &&
                (_board.at(adj.first).at(adj.second) == Cell::EMPTY || 
                _board.at(adj.first).at(adj.second) == Cell::GOAL)) {
                neighbors.push_back(adj);
            }

            if (adj == destination) {
                break;
            }
        }

        // Add neighbors to visited with its associated parent, and then enqueue
        for (const auto &neighbor : neighbors) {
            visited[neighbor] = current;
            queue.push(neighbor);
        }

        // Exit the loop once we found the destination
        if (current == destination) {
            destination_found = true;
            break;
        }
    }
    
    // Exit if there is no path to the destination
    if (!destination_found) {
        return false;
    }

    // Build the valid path from the origin to the destination
    std::stack<std::pair<unsigned int, unsigned int>> paths;
    auto current = destination;

    while (!(origin == current)) {
        paths.push(current);
        current = visited[current];
    }
    
    // Execute move for every node in paths
    current = origin;

    while (!paths.empty()) {
        std::pair next = paths.top();
        std::pair<int, int> offset(next.first - current.first, next.second - current.second);

        for (const auto &[direction, value] : dir_offsets) {
            if (offset == dir_offsets.at(direction)) {
                move(direction);
            }
        }
        
        current = next;
        paths.pop();
    }

    return true;
}

bool Sokoban::undo() {
    if (moves.empty()) {
        return false;
    }

    undone.push_back(std::make_pair(moves.back(), _board));

    moves.pop_back();
    history.pop_back();

    _board = history.back();
    locate_player();

    return true;
}

bool Sokoban::redo() {
    if (undone.empty()) {
        return false;
    }

    Direction direction = undone.back().first;
    _board = undone.back().second;
    update(direction);

    undone.pop_back();

    locate_player();

    return true;
}