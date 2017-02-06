/***************************************************************
 *  @file estimate.cpp
 *  @brief
 *
 *  @author Li Jiaying
 *  @bug no known bugs
 ***************************************************************/
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cassert>
#include <algorithm>
#include "color.h"

using namespace std;

string folder;

#define colored

typedef struct Time_tag {
	float real;
	float user;
	float sys;
} Time;

typedef struct Nsample_tag {
	int randn;
	int slctn;
	friend ostream& operator << (ostream& out, Nsample_tag& ns) {
		out << "R" << ns.randn << ", S" << ns.slctn << ">";
		return out;
	}
} Nsample;

class Testcase {
	public:
		Testcase() {
			no = 0;
			totals.randn = 0;
			totals.slctn = 0;
			totalu.randn = 0;
			totalu.slctn = 0;
		}

	public:
		int no;
		string filename;

		Time selective_time;
		vector<Nsample> selective_samples;
		Nsample totals;
		int spass;
		int s_qas;

		Time unselective_time;
		vector<Nsample> unselective_samples;
		Nsample totalu;
		int upass;
		int u_qas;

		int seed;

		bool operator < (const Testcase& right) const {
			if (s_qas%3 != right.s_qas%3)
				return (s_qas+3)%3 < (right.s_qas+3)%3;
			else
				return seed < right.seed;
		}
		//Testcase* next;

		friend ostream& operator << (ostream& out, Testcase tc) {
			int sround = tc.selective_samples.size();
			int uround = tc.unselective_samples.size();
			if (tc.spass == 0 && tc.upass == 0) 
				out << DARK_GRAY << setw(3) << tc.no << "   " << setw(12) << tc.filename << "\t" 
					<< setw(2) << tc.s_qas << "\t" << setw(4) << tc.seed <<NORMAL << "\t";
			else
				out << BLUE << setw(3) << tc.no << "   " << YELLOW << setw(12) << tc.filename << "\t" 
					<< GREEN << setw(2) << tc.s_qas << "\t" << RED << setw(4) << tc.seed << NORMAL << "\t";
			if (tc.spass == 0) out << DARK_GRAY;
			out << "\t++S\t" << tc.selective_samples.size() 
				<< "\t" << tc.selective_time.real
				<< "\t" << tc.totals.randn + tc.totals.slctn + sround;
			//<< "\t" << tc.spass;
			out << NORMAL;
			if (tc.upass == 0) out << DARK_GRAY;
			out << "\t\t--S\t" << tc.unselective_samples.size() 
				<< "\t" << tc.unselective_time.real
				<< "\t" << tc.totalu.randn + uround;
			//<< "\t" << tc.upass;
			out << NORMAL;

			return out;
		}
};

//bool compare_testcase(const Testcase& left, const Testcase& right) {
//	return left > right; 
//}

vector<vector<Testcase> > testcases;

void sort_testcases() {
	for (size_t i = 0; i < testcases.size(); i++) {
		sort(testcases.at(i).begin(), testcases.at(i).end());
	}
}

static bool parse_Nsample(string s, Nsample& ns) {
	stringstream ss;
	ss << s;
	ss >> ns.randn >> ns.slctn;
	//if (ns.randn >= 999999 || ns.slctn >= 999999)
	//	cout << "Wrong parsing...\n";
	if (ss.fail()) {
		ns.randn = ns.slctn = 0;
		return false;
	}
	return true;
}

static bool parse_time(string s, Time& t) {
	stringstream ss;
	ss << s;
	string str;
	float tt;
	ss >> str >> tt;
	if (ss.fail()) return false;
	if (str.find("real") != string::npos) t.real = tt;
	else if (str == "user") t.user = tt;
	else if (str == "sys") t.sys = tt;
	return true;
}


//void parse_statistics(ifstream& fin) {
void parse_statistics(char* folder) {
	string sst = string(folder) + "/statistics";
	ifstream fin(sst.c_str());
	string tmpstr;
	int no = 1;
	while (!fin.eof()) {
		Testcase tc;
		tc.totals.randn = 0;
		tc.totals.slctn = 0;
		tc.no = no++;

		while (tc.filename.length() == 0) {
			getline(fin, tc.filename);	
			if (fin.eof()) goto exit_main;
		}

		int select;
		fin >> select >> tc.s_qas >> tc.seed;
		tc.s_qas = tc.s_qas >= 2? -1:tc.s_qas;

		while (true) {
			Nsample ns;
			getline(fin, tmpstr, '\n');
			if (tmpstr.size() == 0) continue;
			//cout << "  |--> parse samples\n";
			if (parse_Nsample(tmpstr, ns) == false) {
				if (parse_time(tmpstr, tc.selective_time) == false) {
					//cout << "  |--> parse time\n";
					continue;
				} else {
					getline(fin, tmpstr);
					//cout << "**" << tmpstr << "  |--> parse time\n";
					parse_time(tmpstr, tc.selective_time);
					getline(fin, tmpstr);
					//cout << "**" << tmpstr << "  |--> parse time\n";
					parse_time(tmpstr, tc.selective_time);
					break;
				}
			}

			tc.selective_samples.push_back(ns);
			tc.totals.randn += ns.randn;
			tc.totals.slctn += ns.slctn;
			//if (tc.totals.randn >= 999999 || tc.totals.slctn >= 999999)
			//	cout << "Wrong parsing...\n";
		}
		fin >> tmpstr >> tc.spass;
		//cout << tmpstr << " "  << tc.spass << endl;


		fin >> tmpstr;
		while (tmpstr.length() == 0) {
			getline(fin, tmpstr);	
		}
		//cout << "filename:" << tc.filename << " filename2:" << tmpstr << endl;
		assert(tmpstr ==tc.filename);
		fin >> select >> tc.u_qas >> tc.seed;
		tc.u_qas = tc.u_qas >= 2? -1:tc.u_qas;
		while (true) {
			Nsample ns;
			getline(fin, tmpstr, '\n');
			//cout << "**" << tmpstr << endl;
			if (tmpstr.size() == 0) continue;

			if (parse_Nsample(tmpstr, ns) == false) {
				//cout << "  |--> parse time " << tmpstr << "\n";
				if (parse_time(tmpstr, tc.unselective_time) == false)
					continue;
				else {
					getline(fin, tmpstr);
					//cout << "  |--> parse time " << tmpstr << "\n";
					parse_time(tmpstr, tc.unselective_time);
					getline(fin, tmpstr);
					//cout << "  |--> parse time " << tmpstr << "\n";
					parse_time(tmpstr, tc.unselective_time);
					break;
				}
			}
			tc.unselective_samples.push_back(ns);
			tc.totalu.randn += ns.randn;
			tc.totalu.slctn += ns.slctn;
		}
		fin >> tmpstr >> tc.upass;
		//cout << tmpstr << " "  << tc.upass << endl;

		bool isFound = false;
		for (size_t i = 0; i < testcases.size(); i++) {
			if (testcases.at(i).at(0).filename == tc.filename) {
				testcases.at(i).push_back(tc);
				isFound = true;
			}
		}
		if (isFound == false) {
			vector<Testcase> tcs;
			tcs.push_back(tc);
			testcases.push_back(tcs);
		}
	}

exit_main:
	fin.close();
	return;
}

int main(int argc, char** argv)
{
	//string stfilestr = string(argv[1]) + "/statistics";
	//folder = argv[1];
	//ifstream fp(stfilestr.c_str());
	parse_statistics(argv[1]);
	//sort_testcases();
	//fp.close();
	for (size_t i = 0; i < testcases.size(); i++) {
		//if (i%15==0) 
		cout << "-------------------------------------------------------------------"
			<< "-------------------------------------------------------------------\n";
		cout << CYAN << "No.       Filename\tQS\tSeed\t\tSlct\tRounds\tTime\tSamples\t\tUslt\tRounds\tTime\tSamples\n" << NORMAL;
		for (size_t j = 0; j < testcases.at(i).size(); j++) {
			cout << testcases.at(i).at(j) << endl;
		}
	}

	return 0;
}