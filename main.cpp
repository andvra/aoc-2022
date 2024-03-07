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

	auto root_dir = std::filesystem::current_path()
		.parent_path()
		.parent_path()
		.parent_path()
		.append("input");

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

void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

std::string trim(std::string s) {
	ltrim(s);
	rtrim(s);

	return s;
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
	int pt1;
	int pt2;
	// Most answers are numbers. Appently, some are strings. If string is set, it overrides int.
	std::string pt1_string;
	std::string pt2_string;
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

	result->pt1 = calc(1);
	result->pt2 = calc(3);

}

void aoc02(std::vector<std::string> lines, Task_result* result) {
	struct Outcomes {
		char first;
		char second;
		int val;
	};

	std::vector<Outcomes> outcomes = {
		{'A','X',3},
		{'A','Y',6},
		{'A','Z',0},
		{'B','X',0},
		{'B','Y',3},
		{'B','Z',6},
		{'C','X',6},
		{'C','Y',0},
		{'C','Z',3},
	};

	int total_score = 0;
	for (auto& line : lines) {
		char cf = line[0];
		char cs = line[2];
		int score = cs - 'X' + 1;
		for (int i = 0; i < outcomes.size(); i++) {
			auto& outcome = outcomes[i];
			if (outcome.first == cf && outcome.second == cs) {
				score += outcome.val;
				break;
			}
		}
		total_score += score;
	}

	result->pt1 = total_score;

	std::map<char, int> lose_scores = {
		{'A', 3},
		{'B', 1},
		{'C', 2}
	};

	std::map<char, int> draw_scores = {
		{'A', 1},
		{'B', 2},
		{'C', 3}
	};

	std::map<char, int> win_scores = {
		{'A', 2},
		{'B', 3},
		{'C', 1}
	};

	std::map<char, std::map<char, int>> map_outcome = {
		{'X',lose_scores},
		{'Y',draw_scores},
		{'Z',win_scores}
	};

	total_score = 0;
	for (auto& line : lines) {
		char cf = line[0];
		char cs = line[2];
		switch (cs) {
		case 'X': total_score += 0 + lose_scores[cf]; break;
		case 'Y': total_score += 3 + draw_scores[cf]; break;
		case 'Z': total_score += 6 + win_scores[cf]; break;
		}
	}
	result->pt2 = total_score;
}

void aoc03(std::vector<std::string> lines, Task_result* result) {
	int prio_sum = 0;

	auto compartment_to_binary = [](std::string s) {
		unsigned long long val = {};
		for (char c : s) {
			if (c >= 'a') {
				c -= 'a';
				c += 1;
			}
			else {
				c -= 'A';
				c += 27;
			}
			unsigned long long x = (unsigned int)c;
			val |= (1ull << (unsigned long long)c);
		}
		return val;
		};

	for (auto& line : lines) {
		unsigned long long vals[2] = {};
		vals[0] = compartment_to_binary(line.substr(0, line.size() / 2));
		vals[1] = compartment_to_binary(line.substr(line.size() / 2));
		auto res = vals[0] & vals[1];
		auto prio = std::countr_zero(res);
		prio_sum += prio;
	}
	result->pt1 = prio_sum;

	prio_sum = 0;
	for (int idx_group = 0; idx_group < lines.size() / 3; idx_group++) {
		unsigned long long vals[3] = {};
		int group_size = 3;
		for (int i = 0; i < group_size; i++) {
			vals[i] = compartment_to_binary(lines[idx_group * group_size + i]);
		}
		auto res = vals[0] & vals[1] & vals[2];
		auto prio = std::countr_zero(res);
		prio_sum += prio;
	}
	result->pt2 = prio_sum;
}

void aoc04(std::vector<std::string> lines, Task_result* result) {
	struct Range {
		int start;
		int end_incl;
	};

	int num_contained = 0;
	int num_overlap = 0;
	for (auto& line : lines) {
		auto elve_ranges = split_string(line, ",");
		std::vector<std::string> range_string[2] = {
			split_string(elve_ranges[0], "-"),
			split_string(elve_ranges[1], "-")
		};
		std::vector<Range> ranges(2);
		ranges[0].start = std::atoi(range_string[0][0].c_str());
		ranges[0].end_incl = std::atoi(range_string[0][1].c_str());
		ranges[1].start = std::atoi(range_string[1][0].c_str());
		ranges[1].end_incl = std::atoi(range_string[1][1].c_str());
		bool is_contained = false;
		bool has_overlap = false;
		for (int i = 0; i < 2; i++) {
			auto r1 = i == 0 ? ranges[0] : ranges[1];
			auto r2 = i == 0 ? ranges[1] : ranges[0];
			if (r1.start <= r2.start && r1.end_incl >= r2.end_incl) {
				is_contained = true;
			}
			if (r1.start <= r2.end_incl && r1.end_incl >= r2.start) {
				has_overlap = true;
			}
		}
		if (is_contained) {
			num_contained++;
		}
		if (has_overlap) {
			num_overlap++;
		}
	}
	result->pt1 = num_contained;
	result->pt2 = num_overlap;
}

void aoc05(std::vector<std::string> lines, Task_result* result) {
	auto num_stacks = (lines[0].size() + 1) / 4;
	std::vector<std::vector<char>> stacks_orig(num_stacks, std::vector<char>());

	size_t num_lines = 0;

	for (auto& line : lines) {
		if (line.empty()) {
			num_lines--;
			break;
		}
		num_lines++;
	}

	for (int idx_container = num_lines - 1; idx_container >= 0; idx_container--) {
		for (int idx_stack = 0; idx_stack < num_stacks; idx_stack++) {
			auto cc = lines[idx_container][4 * idx_stack + 1];
			if (cc != ' ') {
				stacks_orig[idx_stack].push_back(cc);
			}
		}
	}

	struct Move {
		int cnt;
		int idx_from;
		int idx_to;
	};

	auto idx_move_start = num_lines + 2;

	auto num_moves = lines.size() - idx_move_start;
	std::vector<Move> moves(num_moves);

	for (int idx_move = 0; idx_move < num_moves; idx_move++) {
		auto line = lines[idx_move_start + idx_move];
		auto a = 0;
		line = replace_all(line, "move ", "");
		line = replace_all(line, "from ", "");
		line = replace_all(line, "to ", "");
		auto line_parts = split_string(line, " ");
		moves[idx_move] = { std::atoi(line_parts[0].c_str()), std::atoi(line_parts[1].c_str()) - 1, std::atoi(line_parts[2].c_str()) - 1 };
	}

	for (int idx_pt = 1; idx_pt <= 2; idx_pt++) {
		auto stacks = stacks_orig;
		for (auto& move : moves) {
			auto num_el_from = stacks[move.idx_from].size();
			for (int i = 0; i < move.cnt; i++) {
				char el = {};
				if (idx_pt == 1) {
					el = stacks[move.idx_from][num_el_from - 1 - i];
				}
				if (idx_pt == 2) {
					el = stacks[move.idx_from][num_el_from - move.cnt + i];
				}
				stacks[move.idx_to].push_back(el);
			}
			for (int i = 0; i < move.cnt; i++) {
				stacks[move.idx_from].pop_back();
			}
		}

		std::string pt = {};
		for (auto& stack : stacks) {
			pt += stack.back();
		}

		if (idx_pt == 1) {
			result->pt1_string = pt;
		}
		if (idx_pt == 2) {
			result->pt2_string = pt;
		}
	}
}

bool aoc(int id) {
	std::map<int, std::function<void(std::vector<std::string>, Task_result*)>> fns = {
		{1, aoc01},
		{2, aoc02},
		{3, aoc03},
		{4, aoc04},
		{5, aoc05}
	};

	if (fns.count(id) == 0) {
		std::cout << "Could not find implementation for ID " << id << std::endl;
		return false;
	}

	auto run_with_file = [&id, &fns](bool use_test_data) {
		std::string fn_relative = std::format("aoc{:02}-{}.txt", id, use_test_data ? "test" : "real");
		std::string fn_absolute = {};

		if (!get_data_file_name(&fn_absolute, fn_relative)) {
			std::cout << "Could not get data file name" << std::endl;
			return false;
		}

		auto lines = read_file(fn_absolute);
		Task_result result = {};

		fns[id](lines, &result);

		std::string pt1 = std::to_string(result.pt1);
		std::string pt2 = std::to_string(result.pt2);
		if (!result.pt1_string.empty()) {
			pt1 = result.pt1_string;
		}
		if (!result.pt2_string.empty()) {
			pt2 = result.pt2_string;
		}
		std::cout << std::format("AOC-{:02} ({}):\n  pt1: {}\n  pt2: {}", id, use_test_data ? "test" : "real", pt1, pt2) << std::endl;
		};

	run_with_file(true);
	run_with_file(false);

	return true;
}

int main() {
	int aoc_id = 5;
	auto t_start = std::chrono::high_resolution_clock::now();
	aoc(aoc_id);
	auto t_end = std::chrono::high_resolution_clock::now();

	auto duration = duration_cast<std::chrono::milliseconds>(t_end - t_start);
	std::cout << "Duration: " << duration.count() << "ms" << std::endl;

	return 0;

}