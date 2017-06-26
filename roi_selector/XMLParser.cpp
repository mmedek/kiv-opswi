#include <iostream>
#include <vector>
#include <map>
#include "XMLParser.h"

static int fpeek(FILE* f) {
	int c = fgetc(f);
	ungetc(c, f);
	return c;
}

static bool isWhitespaceChar(int c)
{
	return (c == ' ' || c == '\r' || c == '\n' || c == '\t');
}

static void ignoreWhitespace(FILE* f) {
	int c;
	do
	{
		c = fgetc(f);
	} while (isWhitespaceChar(c));

	ungetc(c, f);
}

XMLTag* XMLTag::parseBody(FILE* f) {
	XMLTag* tag = new XMLTag;

	tag->tagName = "";
	int c;
	while (!feof(f) && (c = fgetc(f)) != '>' && !isWhitespaceChar(c))
	{
		tag->tagName += (char)c;
	}

	if (!isWhitespaceChar(c))
		ungetc(c, f);

	ignoreWhitespace(f);

	while (!feof(f) && (c = fgetc(f)) != '/')
	{
		ungetc(c, f);

		if (fpeek(f) == '>')
			break;

		ignoreWhitespace(f);

		std::string attr, val;
		while (!feof(f) && (c = fgetc(f)) != '=')
			attr += (char)c;

		ignoreWhitespace(f);

		while (!feof(f) && (c = fgetc(f)) != '"')
			;

		while (!feof(f) && (c = fgetc(f)) != '"')
			val += (char)c;

		ignoreWhitespace(f);

		tag->attributes[attr] = val;
	}

	if (!feof(f) && fpeek(f) == '>' && c != '/') {
		fgetc(f);

		tag->noChildContent = "";
		while (!feof(f))
		{
			ignoreWhitespace(f);

			while (!feof(f) && (c = fgetc(f)) != '<')
				tag->noChildContent += (char)c;

			if (!feof(f) && fpeek(f) == '/')
			{
				fgetc(f);

				std::string endtag = "";
				while (!feof(f) && (c = fgetc(f)) != '>')
					endtag += (char)c;

				if (tag->tagName != endtag)
					std::cerr << "Tag start " << tag->tagName.c_str() << " doesn't match ending tag " << endtag.c_str() << std::endl;

				break;
			}
			else
			{
				XMLTag* childtag = XMLTag::parseBody(f);

				if (childtag)
					tag->children.push_back(childtag);

				tag->noChildContent = "";
			}
		}
	}

	return tag;
}