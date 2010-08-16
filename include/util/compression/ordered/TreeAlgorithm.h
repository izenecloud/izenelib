#ifndef TreeAlgorithm_H_
#define TreeAlgorithm_H_

#include "BitPattern.h"
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include <numeric>

using namespace std;

namespace izenelib {
namespace util{
namespace compression {

/**
 *  \brief  base class of TreeAlgorithm
 */
class PatternAlgorithm {
public:
	virtual BitPattern& generaterPattern(map<char, unsigned int>& charFrequency,
			BitPattern& bp) =0;
	virtual ~PatternAlgorithm() {
	}
};

/**
 *  \brief alphabet tree
 */
class Tree {

public:
	/**
	 *  \brief generate bitPattern from the tree. 
	 */
	BitPattern encode();

public:
	char ch; //symbol
	int w;  //weight
	Tree* left;
	Tree* right;
	bool visited; // flag used to do encode 
public:
	Tree() {
		ch = '\0';
		w = -1;
		left = NULL;
		right = NULL;
		visited = false;
	}
	Tree(char c, int weight) {
		ch = c;
		w = weight;
		left = NULL;
		right = NULL;
		visited = false;
	}

	~Tree() {
		dispose_();
	}

private:
	void dispose_() {
		if (left)
			delete left;
		left = NULL;
		if (right)
			delete right;	
		right = NULL;
	}

};

/**
 *  \brief it uses alphabet tree to generate Bitpattern 
 */
class TreeAlgorithm : public PatternAlgorithm {
public:
	BitPattern& generaterPattern(map<char, unsigned int>& charFrequency,
			BitPattern& bp) {
		vector<char> alpha;
		getAlphabet_(charFrequency, alpha);
		Tree *tree = new Tree;
		tree = makeTree(alpha, charFrequency);
		bp = tree->encode();
		delete tree;
		return bp;
	}
	Tree* makeTree(vector<char>& alpha,  map<char, unsigned int>& counter);
	Tree* makeTree(BitPattern& bitPattern);//make tree from bitPattern

private:
	void getAlphabet_(const map<char, unsigned int>& charFrequency,
			vector<char>& alphabet);

	int minimize_ (vector<char> &alpha,  map<char, unsigned int>& counter);
	int minimize1_(vector<char> &alpha,  map<char, unsigned int>& counter);
	int minimize2_(vector<char> &alpha,  map<char, unsigned int>& counter);
};

}}}
#endif /*TreeAlgorithm_H_*/
