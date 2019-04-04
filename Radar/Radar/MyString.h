/*
一些常用字符串处理函数
*/
#pragma once

#include<vector>
#include<cstring>

std::vector<std::string> split(const std::string& src, const std::string& key);
std::string d2s(double d);
std::string i2s(int d);