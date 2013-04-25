#ifndef WAVELET_TRIE_HPP_
#define WAVELET_TRIE_HPP_


#include "am/succinct/wat_array/bit_array.hpp"
#include <stdint.h>
#include <cassert>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <glibmm.h>

NS_IZENELIB_AM_BEGIN

namespace succinct{

namespace wavelet_trie {

#define EPS 0xFFFFFFFFFFFFFFFFLLU

struct wordtype {
    std::vector<uint64_t> data;
    uint64_t len;

    wordtype() {
        data.clear();
        len = 0;
    }

};
typedef wordtype word_type;

struct trie_node {
    word_type trie_data;
    wat_array::BitArray bit_array;
    uint64_t bit_len;
    uint64_t total;
    trie_node *left;
    trie_node *right;
    trie_node *parent;

    trie_node() : bit_len(0), total(0), left(NULL), right(NULL), parent(NULL) {}
};
typedef trie_node *link_type;

template<class string_type>

class wavelet_trie {

public:
    /*
    	super_root_ is a virtual root;
    */
    wavelet_trie() {
        root_ = new trie_node;
        super_root_ = new trie_node;
        root_->parent = super_root_;
        super_root_->left = root_;
    }

    ~wavelet_trie() {
        clear_node_(super_root_);
        delete(super_root_);
    }


    /*
    	binarization
    	string_set is the input string set(S[0..n-1]);
    	array is the input data after binarization;
    */
    void build_binarization(const std::vector<string_type>& string_set, std::vector<word_type>& array) {

        bool alphabet_set[100000];
        memset(alphabet_set,0,sizeof(alphabet_set));
        memset(alphabet_map_,(size_t)(0),sizeof(alphabet_map_));
        size_t string_set_size;
        for (size_t i = 0; i < string_set.size(); ++i) {
            size_t length = string_set[i].length();
            for (size_t j = 0; j < length; ++j)
                if(!alphabet_set[string_set[i][j]])alphabet_set[string_set[i][j]]=1;
        }

        size_t j = 1;
        for(size_t i = 0; i < 100000; ++i)
            if(alphabet_set[i]) {
//				alphabet_map_.insert(std::pair<size_t, uint64_t>(i, j));
                alphabet_map_[i] = j;
                alphabet_vec_.push_back(i);
                ++j;
            }
        /*

        		std::set<size_t> alphabet_set;
                for (size_t i = 0; i < string_set.size(); ++i)
                    for (size_t j = 0; j < string_set[i].length(); ++j)
        				alphabet_set.insert(static_cast<size_t>(string_set[i][j]));


        		uint64_t j = 0;
        		for (std::set<size_t>::iterator p = alphabet_set.begin(); p != alphabet_set.end(); ++p) {
        			alphabet_map_.insert(std::pair<size_t, uint64_t>(*p, j));
        			alphabet_vec_.push_back(*p);
        			++j;
        		}
        std::cout<<alphabet_vec_.size()<<'\n';
        */

//        alphabet_map_.insert(std::pair<size_t, uint64_t>('\0', alphabet_vec_.size()));

        alphabet_map_['\0'] = alphabet_vec_.size() + 1;
        alphabet_vec_.push_back('\0');

        alphabet_num_ = alphabet_vec_.size();
        alphabet_bit_num_ = get_bit_len_(alphabet_num_);
        length_ = string_set.size();

        for (size_t i = 0; i < string_set.size(); ++i)
            array.push_back(binarization_(string_set[i], 1));


    }

    /*
    	build trie
    	array is input data after binarization
    */
    void build_trie(const std::vector<word_type> &array) {
        for (size_t j = 0; j < array.size(); ++j) {
            word_type x = array[j];
            uint64_t word_len = x.len;
            uint64_t same_bit = 0;
            link_type cur = root_;
            ++cur->total;
            if (NULL == cur->left && NULL == cur->right && 0 == cur->trie_data.len) {
                append_bit_(cur->trie_data, x, 0, word_len);
                cur->trie_data.len = word_len;
                continue;
            }

            for (size_t i = 0; i < word_len; ++i) {//match by bit
                uint64_t tmp_bit = get_bit_(x.data, i);
                if (0 == cur->trie_data.len || same_bit >= cur->trie_data.len) {
                    if (!tmp_bit) { //turn left
                        if (NULL == cur->left) {
                            cur->left = new trie_node;
                            cur->left->parent = cur;
                            append_bit_(cur->left->trie_data, x, i + 1, word_len);
                            cur->left->total = 1;
                            break;
                        } else {
                            cur = cur->left;
                            ++cur->total;
                            same_bit = 0;
                        }
                    } else {
                        if (NULL == cur->right) {
                            cur->right = new trie_node;
                            cur->right->parent = cur;
                            append_bit_(cur->right->trie_data, x, i + 1, word_len);
                            cur->right->total = 1;
                            break;
                        } else {
                            cur = cur->right;
                            ++cur->total;
                            same_bit = 0;
                        }
                    }
                } else if (get_bit_(cur->trie_data.data, same_bit) != tmp_bit) { //match failed,note that every node's degree is 0 or 2
                    link_type tmp = new trie_node;
                    tmp->left = cur->left;
                    if (NULL != tmp->left)tmp->left->parent = tmp;
                    tmp->right = cur->right;
                    if (NULL != tmp->right)tmp->right->parent = tmp;
                    tmp->total = cur->total;
                    if (cur->trie_data.len > same_bit + 1)
                        append_bit_(tmp->trie_data, cur->trie_data, same_bit + 1, cur->trie_data.len);

                    if (!tmp_bit) { //new left
                        cur->right = tmp;
                        tmp->parent = cur;
                        cur->left = new trie_node;
                        cur->left->parent = cur;
                        append_bit_(cur->left->trie_data, x, i + 1, word_len);
                        cur->left->total = 1;
                        cur->trie_data.len = same_bit;
                        break;
                    } else {
                        cur->left = tmp;
                        tmp->parent = cur;
                        cur->right = new trie_node;
                        cur->right->parent = cur;
                        append_bit_(cur->right->trie_data, x, i + 1, word_len);
                        cur->right->total = 1;
                        cur->trie_data.len = same_bit;
                        break;
                    }
                } else {
                    ++same_bit;
                }
            }
        }
    }

    /*
    	build wavelet tree
    	array is input data after binarization
    */
    void build_wavelet(const std::vector<word_type>& array) {

        for (size_t i = 0; i < array.size(); ++i) {
            word_type x = array[i];
            uint64_t word_len = x.len;
            link_type cur = root_;
            size_t j = cur->trie_data.len;
            while (j < word_len) {
                uint64_t tmp_bit = get_bit_(x.data, j);
                if (0 == cur->bit_len) {
                    cur->bit_array.Init(cur->total);//set the size of current node's bitvector
                }
                if (!tmp_bit) {
                    cur->bit_array.SetBit(0, cur->bit_len++);//set current node's bitvector
                    cur = cur->left;
                } else {
                    cur->bit_array.SetBit(1, cur->bit_len++);//set current node's bitvector
                    cur = cur->right;
                }
                j += cur->trie_data.len + 1;
            }
        }
    }

    /*
    	build all bitvectors
    	cur is the root_ of wavelet trie
    */
//OK
    void init_wavelet(link_type cur) {
        cur->bit_array.Build();
        if(NULL != cur->left) init_wavelet(cur->left);
        if(NULL != cur->right) init_wavelet(cur->right);
    }

    /*
    	access(pos) return S[pos]
    */
    string_type access(const uint64_t pos) {
        if (pos >= length_) return "ERROR\n";
        link_type cur = root_;
        word_type tmp_word_0, tmp_word_1;
        word_type ans;//answer after binarization
        uint64_t tmp_pos = pos;

        tmp_word_0.len = 64;
        tmp_word_0.data.push_back(0);
        tmp_word_1.len = 64;
        tmp_word_1.data.push_back(1LLU);
        do {
            uint64_t tmp_bit = cur->bit_array.Lookup(tmp_pos);
            tmp_pos = cur->bit_array.Rank(tmp_bit, tmp_pos + 1) - 1;

            if (tmp_bit) {
                append_bit_(ans, cur->trie_data, 0, cur->trie_data.len);
                append_bit_(ans, tmp_word_1, 63, 64);
                cur = cur->right;
            } else {
                append_bit_(ans, cur->trie_data, 0, cur->trie_data.len);
                append_bit_(ans, tmp_word_0, 63, 64);
                cur = cur->left;
            }
            if (!cur->bit_len) {
                append_bit_(ans, cur->trie_data, 0, cur->trie_data.len);
                break;
            }
        } while (1);

        string_type st = "";
        st = unbinarization_(ans);
        return st;
    }

    /*
    	rank(s,pos) return the number of occurrences of string s in S[0..pos-1]
    */
    uint64_t rank(const string_type s, const uint64_t pos) {
        if (pos > length_ || 0 == pos) return EPS;
        word_type x = binarization_(s, 1);//end to '\0'
        if (EPS == x.len) return 0;
        uint64_t word_len = x.len;
        link_type cur = root_;
        size_t j = 0;
        uint64_t ans = pos;
        for (size_t i = 0; i < word_len; ++i) {
            uint64_t tmp_bit = get_bit_(x.data, i);
            if (j == cur->trie_data.len) {
                if (!tmp_bit) {
                    if (NULL == cur->left) return 0;
                    else {
                        ans = cur->bit_array.Rank(0, ans);
                        if (0 == ans) break;
                        cur = cur->left;
                        j = 0;
                    }
                } else {
                    if (NULL == cur->right) return 0;
                    else {
                        ans = cur->bit_array.Rank(1, ans);
                        if (0 == ans) break;
                        cur = cur->right;
                        j = 0;
                    }
                }
            } else if (get_bit_(cur->trie_data.data, j) != tmp_bit) {
                return 0;
            } else {
                ++j;
            }
        }

        return ans;
    }

    /*
    	select(s,idx) return the position of the idx-th occurrence of s in S[0..n-1]
    */
    uint64_t select(const string_type s, const uint64_t idx) {
        if (idx > length_ || 0 == idx) return EPS;
        word_type x = binarization_(s, 1);//end to '\0'
        uint64_t word_len = x.len;
        link_type cur = root_;
        size_t j = 0;
        uint64_t ans = idx;

        for (size_t i = 0; i < word_len; ++i) {
            uint64_t tmp_bit = get_bit_(x.data, i);
            if (j == cur->trie_data.len) {
                if (!tmp_bit) {
                    if (NULL == cur->left) return EPS;
                    else {
                        cur = cur->left;
                        j = 0;
                    }
                } else {
                    if (NULL == cur->right) return EPS;
                    else {
                        cur = cur->right;
                        j = 0;
                    }
                }
            } else if (get_bit_(cur->trie_data.data, j) != tmp_bit) {
                return EPS;
            } else {
                ++j;
            }
        }


        while (cur != root_) {
            if (cur == cur->parent->left)
                ans = cur->parent->bit_array.Select(0, ans) + 1;
            else
                ans = cur->parent->bit_array.Select(1, ans) + 1;
            if (0 == ans) return EPS;
            cur = cur->parent;
        }
        return ans - 1;
    }

    /*
    	rank_prefix(s,pos) return the number of strings in S[0..pos-1] have prefix s
    */
    uint64_t rank_prefix(const string_type s, const uint64_t pos) {
        if (pos > length_ || 0 == pos) return EPS;
        word_type x = binarization_(s, 0);
        uint64_t word_len = x.len;
        link_type cur = root_;
        size_t j = 0;
        uint64_t ans = pos;
        for (size_t i = 0; i < word_len; ++i) {
            uint64_t tmp_bit = get_bit_(x.data, i);
            if (j == cur->trie_data.len) {
                if (!tmp_bit) {
                    if (NULL == cur->left) return 0;
                    else {
                        ans = cur->bit_array.Rank(0, ans);
                        cur = cur->left;
                        j = 0;
                    }
                } else {
                    if (NULL == cur->right) return 0;
                    else {
                        ans = cur->bit_array.Rank(1, ans);
                        cur = cur->right;
                        j = 0;
                    }
                }
            } else if (get_bit_(cur->trie_data.data, j) != tmp_bit) {
                return 0;
            } else {
                ++j;
            }
        }

        return ans;
    }

    /*
    	select(s,idx) return the position of the idx-th string in S[0..n-1] has prefix s
    */
    uint64_t select_prefix(const string_type s, const uint64_t idx) {
        if (idx > length_ || 0 == idx) return EPS;
        word_type x = binarization_(s, 0);
        uint64_t word_len = x.len;
        link_type cur = root_;
        size_t j = 0;
        uint64_t ans = idx;
        for (size_t i = 0; i < word_len; ++i) {
            uint64_t tmp_bit = get_bit_(x.data, i);
            if (j == cur->trie_data.len) {
                if (!tmp_bit) {
                    if (NULL == cur->left) return EPS;
                    else {
                        cur = cur->left;
                        j = 0;
                    }
                } else {
                    if (NULL == cur->right) return EPS;
                    else {
                        cur = cur->right;
                        j = 0;
                    }
                }
            } else if (get_bit_(cur->trie_data.data, j) != tmp_bit) {
                return EPS;
            } else {
                ++j;
            }
        }

        while (cur != root_) {
            if (cur == cur->parent->left)
                ans = cur->parent->bit_array.Select(0, ans) + 1;
            else
                ans = cur->parent->bit_array.Select(1, ans) + 1;
            if (0 == ans) return EPS;
            cur = cur->parent;
        }

        return ans - 1;
    }

    /*
    	build wavelet trie
    */
    void build(const std::vector<string_type> &string_set) {
//        std::cout<<"begin to build\n";
//        clock_t time1, time2, time3, time4, time5;
//        time1 = clock();
        build_binarization(string_set, alphabet_);
//        time2 = clock();
//        std::cout<<"build_binarization_ cost "<<(double)(time2 - time1) / CLOCKS_PER_SEC<<"seconds.\n";
//        std::cout<<"build binarization_ ok\n";
        build_trie(alphabet_);
//        time3 = clock();
//        std::cout<<"build_trie cost "<<(double)(time3 - time2) / CLOCKS_PER_SEC<<"seconds.\n";
//        std::cout<<"build trie ok\n";
        build_wavelet(alphabet_);
//        time4 = clock();
//        std::cout<<"build_wavelet cost "<<(double)(time4 - time3) / CLOCKS_PER_SEC<<"seconds.\n";
//        std::cout<<"build wavelet ok\n";

        init_wavelet(root_);
//        time5 = clock();
//        std::cout<<"init_wavelet cost "<<(double)(time5 - time4) / CLOCKS_PER_SEC<<"seconds.\n";

//        std::cout<<"Build cost "<<(double)(time5 - time1) / CLOCKS_PER_SEC<<"seconds.\n";
//cout<<root_->bit_array.Select(0, 0)<<'\n';
//for(size_t i = 0; i < root_->bit_len; ++i)cout<<root_->bit_array.Select(0, i + 1)<<' '; cout<<'\n';
//for(size_t i = 0; i < root_->bit_len; ++i)cout<<root_->bit_array.Rank(1, i + 1)<<' '; cout<<'\n';

        /*
        print(root_->trie_data);
        print(root_->left->trie_data);
        print(root_->right->trie_data);
        print(root_->left->left->trie_data);
        print(root_->left->right->trie_data);
        */
        /*
        for(size_t i = 0; i < root_->bit_len; ++i)cout<<root_->bit_array.Lookup(i)<<' ';cout<<'\n';
        for(size_t i = 0; i < root_->left->bit_len; ++i)cout<<root_->left->bit_array.Lookup(i)<<' ';cout<<'\n';
        for(size_t i = 0; i < root_->right->bit_len; ++i)cout<<root_->right->bit_array.Lookup(i)<<' ';cout<<'\n';
        for(size_t i = 0; i < root_->left->left->bit_len; ++i)cout<<root_->left->left->bit_array.Lookup(i)<<' ';cout<<'\n';
        for(size_t i = 0; i < root_->left->right->bit_len; ++i)cout<<root_->left->right->bit_array.Lookup(i)<<' ';cout<<'\n';
        */
    }


private:

    void clear_node_(link_type cur) {
        if (NULL == cur) return;
        clear_node_(cur->left);
        delete cur->left;
        cur->left = NULL;
        clear_node_(cur->right);
        delete cur->right;
        cur->right = NULL;
    }

    /*
    	return x's i-th bit
    	for example, x=1LLU, alphabet_bit_num_=4, then x=0001
    */

    inline uint64_t get_bit_(const uint64_t x, const size_t i) {
        return (x >> (alphabet_bit_num_ - i - 1)) & 1LLU;
    }

    inline uint64_t get_bit_(const std::vector<uint64_t> &x, const size_t i) {
        return (x[i >> 6] >> (63 - (i & 63))) & 1LLU;
    }

    /*
    	append x[p..len-1] to y,and increase y.len
    	for example, x=101,y=100,p=1,len=3, then y=10001,y.len=5
    */
    void append_bit_(word_type &y, const word_type &x, const uint64_t p, const uint64_t len) {
        if (0 == x.len) return ;
        uint64_t mask = 0x8000000000000000LLU;
        mask >>= (y.len & 63);
        for (size_t i = p; i < len; ++i) {
            if ((y.len & 63) == 0) {
                mask = 0x8000000000000000LLU;
                y.data.push_back(0);
            }
            if (get_bit_(x.data, i))
                y.data[y.len >> 6] |= mask;
            mask >>= 1;
            ++y.len;
        }
    }

    uint64_t get_bit_len_(uint64_t x) {
        size_t i = 0;
        while (x) {
            x >>= 1;
            ++i;
        }
        return i;
    }

    string_type unbinarization_(const word_type &x) {
        string_type st = "";
        uint64_t mask = 0;
        uint64_t ans = 0;
        size_t tmp = 0;

        mask = 1LLU << (alphabet_bit_num_ - 1);
        for (size_t i = 0; i < x.len; ++i) {
            if (get_bit_(x.data, i))
                ans |= mask;
            ++tmp;
            mask >>= 1;
            if (tmp == alphabet_bit_num_) {
                st += static_cast<gunichar>(alphabet_vec_[ans]);
                ans = 0;
                tmp = 0;
                mask = 1LLU << (alphabet_bit_num_ - 1);
            }
        }
        return st;
    }

    word_type binarization_(const string_type &x, const uint64_t flag) {
        word_type ans;
        uint64_t mask = 0;

        for (size_t i = 0; i < x.length(); ++i) {

//            if (alphabet_map_.find(x[i]) == alphabet_map_.end()) {
            if (!alphabet_map_[x[i]]) {
                ans.len = EPS;
                return ans;
            }

            uint64_t tmp = alphabet_map_[x[i]] - 1;
            for (size_t j = 0; j < alphabet_bit_num_; ++j) {
                if (0 == (ans.len & 63)) {
                    mask = 0x8000000000000000LLU;
                    ans.data.push_back(0);
                }

                if (get_bit_(tmp, j))
                    ans.data[ans.len >> 6] |= mask;
                mask >>= 1;
                ++ans.len;
            }
        }

        if (flag) {
            uint64_t tmp = alphabet_map_['\0'] - 1;
            for (size_t j = 0; j < alphabet_bit_num_; ++j) {
                if (0 == (ans.len & 63)) {
                    mask = 0x8000000000000000LLU;
                    ans.data.push_back(0);
                }
                if (get_bit_(tmp, j))
                    ans.data[ans.len >> 6] |= mask;
                mask >>= 1;
                ++ans.len;
            }
        }

        return ans;
    }

    void print_(word_type x) {
        if (0 == x.len) {
            std::cout<<"NULL\n";
            return ;
        }
        for (size_t i = 0; i < x.len; ++i)
            std::cout<<get_bit_(x,i);
        std::cout<<'\n';
    }


    uint64_t alphabet_num_;
    uint64_t alphabet_bit_num_;
    uint64_t length_;
    std::vector<size_t> alphabet_vec_;
//    std::map<size_t, uint64_t> alphabet_map_;
    size_t alphabet_map_[100000];
    std::vector<word_type> alphabet_;

    link_type root_, super_root_;
};

}

}

NS_IZENELIB_AM_END

#endif




