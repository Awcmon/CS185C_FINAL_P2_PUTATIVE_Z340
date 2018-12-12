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

//global lol please forgive me
vector<vector<int>> CONSTRAINT_TABLE = { {},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{ 4 },{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{ 20 },{},{},{},{},{ 19 },{},{},{ 14 },{ 26 },{ 21 },{ 33 },{ 12 },{ 22 },{},{ 0 },{},{},{ 18,4 },{ 52,18,4 },{},{ 6 },{ 5 },{},{ 30 },{ 7 },{},{ 53,52,18,4 },{ 23 },{ 39,19 },{ 63,39,19 },{ 2 },{ 31 },{ 15 },{},{},{ 38 },{ 64,63,39,19 },{ 48 },{},{},{ 16 },{ 10 },{},{},{ 8 },{ 71,64,63,39,19 },{},{},{ 9 },{},{ 61,53,52,18,4 },{ 57 },{ 65,2 },{ 55,6 },{ 78 },{ 56,5 },{ 62,23 },{},{ 58,30 },{ 75,16 },{},{ 83,9 },{ 89,78 },{ 3 },{ 67,15 },{ 25 },{ 44,21 },{ 47,22 },{ 77 },{ 80,71,64,63,39,19 },{ 66,31 },{},{ 24 },{},{ 99,67,15 },{ 40 },{ 37 },{},{ 42,14 },{ 59,7 },{ 28 },{ 72,48 },{ 46,12 },{ 76,10 },{ 101,44,21 },{ 113,42,14 },{ 109,99,67,15 },{ 50 },{ 32 },{ 74 },{ 102,47,22 },{ 91,62,23 },{ 104,80,71,64,63,39,19 },{ 68 },{ 17 },{ 27 },{ 116,72,48 },{ 127,104,80,71,64,63,39,19 },{},{ 117,46,12 },{ 69 } };

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

class MarkovChain
{
public:
	unordered_map<string, vector<string>> suffixes;
	int order;

	vector<string> prefix;
	std::default_random_engine gen;

	vector<vector<int>> constraintTable;
	string generatedText;

	MarkovChain();
	MarkovChain(int _order, unsigned _seedAdditive);
	void clear();
	void init(int _order, unsigned _seedAdditive);
	void analyze(string s); //build the markov chain
	string generateNext();
	string generateNextIntelligent();
	string generate(int n);
	void shift(string suffix);
	string strVecToStr(vector<string> s);
	float constraintScoreWord(string word);

private:

};

MarkovChain::MarkovChain()
{
}

MarkovChain::MarkovChain(int _order, unsigned _seedAdditive)
{
	init(_order, _seedAdditive);
}

void MarkovChain::clear()
{
	prefix.clear();
	generatedText.clear();
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
		if (prefixStr != "")
		{
			//cout << suffix << "\n";
			suffixes[""].push_back(suffix);
		}
		
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

string MarkovChain::generateNextIntelligent()
{
	//cout << "lol";
	string prefixStr = strVecToStr(prefix);
	//cout << prefixStr << "\n";
	if (suffixes.count(prefixStr) <= 0) { return ""; }
	vector<string> choices = suffixes[prefixStr];
	if (choices.size() == 0) { return ""; }
	
	std::sort(choices.begin(), choices.end(),
		[this](const string & a, const string & b) -> bool
	{
		return constraintScoreWord(a) > constraintScoreWord(b);
	});
	
	float highestWordScore = constraintScoreWord(choices[0]);
	vector<string> filtered;
	std::copy_if(choices.begin(), choices.end(), std::back_inserter(filtered), [this, highestWordScore](string s) {return constraintScoreWord(s) >= highestWordScore - 0.1f; });
	
	std::uniform_int_distribution<int> distr(0, filtered.size() - 1);
	string next = filtered[distr(gen)];
	shift(next);
	generatedText += next;
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

float MarkovChain::constraintScoreWord(string word)
{
	string putativeText = generatedText + word;
	int constraintOffset = generatedText.size();
	int score = 0;
	for (int i = 0; i < word.size(); i++)
	{
		int ind = i + constraintOffset;
		if (ind >= CONSTRAINT_TABLE.size()) { break; } //break if constraint index exceeds bounds of constraint table
		if (CONSTRAINT_TABLE[ind].size() == 0) { score++; continue; } //skip if this symbol is unrestrained
		for (int j = 0; j < CONSTRAINT_TABLE[ind].size(); j++)
		{
			if (word[i] == putativeText[CONSTRAINT_TABLE[ind][j]])
			{
				score++;
				break;
			}
		}
	}
	return ((float)score) / ((float)word.size());
}

std::string elapsedTime(std::chrono::system_clock::time_point start)
{
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	return std::to_string(elapsed.count()) + "ms";
}

int main(int argc, char* argv[])
{
	auto start = std::chrono::system_clock::now();

	string filename = argv[1];
	int order = std::stoi(argv[2]);
	int nThreads = std::stoi(argv[3]);
	int itersPerThread = std::stoi(argv[4]);

	cout << "File: " << filename << ", order: " << order << ", nThreads: " << nThreads << ", iters per thread: " << itersPerThread << "\n";

	std::ifstream t(filename);
	std::string data((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	t.close();

	MarkovChain markov(order, 1);
	markov.analyze(data);
	
	string bestStr = "";
	int bestScore = 0;
	for (int j = 0; j < itersPerThread; j++)
	{
		string output = "";
		while(output.size() < 136)
		{
			string next = markov.generateNextIntelligent();
			if (next == "") { break; }
			output += next;
		}
		if (constraintScore(output, CONSTRAINT_TABLE) > bestScore)
		{
			bestStr = output;
			bestScore = constraintScore(output, CONSTRAINT_TABLE);
		}
		markov.clear();
		output = "";
	}


	cout << bestStr << "\nScore: " << bestScore << "\n";

	std::cout << "Execution took " << elapsedTime(start) << ".\n\n";

    return 0;
}
