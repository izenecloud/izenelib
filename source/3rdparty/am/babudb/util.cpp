// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Licensed under the BSD License, see LICENSE file for details.

#include "util.h"
#include <sstream>
using namespace std;

#include "platform/path.h"

namespace babudb {

bool matchFilename(const YIELD::Path& fullpath, const string& desired_prefix, const string& desired_ext, unsigned int& lsn) {
	pair<YIELD::Path,YIELD::Path> parts = fullpath.split();

	std::istringstream tokenizer(parts.second.getHostCharsetPath());

	string name;
	if(!std::getline(tokenizer,name,'_'))
		return false;

	string lsn_str;
	if(!std::getline(tokenizer,lsn_str,'.'))
		return false;

	std::istringstream lsn_conv(lsn_str);
	if(!(lsn_conv >> lsn))
		return false;

	string ext;
	if(!std::getline(tokenizer,ext))
		return false;

	return ext == desired_ext && name == desired_prefix;
}

}
