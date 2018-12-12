#include <cstdio>
#include <unordered_map>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <fstream>


using std::unordered_map;
using std::vector;
using std::string;
using std::cout;

//from stackoverflow lol https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
template<typename Out>
void split(const std::string &s, char delim, Out result) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

class MarkovChain
{
public:
	unordered_map<string, vector<string>> suffixes;
	vector<string> prefix;
	int order;
	std::default_random_engine gen;

	MarkovChain();
	MarkovChain(int _order, unsigned _seedAdditive);
	void init(int _order, unsigned _seedAdditive);
	void analyze(string s); //build the markov chain
	string generateNext();
	string generate(int n);
	void shift(string suffix);
	string strVecToStr(vector<string> s);

private:

};

MarkovChain::MarkovChain()
{
}

MarkovChain::MarkovChain(int _order, unsigned _seedAdditive)
{
	init(_order, _seedAdditive);
}

void MarkovChain::init(int _order, unsigned _seedAdditive)
{
	order = _order;
	gen = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count() + _seedAdditive);

}

void MarkovChain::analyze(string s)
{
	vector<string> exploded = split(s, ' ');

	for (int i = 0; i < exploded.size(); i++)
	{
		string suffix = exploded[i];
		string prefixStr = strVecToStr(prefix);
		suffixes[prefixStr].push_back(suffix);
		shift(suffix);
	}
}

string MarkovChain::generateNext()
{
	string prefixStr = strVecToStr(prefix);
	if (suffixes.count(prefixStr) <= 0) { return ""; }
	vector<string> choices = suffixes[prefixStr];
	if (choices.size() == 0) { return ""; }
	std::uniform_int_distribution<int> distr(0, choices.size() - 1);
	string next = choices[distr(gen)];
	shift(next);
	return next;
}

string MarkovChain::generate(int n)
{
	return string();
}

void MarkovChain::shift(string suffix)
{
	prefix.push_back(suffix);
	if (prefix.size() > order)
	{
		prefix.erase(prefix.begin());
	}
}

string MarkovChain::strVecToStr(vector<string> s)
{
	string output = "";

	for (int i = 0; i < s.size(); i++)
	{
		output += s[i];
	}

	return output;
}

std::string elapsedTime(std::chrono::system_clock::time_point start)
{
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	return std::to_string(elapsed.count()) + "ms";
}

//meant for scoring entire generated text in terms of constraints fulfilled
//string must be of length 136! or at least, the length of the constraint table
int constraintScore(string s, vector<vector<int>> &constraintTable)
{
	int score = 0;
	for (int i = 0; i < s.size(); i++)
	{
		if (i >= constraintTable.size()) { break; } //break if constraint index exceeds bounds of constraint table
		if (constraintTable[i].size() == 0) { continue; } //skip if this symbol is unrestrained
		for (int j = 0; j < constraintTable[i].size(); j++)
		{
			if (s[i] == s[constraintTable[i][j]])
			{
				score++;
				break;
			}
		}
	}
	return score;
}

/*
int constraintScore(string s, vector<vector<int>> &constraintTable, int constraintOffset = 0)
{
int score = 0;
for (int i = 0; i < s.size(); i++)
{
int ind = i + constraintOffset;
if (ind >= constraintTable.size()) { break; } //break if constraint index exceeds bounds of constraint table
if (constraintTable[ind].size() == 0) { continue; } //skip if this symbol is unrestrained
for (int j = 0; j < constraintTable[ind].size(); j++)
{
if (s[i] == s[constraintTable[ind][j]])
{
score++;
break;
}
}
}
return score;
}
*/

int main(int argc, char* argv[])
{
	auto start = std::chrono::system_clock::now();

	string filename = argv[1];
	int order = std::stoi(argv[2]);
	int nThreads = std::stoi(argv[3]);
	int itersPerThread = std::stoi(argv[4]);

	std::ifstream t(filename);
	std::string data((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	t.close();

	MarkovChain markov(order, 1);
	markov.analyze(data);

	vector<vector<int>> constraintTable = { {},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{ 4 },{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{ 20 },{},{},{},{},{ 19 },{},{},{ 14 },{ 26 },{ 21 },{ 33 },{ 12 },{ 22 },{},{ 0 },{},{},{ 18,4 },{ 52,18,4 },{},{ 6 },{ 5 },{},{ 30 },{ 7 },{},{ 53,52,18,4 },{ 23 },{ 39,19 },{ 63,39,19 },{ 2 },{ 31 },{ 15 },{},{},{ 38 },{ 64,63,39,19 },{ 48 },{},{},{ 16 },{ 10 },{},{},{ 8 },{ 71,64,63,39,19 },{},{},{ 9 },{},{ 61,53,52,18,4 },{ 57 },{ 65,2 },{ 55,6 },{ 78 },{ 56,5 },{ 62,23 },{},{ 58,30 },{ 75,16 },{},{ 83,9 },{ 89,78 },{ 3 },{ 67,15 },{ 25 },{ 44,21 },{ 47,22 },{ 77 },{ 80,71,64,63,39,19 },{ 66,31 },{},{ 24 },{},{ 99,67,15 },{ 40 },{ 37 },{},{ 42,14 },{ 59,7 },{ 28 },{ 72,48 },{ 46,12 },{ 76,10 },{ 101,44,21 },{ 113,42,14 },{ 109,99,67,15 },{ 50 },{ 32 },{ 74 },{ 102,47,22 },{ 91,62,23 },{ 104,80,71,64,63,39,19 },{ 68 },{ 17 },{ 27 },{ 116,72,48 },{ 127,104,80,71,64,63,39,19 },{},{ 117,46,12 },{ 69 } };
	
	string output = "";
	for (int i = 0; i < 100; i++)
	{
		output += markov.generateNext();
	}

	cout << output << "\nScore: " << constraintScore(output, constraintTable) << "\n";

	std::cout << "Execution took " << elapsedTime(start) << ".\n\n";

    return 0;
}
