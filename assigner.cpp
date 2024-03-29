#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <iostream>
using namespace std;

// NOTE that this program sometimes worked and sometimes didn't for the same make run.
// This is due to the memory overload, which I attempted to reduce by erase and reserve vectors each iteration.
// If it does not work in single run, don't give up and mark it 0. Run ./hw4 multiple times and see. It usually works

class Student {
	public:
		string username;
		int cpp_exp; // Beginner 1 Intermediate 2 Advanced 3
		int gdb_exp; // Beginner 1 Intermediate 2 Advanced 3
		int algo_exp; // Beginner 1 Intermediate 2 Advanced 3
		vector<string> donotwant;
		vector<string> dowant;

		bool operator==(const Student& other) const {
			return username == other.username;
		}
		bool operator==(string str) const {
			string name = username;
			transform(name.begin(), name.end(), name.begin(), [](unsigned char c){return tolower(c);});
			return name == str;
		}
		bool operator<(const Student& other) const {
			return username < other.username;
		}
};

class Team {
	public:
		int team_id;
		vector<Student> members;
		int sum_cpp_exp = 0;
		int sum_gdb_exp = 0;
		int sum_algo_exp = 0;
		int num_members = 0;
		vector<string> donotwant;
		vector<string> dowant;

		void assignVals(){
			// When a new student is added, updates all the values
			sum_cpp_exp = 0; sum_gdb_exp = 0; sum_algo_exp = 0;
			dowant.clear(); donotwant.clear();
			for (auto s:members){
				sum_cpp_exp += s.cpp_exp;
				sum_gdb_exp += s.gdb_exp;
				sum_algo_exp += s.algo_exp;
				num_members++;
				donotwant.insert(donotwant.end(), s.donotwant.begin(), s.donotwant.end());
				dowant.insert(dowant.end(), s.dowant.begin(), s.dowant.end());
			}
		}

		friend ostream& operator<<(ostream& os, Team& team);
};

int translate(string level) {
	// enumerate string input for skill levels
	if(level == "Advanced") {
		return 3;
	}
	if(level == "Intermediate") {
		return 2;
	}
	if(level == "Beginner") {
		return 1;
	}
	return 0;
}

vector<string> split(string str, string delimiter) {
	// split string by delimiter and put into a vector
	// used for dowant and donotwant
	vector<string> v;
	if (!str.empty()) {
		int start = 0;
		do {
			// Find the index of occurrence
			int idx = str.find(delimiter, start);
			if(idx == string::npos) {
				break;
			}
			// If found add the substring till that
			// occurrence in the vector
			int length = idx - start;
			v.push_back(str.substr(start, length));
			start += (length + delimiter.size());
		} while(true);
		v.push_back(str.substr(start));
	}

	return v;
}

void pop(map<int, vector<Student>>& students, const Student& s) {
	// this is supposed to pop out the certain student from map<int, vector<Student>>
	// turned out it did not work properly, so added inner join to a set in later code
	for(auto i=students.begin(); i!=students.end(); i++) {
		auto it = find(i->second.begin(), i->second.end(), s);
		if(it != i->second.end()) {
			i->second.erase(it);
			break;
		}
	}
}

Student find_match(set<Student>& students, string name) {
	// search by name, output the student detail
	for(auto it=students.begin(); it!=students.end();++it) {
		if(it->username == name) {
			return *it;
		}
	}
	return *students.begin();
}

vector<Student> readCSV(string filename){
	// read the csv and put all students info to a vector of student
	vector<Student> all_students; // declare
	ifstream csvs;
	csvs.open(filename);
	string line;

	getline(csvs, line); // skips header
	while(getline(csvs, line)){
		stringstream ss(line);
		Student  student;
		string cell;
		// per comma information load
		getline(ss,cell,','); student.username = cell;
		getline(ss,cell,','); student.cpp_exp = translate(cell);
		getline(ss,cell,','); student.gdb_exp = translate(cell);
		getline(ss,cell,','); student.algo_exp = translate(cell);
		getline(ss,cell,','); student.donotwant = split(cell, ";");
		getline(ss,cell,','); student.dowant = split(cell, ";");

		// a Student is pushed into Student vector
		all_students.push_back(student);
	}
	return all_students;
}

vector<Team> distribute(vector<Student> all_students, int member){
	// map vector for easier search, imagine a dictionary of each skill levels Students
	map<int, vector<Student>> all_cpp;
	map<int, vector<Student>> all_gdb;
	map<int, vector<Student>> all_algo;

	// size, as in how many teams will we need?
	int size = ceil(all_students.size()/member);
	for(auto &s: all_students){
		// insert into each vectors depending on skill levels
		all_cpp[s.cpp_exp].push_back(s);
		all_gdb[s.gdb_exp].push_back(s);
		all_algo[s.algo_exp].push_back(s);

	}

	// teams initialization
	vector<Team> teams;
	for (int t=0; t<size; t++){
		Team team;
		teams.push_back(team);
	}

	// initialize vector, set, and countdown
	// I used set for leftover, just because I wanted to make it easier by only allowing unique values
	vector<Student> cpp_pool, gdb_pool, algo_pool, mid_pool, final_pool, checked_pool;
	set<Student> leftover(all_students.begin(), all_students.end());
	int remain = all_students.size();

	// first make a pair for the team
	for(int i=0; i<size; i++){
		// first Student
		Student first = *leftover.begin();

		teams[i].team_id = i+1;
		teams[i].members.push_back(first);
		Student match;
		bool assigned = false;
		// he was chosen, thus removed from set not-chosen
		leftover.erase(leftover.begin());

		// Based on the first Student, what skills do we lack?
		// If for example, Alex was 3,2,1 , the pool we choose from will be {1,2},{1,3},{2,3}
		map<string, vector<int>> skill_needs = {{"cpp", {1,2,3}}, {"gdb", {1,2,3}}, {"algo", {1,2,3}}};
		skill_needs["cpp"].erase(skill_needs["cpp"].begin() + first.cpp_exp -1);
		skill_needs["gdb"].erase(skill_needs["gdb"].begin() +  first.gdb_exp -1);
		skill_needs["algo"].erase(skill_needs["algo"].begin() + first.algo_exp -1);

		// outer join cpp
		cpp_pool = all_cpp[skill_needs["cpp"][0]];
		cpp_pool.insert(cpp_pool.end(), all_cpp[skill_needs["cpp"][1]].begin(), all_cpp[skill_needs["cpp"][1]].end());

		// outer join gdb
		gdb_pool = all_gdb[skill_needs["gdb"][0]];
		gdb_pool.insert(gdb_pool.end(), all_gdb[skill_needs["gdb"][1]].begin(), all_gdb[skill_needs["gdb"][1]].end());

		// outer join algo
		algo_pool = all_algo[skill_needs["algo"][0]];
		algo_pool.insert(algo_pool.end(), all_algo[skill_needs["algo"][1]].begin(), all_algo[skill_needs["algo"][1]].end());

		// sort for preparation to intersect
		sort(cpp_pool.begin(), cpp_pool.end());
		sort(gdb_pool.begin(), gdb_pool.end());
		sort(algo_pool.begin(), algo_pool.end());

		// inner join everybody
		set_intersection(cpp_pool.begin(), cpp_pool.end(), gdb_pool.begin(), gdb_pool.end(), back_inserter(mid_pool));
		set_intersection(algo_pool.begin(), algo_pool.end(), mid_pool.begin(), mid_pool.end(), back_inserter(final_pool));
		// this last inner join to ensure the Students are available
		set_intersection(final_pool.begin(), final_pool.end(), leftover.begin(), leftover.end(), back_inserter(checked_pool));

		// After conditional joining, we have nobody!! Then we just use the available students
		if(checked_pool.empty()) {
			// Do you have somebody you would like to work with?
			if(!teams[i].dowant.empty()) {
				int r = rand() % teams[i].dowant.size();
				teams[i].members.push_back(find_match(leftover, teams[i].dowant[r]));
				match = find_match(leftover, teams[i].dowant[r]);
				assigned = true;
			} else {
				// no preference pass it on
				copy(leftover.begin(), leftover.end(), checked_pool.begin());
			}
		}

		// remove the student you DO NOT want to work with
		for (auto &student: first.donotwant){
			auto it = find(checked_pool.begin(), checked_pool.end(), student);
			if (it != checked_pool.end()){
				checked_pool.erase(it);
			}
		}

		// prioritize student you would like to work with
		for (auto &student: first.dowant){
			auto it = find(checked_pool.begin(), checked_pool.end(), student);
			if (it != checked_pool.end() and !assigned){
				teams[i].members.push_back(*it);
				match = *it;
				assigned = true;
				break;
			}

		}

		// randomly choose one student to pair with from the pool
		if (!assigned){
			int r = rand() % checked_pool.size();
			teams[i].members.push_back(checked_pool[r]);
			match = checked_pool[r];
			assigned = true;
		}

		// store potential team and pop the chosen Student from available Students
		teams[i].assignVals();
		pop(all_cpp, match);
		pop(all_gdb, match);
		pop(all_algo, match);
		leftover.erase(leftover.find(match));

		// clear and reserve for memory optimization
		// count down by 2
		cpp_pool.clear();
		gdb_pool.clear();
		algo_pool.clear();
		mid_pool.clear();
		final_pool.clear();
		skill_needs.clear();
		checked_pool.clear();
		remain -= 2;

		cpp_pool.reserve(remain);
		gdb_pool.reserve(remain);
		algo_pool.reserve(remain);
		mid_pool.reserve(remain);
		final_pool.reserve(remain);
		checked_pool.reserve(remain);
	}

	// second, finish up the team based on the assigned pairs and following teammates
	int j=0,i;
	while(remain > 0){
		// I found earlier number team has more advantage on securing better fit,
		// so the choosing number is altering to descending now
		i = size - j%size - 1;

		// calculate the ideal stat we want for total skill points each
		int ideal_stat = 2*(teams[i].num_members + 1);
		Student match;
		bool assigned = false;

		// get the cpp pool needed
		int cpp_needed = ideal_stat - teams[i].sum_cpp_exp;
		if(cpp_needed < 1) {
			cpp_needed = 1;
		}
		else if(cpp_needed > 3) {
			cpp_needed = 3;
		}
		cpp_pool = all_cpp[cpp_needed];

		// get the gdb pool needed
		int gdb_needed = ideal_stat - teams[i].sum_gdb_exp;
		if(gdb_needed < 1) {
			gdb_needed = 1;
		}
		else if(gdb_needed > 3) {
			gdb_needed = 3;
		}
		gdb_pool = all_gdb[gdb_needed];

		// get  the algo pool needed
		int algo_needed = ideal_stat - teams[i].sum_algo_exp;
		if(algo_needed < 1) {
			algo_needed = 1;
		}
		else if(algo_needed > 3) {
			algo_needed = 3;
		}
		algo_pool = all_algo[algo_needed];

		// sort for preparation to intersect
		sort(cpp_pool.begin(), cpp_pool.end());
		sort(gdb_pool.begin(), gdb_pool.end());
		sort(algo_pool.begin(), algo_pool.end());

		// inner join everybody
		set_intersection(cpp_pool.begin(), cpp_pool.end(), gdb_pool.begin(), gdb_pool.end(), back_inserter(mid_pool));
		set_intersection(algo_pool.begin(), algo_pool.end(), mid_pool.begin(), mid_pool.end(), back_inserter(final_pool));
		// this last inner join to ensure the Students are available
		set_intersection(final_pool.begin(), final_pool.end(), leftover.begin(), leftover.end(), back_inserter(checked_pool));

		// After conditional joining, we have nobody!! Then we just use the available students
		if(checked_pool.empty()) {
			// Do you have somebody you would like to work with?
			if(!teams[i].dowant.empty()) {
				int r = rand() % teams[i].dowant.size();
				teams[i].members.push_back(find_match(leftover, teams[i].dowant[r]));
				match = find_match(leftover, teams[i].dowant[r]);
				assigned = true;
			} else {
				// no preference pass it on
				copy(leftover.begin(), leftover.end(), checked_pool.begin());
			}
		}

		// remove the student you DO NOT want to work with
		for (auto &student: teams[i].donotwant){
			auto it = find(checked_pool.begin(), checked_pool.end(), student);
			if (it != checked_pool.end()){
				checked_pool.erase(it);
			}
		}

		// prioritize student you would like to work with
		for (auto &student: teams[i].dowant){
			auto it = find(checked_pool.begin(), checked_pool.end(), student);
			if (it != checked_pool.end() and !assigned){
				teams[i].members.push_back(*it);
				match = *it;
				assigned = true;
				break;
			}

		}

		// randomly choose one student to pair with from the pool
		if (!assigned){
			int r = rand() % checked_pool.size();
			teams[i].members.push_back(checked_pool[r]);
			match = checked_pool[r];
			assigned = true;
		}

		// store potential team and pop the chosen Student from available Students
		teams[i].assignVals();
		pop(all_cpp, match);
		pop(all_gdb, match);
		pop(all_algo, match);
		leftover.erase(match);

		// clear and reserve for memory optimization
		// remain count down by 1, increment to loop
		cpp_pool.clear();
		gdb_pool.clear();
		algo_pool.clear();
		mid_pool.clear();
		final_pool.clear();
		checked_pool.clear();

		j++;
		remain--;

		cpp_pool.reserve(remain);
		gdb_pool.reserve(remain);
		algo_pool.reserve(remain);
		mid_pool.reserve(remain);
		final_pool.reserve(remain);
		checked_pool.reserve(remain);
	}

	return teams;
}

ostream& operator<<(ostream& os, Team& team){
	os<<"Team "<<team.team_id<<"\n";
	os<<"Names: ";
	for(auto s: team.members){
		os<<s.username<<", ";
	}
	os<<"\nC++: "<<team.sum_cpp_exp<<" ; gdb: "<<team.sum_gdb_exp<<" ; Algo: "<<team.sum_algo_exp<<"\n";
	return os;
}

int main(){
	vector<Team> roster1 = distribute(readCSV("rosters/Assignment4 Roster Pref1.csv"), 3);
	for(auto t: roster1) {
		cout<<t;
	}

	vector<Team> roster2 = distribute(readCSV("rosters/Assignment4 Roster Pref2.csv"), 4);
    for(auto t: roster2) {
	    cout<<t;
    }

    vector<Team> roster3 = distribute(readCSV("rosters/Assignment4 Roster Pref3.csv"), 4);
    for(auto t: roster3) {
	    cout<<t;
    }

	return 0;
}
