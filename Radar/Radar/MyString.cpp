#include"stdafx.h"
#include"MyString.h"

std::vector<std::string> split(const std::string& src, const std::string& key){
	using namespace std;
	vector<string> v;
	size_t first = 0;
	size_t last = src.find(key);
	while (string::npos != last){
		v.push_back(src.substr(first, (last - first)));
		first = last + 1;
		last = src.find(key, first);
	}
	v.push_back(src.substr(first));
	return v;
}

std::string d2s(double d){
	char str[32];
	sprintf(str, "%.8lf", d);
	std::string s(str);
	return s;
}

std::string i2s(int d){
	char str[32];
	sprintf(str, "%d", d);
	std::string s(str);
	return s;
}