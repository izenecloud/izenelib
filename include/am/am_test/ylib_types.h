#ifndef YLIB_TYPES_H_
#define YLIB_TYPES_H_

#include "lock.h"
#include "YString.h"
#include "ExtendibleHash.h"
#include "ExtendibleHashFile.h"
#include "LinearHashTable.h"
#include "LinearHashFile.h"
#include "EfficientLHT.h"
#include "BTree.h"
#include "BPTree.h"
//#include "ExtendibleHashLSB.h"

//#include "PtrTree.h"
#include "RBTrie.h"
#include "SkipList.h"
#include "SplayTrie.h"
#include "TernarySearchTree.h"
#include "Tree.h"
#include "Trie.h"

#include "ExtendibleHashFile.h"
#include "LinearHashFile.h"
#include <util/ProcMemInfo.h>

namespace izenelib {
namespace am_test {

template<> inline YString generateData<YString>(const int a, int num, bool rand) {
	char p[10];
	int b;
	if (rand)
		b = myrand()%(num+1);
	else
		b = a;
	sprintf(p, "%08d", b);
	return YString(p);
}

}
}

#endif /*YLIB_TYPES_H_*/
