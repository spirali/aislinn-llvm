//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Generator of XML file
///
//===----------------------------------------------------------------------===//

#include "XML.h"
#include <stdlib.h>

using namespace aislinn;

XML::XML(const std::string filename) : open_tag(false)
{
  file = fopen(filename.c_str(), "w");
  if (file == NULL) {
    perror("fopen");
    exit(1);
  }
}

XML::~XML()
{
  fclose(file);
}

void XML::child(const std::string & name)
{
	if (open_tag) {
		fprintf(file, ">");
	}
	fprintf(file, "<%s", name.c_str());
	stack.push(name);
	open_tag = true;
}

void XML::back()
{
	if (open_tag) {
		open_tag = false;
		fprintf(file, " />");
	} else {
		fprintf(file, "</%s>", stack.top().c_str());
	}
	stack.pop();
}

static void find_and_replace(std::string &s, const char c, const std::string replace)
{
	size_t i = 0;
	while ((i = s.find(c, i)) != std::string::npos)
	{
		s.replace(i, 1, replace);
		i++;
	}
}

static void sanitize_string(std::string &s)
{
	find_and_replace(s, '&', "&amp;");
	find_and_replace(s, '<', "&lt;");
	find_and_replace(s, '>', "&gt;");
}

void XML::close_tag_head()
{
    if (open_tag) {
            fprintf(file, ">");
            open_tag = false;
    }
}

void XML::set(const std::string & name, const std::string & value)
{
	std::string v = value;
	sanitize_string(v);
	/*find_and_replace(v, '\n', "\\n");
	find_and_replace(v, '\t', "\\t");
	find_and_replace(v, '\r', "\\r");*/
	find_and_replace(v, '\'', "\\'");
	_set(name, v);
}

void XML::text(const std::string &text)
{
	close_tag_head();
	std::string v = text;
	sanitize_string(v);
	fputs(v.c_str(), file);
}

void XML::text(int i)
{
	close_tag_head();
        fprintf(file, "%i", i);
}

void XML::_set(const std::string & name, const std::string & value)
{
	fprintf(file, " %s='%s'", name.c_str(), value.c_str());
}

void XML::set(const std::string & name, const bool value)
{
	if (value) {
		_set(name, "true");
	} else {
		_set(name, "false");
	}
}

void XML::set(const std::string & name, const int value)
{
	fprintf(file, " %s='%i'", name.c_str(), value);
}

void XML::set(const std::string & name, void *p)
{
	fprintf(file, " %s='%p'", name.c_str(), p);
}

void XML::set(const std::string & name, const size_t value)
{
	fprintf(file, " %s='%llu'", name.c_str(), (unsigned long long) value);
}
