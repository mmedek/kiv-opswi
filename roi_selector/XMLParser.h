#pragma once

#include <vector>
#include <map>

struct XMLTag {
	std::string tagName;
	std::map<std::string, std::string> attributes;
	std::vector<XMLTag*> children;
	std::string noChildContent;

	static XMLTag* parseBody(FILE* f);
};