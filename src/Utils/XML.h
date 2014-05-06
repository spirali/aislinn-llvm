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

#ifndef AISLINN_UTILS_XML
#define AISLINN_UTILS_XML

#include <stdio.h>
#include <string>
#include <stack>

namespace aislinn {

class XML {
	public:
		XML(const std::string filename);
                ~XML();

		void child(const std::string &name);
		void back();
		void set(const std::string &name, const int i);
		void set(const std::string &name, const size_t i);
		void set(const std::string &name, void *p);
		void set(const std::string &name, const std::string &s);
		void set(const std::string &name, const char *s) {
			set(name, std::string(s));
		}
		void set(const std::string &name, const bool value);

		void text(const std::string &text);
		void text(int i);

                template<typename T> void simpleChild(const std::string &name,
                                                      const T &value) {
                  child(name);
                  text(value);
                  back();
                }

                void emptyChild(const std::string &name) {
                  child(name);
                  back();
                }

	protected:
		void _set(const std::string &name, const std::string &s);
		void close_tag_head();
                void start_text();
		FILE *file;
		std::stack<std::string> stack;
		bool open_tag;
};

}

#endif
