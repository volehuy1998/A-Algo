#pragma once
#include <bits/stdc++.h>
using namespace std;

struct machine {
	static random_device device;
	static mt19937 gen;
	static uniform_int_distribution<> dis;
	static int get() {
		return dis(gen);
	}
};
random_device machine::device;
mt19937 machine::gen(machine::device());
uniform_int_distribution<> machine::dis(1, 100);
