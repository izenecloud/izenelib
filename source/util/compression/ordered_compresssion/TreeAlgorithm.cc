#include "TreeAlgorithm.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace izenelib {
namespace util{
namespace compression {


BitPattern Tree::encode() {

	stack<Tree*> nodes;
	vector<char> code;
	map<char, Pattern> dict;

	/* go down to the left most */
	Tree* pt = this;
	nodes.push(pt);
	while (pt->left) {
		pt = pt->left;
		nodes.push(pt);
		code.push_back('0');
	}

	while ( !nodes.empty() ) {
		pt = nodes.top();

		if (pt->visited == true) {
			code.pop_back();
			nodes.pop();
			continue;
		} else {
			pt->visited = true;
		}

		if (pt->right == NULL && pt->left == NULL) {
			/* a leaf node, compute code */
			Pattern bincode;
			bincode.nbits = code.size();
			bincode.bits = 0x00000000;
			ub4 mask =0x80000000;
			for (unsigned int i = 0; i < code.size(); i++) {
				//cout<<i<<" : "<<code[i]<<endl;
				if (code[i] == '1') {
					bincode.bits |= (mask>>i);
				}
			}
			dict[pt->ch] = bincode;
			//	cout<<" encode "<<setbase(16)<<bincode.bits<<endl;

			nodes.pop();
			code.pop_back();
		}

		else if (pt->right) {
			/* pt has right child */
			pt = pt->right;
			nodes.push(pt);
			code.push_back('1');

			/* go down to the left most */
			while (pt->left) {
				pt = pt->left;
				nodes.push(pt);
				code.push_back('0');
			}
		}
	} // end loop on stack 
	return BitPattern(dict);

}

void TreeAlgorithm::getAlphabet_(const map<char, unsigned int>& charFrequency,
		vector<char>& alphabet) {
	map<char, unsigned int>::const_iterator iter;
	alphabet.clear();
	for (iter = charFrequency.begin(); iter != charFrequency.end(); ++iter) {
		char ch = iter->first;
		alphabet.push_back(ch);
	}
	sort(alphabet.begin(), alphabet.end() );
}

Tree* TreeAlgorithm::makeTree(vector<char>& alpha, map<char, unsigned>& counter) {

	int n = alpha.size();
	/* empty alphabet */
	if (n == 0) {
		return NULL;
	}

	/* single node */
	else if (n == 1) {
		char ch = alpha[0];
		int w = counter[ch];

		Tree* t = new Tree (ch, w);
		return t;
	}

	/* build recursively */
	else {
		int k = minimize_(alpha, counter);

		/* split alphabet into two parts */
		vector<char> left;
		vector<char> right;

		for (int i = 0; i < k; i++) {
			left.push_back(alpha[i]);
		}
		for (int i = k; i < n; i++) {
			right.push_back(alpha[i]);
		}

		Tree* left_tree = makeTree(left, counter);
		Tree* right_tree = makeTree(right, counter);
		Tree* t = new Tree ('\0', 0);
		t->left = left_tree;
		t->right = right_tree;
		return t;
	}
}

Tree* TreeAlgorithm::makeTree(BitPattern& bitPattern) {
	map<char, Pattern>::iterator it;
	map<char, Pattern> bpMap = bitPattern.getMap();

	Tree* t;
	Tree* root;
	t = new Tree;
	root = t;
	for (it = bpMap.begin(); it != bpMap.end(); it++) {
		t = root;
		char ch = it->first;
		Pattern pattern = it->second;

		//	cout<<"Pattern: "<<(int)pattern.nbits<<" "<< pattern.bits<<endl;
		size_t sz = (unsigned int)pattern.nbits;
		ub4 mask = 0x80000000;
		for (size_t i= 0; i<sz; i++) {
			if (pattern.bits & (mask>>i)) {
				if ( !t->right) {
					t->right = new Tree;
				}
				t =t->right;
			} else {
				if ( !t->left) {
					t->left = new Tree;
				}
				t = t->left;
			}
			if (i == sz-1)
				t->ch = ch;
		}
	}
	return root;

}

int TreeAlgorithm::minimize_(vector<char> &alpha, map<char, unsigned int>& counter) {

	unsigned int sum1 = 0;
	unsigned int sum2 = 0;
	size_t sz = alpha.size();

	unsigned int left = 0;
	unsigned int right = sz - 1;	

	if(sz == 2)return 1;
	
	sum1 += counter[ alpha[left] ]; left++;
	sum2 += counter[ alpha[right] ]; right--;
	while (left <= right) {
		if (sum1 < sum2) {
			sum1 += counter[ alpha[left] ];
			left++;
		} else if (sum1 > sum2) {
			sum2 += counter[ alpha[right] ];
			right--;
		} else {
			if(counter[ alpha[left-1] ] <= counter[ alpha[right+1] ])
			{
				sum1 += counter[ alpha[left] ];
				left++;
			}
			else{
				sum2 += counter[ alpha[right] ];
				right--;
			}
		}
	}
	//cout<<left<<" == "<<minimize1_(alpha, counter)<<endl;
	return left;
}

int TreeAlgorithm::minimize1_(vector<char> &alpha,
		map<char, unsigned int>& counter) {
	vector<int> weights;

	for (vector<char>::const_iterator iter = alpha.begin(); iter != alpha.end(); iter++) {
		char ch = *iter;
		weights.push_back(counter[ch]);
	}

	int minimal = 0;
	int minimal_k = 0;

	int n = alpha.size();
	for (int k = 0; k < n-1; k++) {
		vector<int>::iterator iter;

		iter = weights.begin() + k + 1;

		int sum_left = accumulate(weights.begin(), iter, 0);
		int sum_right = accumulate(iter, weights.end(), 0);

		int sum = sum_left - sum_right;
		sum = (sum < 0) ? -sum : sum;

		if (k == 0 || sum < minimal) {
			minimal = sum;
			minimal_k = k;
		}
	}
	//	cout<<minimal_k + 1<<" == "<<minimize1_(alpha, counter)<<endl;
	return minimal_k + 1;
	cout<<minimal_k + 1<<" == "<<minimize_(alpha, counter)<<endl;
}

/**
 *   gready algorithm.  
 *   
 */
int TreeAlgorithm::minimize2_(vector<char> &alpha,
		map<char, unsigned int>& counter) {

	unsigned int sum1 = 0;
	unsigned int sum2 = 0;
	size_t sz = alpha.size();

	unsigned int left = 0;
	unsigned int right = sz - 1;

	while (left <= right) {
		if (sum1 < sum2) {		
			sum1 += counter[ alpha[left] ];
			left++;
		} else if (sum1 > sum2) {			
			sum2 += counter[ alpha[right] ];
			right--;
		} else {
			//if(left >= sz-1-right)
			{
				sum1 += counter[ alpha[left] ];
				left++;
			}
			//else
			{
				sum2 += counter[ alpha[right] ];
				right--;
			}
		}
	}
	//cout<<left<<" == "<<minimize1_(alpha, counter)<<endl;
	return left;
}

/**
 *   gready algorithm.  
 *   
 *
 int TreeAlgorithm::minimize_(vector<char> &alpha, map<char, unsigned int>& counter) {

 unsigned int sum1 = 0;
 unsigned int sum2 = 0;
 size_t sz = alpha.size();

 unsigned int left = 0;
 unsigned int right = sz - 1;

 while (left <= right) {
 if (sum1 < sum2) {
 left++;
 sum1 += counter[ alpha[left] ];
 } else if (sum1 > sum2) {
 right--;
 sum2 += counter[ alpha[right] ];
 } else {	
 sum1 += counter[ alpha[left] ];			
 sum2 += counter[ alpha[right] ];
 left++;	
 right--;
 }
 }
 //cout<<left<<" == "<<minimize1_(alpha, counter)<<endl;
 return left;
 }
 */

/*
 int TreeAlgorithm::minimize_(vector<char> &alpha, map<char, unsigned int>& counter) {
 
 unsigned int minimum = 100000;
 size_t min_idx = 0;
 size_t sz = alpha.size();
 
 for(size_t i=0; i<sz; i++)
 {
 if( fabs( counter[ alpha[0] ] + counter[alpha[sz-1]] - 2*counter[ alpha[i]] )<minimum )   
 {
 min_idx = i;
 }
 }	
 //cout<<left<<" == "<<minimize1_(alpha, counter)<<endl;
 return min_idx;
 }*/

/*
 int TreeAlgorithm::minimize2_(vector<char> &alpha,
 map<char, unsigned int>& counter) {

 vector<unsigned int> sum;
 sum.resize(alpha.size() );
 sum[0] = counter[ alpha[0] ];
 for (size_t i=1; i<alpha.size(); i++) {
 sum[i] = sum[i-1]+counter[ alpha[i] ];
 }
 double minimum = 100000;
 size_t min_idx = 0;
 size_t sz = alpha.size();

 if (sz == 2)
 return 1;

 for (size_t i=0; i<sz; i++) {
 if (fabs( 2*sum[i] - sum[sz-1]) <minimum) {
 min_idx = i;
 minimum = fabs( 2*sum[i] - sum[sz-1]);
 }
 }
 cout<<min_idx+1<<" == "<<minimize1_(alpha, counter)<<endl;
 return min_idx+1;
 }*/

}}}

