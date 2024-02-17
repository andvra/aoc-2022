#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <set>
#include <chrono>
#include <algorithm>
#include <map>
#include <bit>
#include <bitset>
#include <functional>
#include <filesystem>

bool get_data_file_name(std::string* fn_absolute, std::string fn_relative) {
    if (fn_absolute == nullptr) {
        return false;
    }

    std::filesystem::path root_dir(R"(C:\Users\andre\source\test\aoc-2022\input\)");

    auto fn = root_dir / fn_relative;

    if (!std::filesystem::exists(fn)) {
        std::cout << "Could not read input file " << fn.string() << std::endl;
        return false;
    }

    *fn_absolute = fn.string();

    return true;
}

std::vector<std::string> read_file(std::string fn) {
    std::ifstream infile(fn);
    std::string line;
    std::vector<std::string> ret = {};

    while (std::getline(infile, line))
    {
        ret.push_back(line);
    }

    return ret;
}

std::vector<std::string> split_string(std::string s, std::string delimiter) {
    std::vector<std::string> ret = {};

    size_t offset = 0;

    while (true) {
        size_t idx = s.find(delimiter, offset);
        if (idx == std::string::npos) {
            ret.push_back(s.substr(offset));
            break;
        }
        ret.push_back(s.substr(offset, idx - offset));
        offset = idx + delimiter.size();
    }

    return ret;
}

// The last element on each row is the scalar to compare with
std::vector<double> linear_solver(std::vector<std::vector<double>> input) {
    bool done = false;
    int variables_complete = 0;
    int steps = 0;

    while (!done) {
        // 1. Swap rows if needed
        if (input[variables_complete][variables_complete] == 0) {
            bool did_switch = false;
            for (int i = variables_complete + 1; i < input.size(); i++) {
                if (input[i][variables_complete] != 0) {
                    auto tmp = input[i];
                    input[i] = input[variables_complete];
                    input[variables_complete] = tmp;
                    did_switch = true;
                    break;
                }
            }
            if (!did_switch) {
                std::cout << "Couldn't switch. Aborting" << std::endl;
                return {};
            }
        }
        // 2. Scale row
        double scale_factor = input[variables_complete][variables_complete];
        if (scale_factor != 1) {
            for (int i = variables_complete; i < input[variables_complete].size(); i++) {
                input[variables_complete][i] /= scale_factor;
            }
        }
        // 3. Multiply onto other rows
        for (int idx_row = 0; idx_row < input.size(); idx_row++) {
            if (idx_row == variables_complete) {
                continue;
            }
            double factor = input[idx_row][variables_complete];
            if (factor != 0) {
                for (int i = variables_complete; i < input[variables_complete].size(); i++) {
                    input[idx_row][i] -= factor * input[variables_complete][i];
                }
            }
        }
        steps++;
        if (++variables_complete == input.size()) {
            done = true;
        }
    }

    std::vector<double> ret(input.size(), 0);

    for (int i = 0; i < input.size(); i++) {
        ret[i] = input[i][input[i].size() - 1];
    }

    return ret;
}

struct Graph_node {
    int id;
    std::string name;
    std::vector<Graph_node*> neighbors;
};

struct Node_head {
    Graph_node* node;
    Node_head* last_head;
};

std::vector<Graph_node*> shortest_path(Graph_node* src, Graph_node* dst, int num_graph_nodes) {
    std::vector<int> dist(num_graph_nodes, 0);
    int cur_distance = 1;
    std::vector<Graph_node*> to_search(num_graph_nodes * num_graph_nodes); // NB could be optimized when needed
    int num_search_nodes = 0;
    int idx_start = 0;
    to_search[num_search_nodes++] = src;
    int idx_end_exclusive = num_search_nodes;
    std::vector<Graph_node*> ret = {};
    bool done = false;

    while (!done) {
        for (int i = idx_start; i < idx_end_exclusive; i++) {
            if (to_search[i] == dst) {
                Graph_node* el = to_search[i];
                while (el != src) {
                    ret.push_back(el);
                    for (auto n : el->neighbors) {
                        auto dist1 = dist[n->id];
                        auto dist2 = dist[el->id];
                        if ((n == src || dist1 != 0) && dist1 < dist2) {
                            el = n;
                            break;
                        }
                    }
                }
                break;
            }
            for (auto& n : to_search[i]->neighbors) {
                if (n == src) {
                    continue;
                }
                if (dist[n->id] == 0 || dist[n->id] > cur_distance) {
                    dist[n->id] = cur_distance;
                    to_search[num_search_nodes++] = n;
                }
            }
        }
        idx_start = idx_end_exclusive;
        idx_end_exclusive = num_search_nodes;
        cur_distance++;
        if (idx_end_exclusive == idx_start) {
            done = true;
        }
    }

    return ret;
}

std::vector<Graph_node*> connected_nodes(std::vector<Graph_node>& all_nodes, Graph_node* src, int num_graph_nodes) {
    std::vector<bool> visited(num_graph_nodes, false);
    std::vector<Graph_node*> to_search(num_graph_nodes * num_graph_nodes); // NB could be optimized when needed
    int num_search_nodes = 0;
    int idx_start = 0;
    to_search[num_search_nodes++] = src;
    int idx_end_exclusive = num_search_nodes;
    bool done = false;

    while (!done) {
        for (int i = idx_start; i < idx_end_exclusive; i++) {
            visited[to_search[i]->id] = true;
            for (auto& n : to_search[i]->neighbors) {
                if (!visited[n->id]) {
                    to_search[num_search_nodes++] = n;
                }
            }
        }
        idx_start = idx_end_exclusive;
        idx_end_exclusive = num_search_nodes;
        if (idx_end_exclusive == idx_start) {
            done = true;
        }
    }

    std::vector<Graph_node*> ret = {};
    for (int i = 0; i < visited.size(); i++) {
        if (visited[i]) {
            ret.push_back(&all_nodes[i]);
        }
    }

    return ret;
}

std::string replace_all(std::string s, std::string to_replace, std::string replace_with) {
    auto pos = s.find(to_replace, 0);
    while (pos != std::string::npos) {
        s.replace(pos, to_replace.size(), replace_with);
        pos = s.find(to_replace, 0);
    }

    return s;
}

long long gcd(long long a, long long b) {
    while (b != 0) {
        long long m = a % b;
        a = b;
        b = m;
    }

    return a;
}

long long lcm(long long a, long long b) {
    return b * (a / gcd(a, b));
}

long long lcm(std::vector<long long> vals) {
    long long ret = 0;

    if (vals.size() == 0) {
        return 0;
    }

    ret = vals[0];

    for (int i = 1; i < vals.size(); i++) {
        ret = lcm(ret, vals[i]);
    }

    return ret;
}

struct Task_result {
    int task_a;
    int task_b;
};

void aoc01(std::vector<std::string> lines, Task_result* result) {
    auto calc = [&lines](size_t num_top_vals) {
        int elf_id = 0;
        int num_cals = 0;
        std::vector<int> top_vals(num_top_vals, 0);
        int idx_min_top_val = 0;

        for (auto& line : lines) {
            if (line.empty()) {
                if (num_cals > top_vals[idx_min_top_val]) {
                    top_vals[idx_min_top_val] = num_cals;
                    int cur_min = std::numeric_limits<int>::max();
                    for (size_t i = 0; i < num_top_vals; i++) {
                        if (top_vals[i] < cur_min) {
                            idx_min_top_val = i;
                            cur_min = top_vals[i];
                        }
                    }
                }
                elf_id++;
                num_cals = 0;
            }
            else {
                num_cals += std::atoi(line.c_str());
            }
        }

        int sum_top_vals = 0;
        for (auto& v : top_vals) {
            sum_top_vals += v;
        }
        return sum_top_vals;
    };
    
    result->task_a = calc(1);
    result->task_b = calc(3);

}

bool aoc(int id, bool use_test_data) {
    std::map<int, std::function<void(std::vector<std::string>, Task_result*)>> fns = {
        {1, aoc01}
    };

    if (fns.count(id) == 0) {
        std::cout << "Could not find implementation for ID " << id << std::endl;
        return false;
    }

    std::string fn_relative = std::format("aoc{:02}-{}.txt", id, use_test_data ? "test" : "real");
    std::string fn_absolute = {};

    if (!get_data_file_name(&fn_absolute, fn_relative)) {
        std::cout << "Could not get data file name" << std::endl;
        return false;
    }

    auto lines = read_file(fn_absolute);
    Task_result result = {};

    fns[id](lines, &result);

    std::cout << std::format("AOC-{:02}:\n  pt1: {}\n  pt2: {}", id, result.task_a, result.task_b) << std::endl;

    return true;
}

int main() {
    int aoc_id = 1;
    bool use_test_data = false;
    auto t_start = std::chrono::high_resolution_clock::now();
    aoc(aoc_id, use_test_data);
    auto t_end = std::chrono::high_resolution_clock::now();

    auto duration = duration_cast<std::chrono::milliseconds>(t_end - t_start);
    std::cout << "Duration: " << duration.count() << "ms" << std::endl;

	return 0;

}