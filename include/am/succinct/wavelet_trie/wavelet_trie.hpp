#ifndef IZENELIB_AM_SUCCINCT_WAVELET_TRIE_HPP_
#define IZENELIB_AM_SUCCINCT_WAVELET_TRIE_HPP_

#include <types.h>
#include <am/succinct/wat_array/bit_array.hpp>
#include <am/succinct/dbitv/dbitv.hpp>
#include <util/ustring/UString.h>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <am/succinct/dbitv/dbitv.hpp>

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace wavelet_trie
{

#define selectsize 64
#define blocksize 64
#define EPS 0xFFFFFFFFFFFFFFFFLLU

struct wordtype
{
    std::vector<uint64_t> data;
    uint64_t len;

    wordtype() : len(0) {}

};
typedef wordtype word_type;

struct trie_node
{
    word_type trie_data;
    wat_array::BitArray bit_array;
    dense::DBitV bv(bool select = true);
    std::vector<uint64_t>a;
    std::vector<uint32_t>b;
    std::vector<uint32_t>c;
    std::vector<uint32_t>d;    
    uint64_t bit_len;
    uint64_t total;
    uint64_t index;
    uint64_t one_num;
    trie_node *left;
    trie_node *right;
    trie_node *parent;

    trie_node() : bit_len(0), total(0), index(EPS), one_num(0), left(NULL), right(NULL), parent(NULL) {}
};
typedef trie_node *link_type;

template<class string_type>

class wavelet_trie
{

public:
    /*
    	super_root_ is a virtual root;
    */
    wavelet_trie()
    {
        root_ = new trie_node;
        super_root_ = new trie_node;
        root_->parent = super_root_;
        super_root_->left = root_;
    }

    ~wavelet_trie()
    {
        clear_node_(super_root_);
        delete(super_root_);
    }


    /*
    	binarization
    	string_set is the input string set(S[0..n-1]);
    	array is the input data after binarization;
    */
    void build_binarization(const std::vector<string_type>& string_set, std::vector<word_type>& array)
    {

        bool alphabet_set[100000];
        memset(alphabet_set,0,sizeof(alphabet_set));
        memset(alphabet_map_,(size_t)(0),sizeof(alphabet_map_));

        for (size_t i = 0; i < string_set.size(); ++i)
        {
            size_t length = string_set[i].length();
            for (size_t j = 0; j < length; ++j)
                if(!alphabet_set[static_cast<size_t>(string_set[i][j])])
                    alphabet_set[static_cast<size_t>(string_set[i][j])] = 1;
        }

        size_t j = 1;
        for(size_t i = 0; i < 100000; ++i)
            if(alphabet_set[i])
            {
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
    void build_trie(const std::vector<word_type> &array)
    {
        for (size_t j = 0; j < array.size(); ++j)
        {
            word_type x = array[j];
            uint64_t word_len = x.len;
            uint64_t same_bit = 0;
            link_type cur = root_;
            ++cur->total;
            if (NULL == cur->left && NULL == cur->right && 0 == cur->trie_data.len)
            {
                append_bit_(cur->trie_data, x, 0, word_len);
                cur->trie_data.len = word_len;
                continue;
            }

            for (size_t i = 0; i < word_len; ++i)  //match by bit
            {
                uint64_t tmp_bit = get_bit_(x.data, i);
                if (0 == cur->trie_data.len || same_bit >= cur->trie_data.len)
                {
                    if (!tmp_bit)   //turn left
                    {
                        if (NULL == cur->left)
                        {
                            cur->left = new trie_node;
                            cur->left->parent = cur;
                            append_bit_(cur->left->trie_data, x, i + 1, word_len);
                            cur->left->total = 1;
                            break;
                        }
                        else
                        {
                            cur = cur->left;
                            ++cur->total;
                            same_bit = 0;
                        }
                    }
                    else
                    {
                        if (NULL == cur->right)
                        {
                            cur->right = new trie_node;
                            cur->right->parent = cur;
                            append_bit_(cur->right->trie_data, x, i + 1, word_len);
                            cur->right->total = 1;
                            break;
                        }
                        else
                        {
                            cur = cur->right;
                            ++cur->total;
                            same_bit = 0;
                        }
                    }
                }
                else if (get_bit_(cur->trie_data.data, same_bit) != tmp_bit)     //match failed,note that every node's degree is 0 or 2
                {
                    link_type tmp = new trie_node;
                    tmp->left = cur->left;
                    if (NULL != tmp->left)tmp->left->parent = tmp;
                    tmp->right = cur->right;
                    if (NULL != tmp->right)tmp->right->parent = tmp;
                    tmp->total = cur->total;
                    if (cur->trie_data.len > same_bit + 1)
                        append_bit_(tmp->trie_data, cur->trie_data, same_bit + 1, cur->trie_data.len);

                    if (!tmp_bit)   //new left
                    {
                        cur->right = tmp;
                        tmp->parent = cur;
                        cur->left = new trie_node;
                        cur->left->parent = cur;
                        append_bit_(cur->left->trie_data, x, i + 1, word_len);
                        cur->left->total = 1;
                        cur->trie_data.len = same_bit;
                        break;
                    }
                    else
                    {
                        cur->left = tmp;
                        tmp->parent = cur;
                        cur->right = new trie_node;
                        cur->right->parent = cur;
                        append_bit_(cur->right->trie_data, x, i + 1, word_len);
                        cur->right->total = 1;
                        cur->trie_data.len = same_bit;
                        break;
                    }
                }
                else
                {
                    ++same_bit;
                }
            }
        }
    }

    /*
    	build wavelet tree
    	array is input data after binarization
    */
    void build_wavelet(const std::vector<word_type>& array)
    {

        for (size_t i = 0; i < array.size(); ++i)
        {
            word_type x = array[i];
            uint64_t word_len = x.len;
            link_type cur = root_;
            size_t j = cur->trie_data.len;
            while (j < word_len)
            {
                uint64_t tmp_bit = get_bit_(x.data, j);
                if (0 == cur->bit_len)
                {
                    cur->bit_array.Init(cur->total);//set the size of current node's bitvector
                    cur->a.resize((cur->total + blocksize - 1) / blocksize);                    
                }
                if (!tmp_bit)
                {
                    cur->bit_array.SetBit(0, cur->bit_len++);//set current node's bitvector
                    cur = cur->left;
                }
                else
                {
                    cur->bit_array.SetBit(1, cur->bit_len++);//set current node's bitvector
                    cur->a[cur->bit_len / blocksize] |= (1LLU << (blocksize - cur->bit_len % blocksize));                  
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
    void init_wavelet(link_type cur)
    {
        cur->bit_array.Build();
//		cur->bv.build(cur->bit_array.bit_blocks_, cur->bit_array.length_);


        cur->b.push_back(0);
        cur->c.push_back(0);
        cur->d.push_back(0);        
        size_t j=0,i,k=0,l=1,m=1;
        for (i=0;i<cur->bit_len;++i){
            j += get_bit_(cur->a,i);

            if (i % blocksize == blocksize - 1) {
                cur->b.push_back(j);
                ++k;    
            if ((j == l * selectsize) && (k != cur->c[cur->c.size() - 1])) {
                l++;
                cur->c.push_back(k);
            }
            
            if ((i-j+1 == m * selectsize) && (k != cur->d[cur->d.size() - 1])) {
                m++;
                cur->d.push_back(k);
            } 
                       
            }

            if ((j == l * selectsize) && (k+1 != cur->c[cur->c.size() - 1])) {
                l++;
                cur->c.push_back(k+1);
            }
            if ((i-j+1 == m * selectsize) && (k+1 != cur->d[cur->d.size() - 1])) {
                m++;
                cur->d.push_back(k+1);
            }            
        }

        cur->one_num = j;
        
        if (NULL != cur->left && NULL != cur->left->left)
            init_wavelet(cur->left);
        if (NULL != cur->right && NULL != cur->right->right)
            init_wavelet(cur->right);
    }
    
    
uint8_t selectcount(const uint64_t blk, const uint8_t r) {
    
    uint32_t q1, q2, p;
    uint64_t p1, p2, p3;
    uint8_t tmp, tmp1, tmp2, q;
    
    p1 = blk >> 32;
    p2 = blk >> 16;
    p3 = blk >> 48;
    tmp = __builtin_popcount(p1 & 0x00000000FFFFFFFFLLU);
    if (tmp < r) {
        q1 = p2 & 0x000000000000FFFFLLU;
//        tmp1 = __builtin_popcount(q1);
        tmp1 = popcount16_[q1];
        tmp2 = tmp + tmp1;
        if (tmp2 < r) {p = blk & 0x000000000000FFFFLLU; q = r - tmp2; return 48 + select_[p][q];}
            else {p = q1; q = r - tmp; return 32 + select_[p][q];}
    } else {
        q1 = p3 & 0x000000000000FFFFLLU;
//        tmp1 = __builtin_popcount(q1);
        tmp1 = popcount16_[q1];        
        if (tmp1 < r) {p = p1 & 0x000000000000FFFFLLU; q = r - tmp1; return 16 + select_[p][q];}
            else {p = q1; q = r; return select_[p][q];}
        
    }
    return select_[p][q];

}

inline uint64_t bit_rank(const link_type cur,const uint32_t pos,const bool bit) {
    uint32_t n,k,s;
    if (pos > cur->bit_len) return 0;
    k = pos / blocksize;
//    m=pos%bsize;
/*
//if(pos/bsize>cur->c.size()||k>cur->b.size())std::cout<<pos/bsize<<' '<<k<<'\n';
    s+=cur->b[k]+cur->c[pos/bsize];
//    for(j=pos-m;j<pos;j++) s+=get_bit_(cur->a,j);
//if((cur->a[k]>>(sbsize-pos%sbsize))&one[m]>255||(cur->a[k]>>(sbsize-pos%sbsize))&one[m]<0)std::cout<<((cur->a[k]>>(sbsize-pos%sbsize))&one[m])<<'\n';
    s+=poptable[(cur->a[k]>>(sbsize-pos%sbsize))&one[m]];
*/
    n = pos % blocksize;
    s = cur->b[k];
    if (n) s += __builtin_popcountl(cur->a[k] >> (blocksize - n));

    if (bit) return s;
    else return pos-s;
}

uint64_t bit_select(const link_type cur,const uint32_t idx,const bool bit) {
    if (!idx) return 0;
    uint32_t k = idx / selectsize, low, high, mid, ans;
    
    if(bit){
        if (idx > cur->one_num) return EPS;
        low = cur->c[k];

        if (k + 1 < cur->c.size()) high = cur->c[k + 1];
        else high = cur->b.size();

        while (low < high) {        
            mid = (high + low) / 2;
            if (cur->b[mid] < idx) low = mid + 1;
            else high = mid;
        }
        --low;
        uint32_t tmp = cur->b[low];
//        ans = low * blocksize + izenelib::am::SuccinctUtils::selectBlock(cur->a[low], idx - tmp);
        ans = low * blocksize +selectcount(cur->a[low], idx - tmp);
    } else {
        if (idx > cur->bit_len - cur->one_num) return EPS;
        low = cur->d[k];

        if (k + 1 < cur->d.size()) high = cur->d[k + 1];
        else high = cur->b.size();

        while (low < high) {        
            mid = (high + low) / 2;
            if (mid * blocksize - cur->b[mid] < idx) low = mid + 1;
            else high = mid;
        }
        --low;
        uint32_t tmp = cur->b[low];
//        ans = low * blocksize + izenelib::am::SuccinctUtils::selectBlock(~cur->a[low], idx - (low * blocksize - tmp));
        ans = low * blocksize + selectcount(~cur->a[low], idx - (low * blocksize - tmp));   
    }
    return ans;
}
    
	uint64_t check(const std::vector<size_t> &s) {
		link_type cur = root_;
		uint64_t tmp_bit = 0;
		uint64_t bit = 0;
		uint64_t len = -1;
		size_t p = 0, q = 0;
        uint64_t word_len = s.size() * alphabet_bit_num_;
		
		while (cur->bit_len) {
			len += cur->trie_data.len + 1;
            p = len >> 4;
            q = len & 15;
			bit = ((alphabet_map_[s[p]] - 1) >> (alphabet_bit_num_ - 1 - q)) & 1LLU;			
			if (!bit) {
				cur = cur->left;
			} else {
				cur = cur->right;
			}

		};
		
		return cur->index;
    }
    
	uint64_t new_rank(const std::vector<size_t> &s, uint64_t &ans) {

		link_type cur = root_;
        if (ans > length_ || 0 == ans) return EPS;        
		uint64_t bit = 0;
		uint64_t len = -1;
		size_t p = 0, q = 0;
        uint64_t word_len = s.size() * alphabet_bit_num_;


		while (cur->bit_len) {
			len += cur->trie_data.len + 1;
	
//			if (len > word_len) break;
//			p = len / alphabet_bit_num_;
//			q = len % alphabet_bit_num_;
            p = len >> 4;
            q = len & 15;
			bit = ((alphabet_map_[s[p]] - 1) >> (alphabet_bit_num_ - 1 - q)) & 1LLU;			

       
//			ans = cur->bv.rank(ans, bit);

			ans = bit_rank(cur, ans, bit);
			
			if (0 == ans) return EPS;
			if (!bit) {
				cur = cur->left;
			} else {
				cur = cur->right;
			}

		};
		return cur->index;
	}    
	
	uint64_t new_select(const std::vector<size_t> &s, uint64_t &ans) {
        if (ans > length_ || 0 == ans) return EPS;
		link_type cur = root_;
//        --ans;//dbitv
		uint64_t tmp_bit = 0;
		uint64_t bit = 0;
		uint64_t len = -1;
		size_t p = 0, q = 0;
        uint64_t word_len = s.size() * alphabet_bit_num_;
        size_t index = EPS;
		
		while (cur->bit_len) {
			len += cur->trie_data.len + 1;
//			if (len > word_len) break;
            p = len >> 4;
            q = len & 15;
			bit = ((alphabet_map_[s[p]] - 1) >> (alphabet_bit_num_ - 1 - q)) & 1LLU;			

			if (!bit) {
				cur = cur->left;
			} else {
				cur = cur->right;
			}

		};
		index = cur->index;

        link_type tmp;
        while (cur != root_) {
            tmp = cur->parent;
            if (cur == tmp->left){
//                ans = tmp->bit_array.Select(0, ans) + 1;
//                ans = tmp->bv.select(ans, 0);
                ans = bit_select(tmp, ans, 0) + 1;
            }else{

//                ans = tmp->bit_array.Select(1, ans) + 1;
//                ans = tmp->bv.select(ans, 1);
                ans = bit_select(tmp, ans, 1) + 1;
            }
            if (EPS == ans) return 0;
            cur = tmp;
        }
        --ans;//not dbitv

        return index;
    }    

    /*
    	access(pos) return S[pos]
    */
    string_type access(const uint64_t pos)
    {
//        if (pos >= length_) return "ERROR\n";
        if (pos >= length_)
        {
            throw "wavelet_trie::access()";
        }
        link_type cur = root_;
        word_type tmp_word_0, tmp_word_1;
        word_type ans;//answer after binarization
        uint64_t tmp_pos = pos;

        tmp_word_0.len = 64;
        tmp_word_0.data.push_back(0);
        tmp_word_1.len = 64;
        tmp_word_1.data.push_back(1LLU);
        do
        {
            uint64_t tmp_bit = cur->bit_array.Lookup(tmp_pos);
            tmp_pos = cur->bit_array.Rank(tmp_bit, tmp_pos + 1) - 1;
//            uint64_t tmp_bit = cur->bv.lookup(tmp_pos);
//            tmp_pos = cur->bv.rank(tmp_pos + 1, tmp_bit) - 1;
            if (tmp_bit)
            {
                append_bit_(ans, cur->trie_data, 0, cur->trie_data.len);
                append_bit_(ans, tmp_word_1, 63, 64);
                cur = cur->right;
            }
            else
            {
                append_bit_(ans, cur->trie_data, 0, cur->trie_data.len);
                append_bit_(ans, tmp_word_0, 63, 64);
                cur = cur->left;
            }
            if (!cur->bit_len)
            {
                append_bit_(ans, cur->trie_data, 0, cur->trie_data.len);
                break;
            }
        }
        while (1);

        string_type st;
        st = unbinarization_(ans);
        return st;
    }

    /*
    	rank(s,pos) return the number of occurrences of string s in S[0..pos-1]
    */
    uint64_t rank(const std::vector<size_t> &s, const uint64_t pos)
    {
        uint64_t ans = pos;
        if (pos > length_ || 0 == pos) return EPS;
        if (0 == s.size()) return 0;
        uint64_t word_len = s.size() * alphabet_bit_num_;
        link_type cur = root_;
        size_t j = 0;
        uint64_t tmp_bit = 0;
        size_t p = 0;
        size_t q = 0;
        for (size_t i = 0; i < word_len; ++i)
        {
            tmp_bit = ((alphabet_map_[s[p]] - 1) >> (alphabet_bit_num_ - q -1)) & 1LLU;
            if (j == cur->trie_data.len)
            {
                if (!tmp_bit)
                {
                    if (NULL == cur->left) return 0;
                    else
                    {
                        ans = cur->bit_array.Rank(0, ans);
//						ans = cur->bv.rank0(ans);
                        if (0 == ans) return ans;
                        cur = cur->left;
                        j = 0;
                    }
                }
                else
                {
                    if (NULL == cur->right) return 0;
                    else
                    {
                        ans = cur->bit_array.Rank(1, ans);
//						ans = cur->bv.rank1(ans);
                        if (0 == ans) return ans;
                        cur = cur->right;
                        j = 0;
                    }
                }
            }
            else if (get_bit_(cur->trie_data.data, j) != tmp_bit)
            {
                return 0;
            }
            else
            {
                ++j;
            }
            ++q;
            if (q == alphabet_bit_num_)
            {
                q = 0;
                ++p;
            }
        }
        return ans;
    }

    /*
    	select(s,idx) return the position of the idx-th occurrence of s in S[0..n-1]
    */
    uint64_t select(const std::vector<size_t> &s, const uint64_t idx)
    {
        uint64_t ans = idx;
//        uint64_t ans = idx - 1;
        if (idx > length_ || 0 == idx) return EPS;
        if (0 == s.size()) return 0;
        uint64_t word_len = s.size() * alphabet_bit_num_;
        link_type cur = root_;
        size_t j = 0;
        uint64_t tmp_bit = 0;
        size_t p = 0;
        size_t q = 0;
        for (size_t i = 0; i < word_len; ++i)
        {
            tmp_bit = ((alphabet_map_[s[p]] - 1) >> (alphabet_bit_num_ - q -1)) & 1LLU;
            if (j == cur->trie_data.len)
            {
                if (!tmp_bit)
                {
                    if (NULL == cur->left) return EPS;
                    else
                    {
                        cur = cur->left;
                        j = 0;
                    }
                }
                else
                {
                    if (NULL == cur->right) return EPS;
                    else
                    {
                        cur = cur->right;
                        j = 0;
                    }
                }
            }
            else if (get_bit_(cur->trie_data.data, j) != tmp_bit)
            {
                return EPS;
            }
            else
            {
                ++j;
            }
            ++q;
            if (q == alphabet_bit_num_)
            {
                q = 0;
                ++p;
            }
        }

        while (cur != root_)
        {
            if (cur == cur->parent->left)
                ans = cur->parent->bit_array.Select(0, ans) + 1;
//                ans = cur->parent->bv.select0(ans);
            else
                ans = cur->parent->bit_array.Select(1, ans) + 1;
//                ans = cur->parent->bv.select1(ans);
            if (0 == ans) return EPS;//dbitv
            cur = cur->parent;
        }

        return ans;
    }

    /*
    	rank_prefix(s,pos) return the number of strings in S[0..pos-1] have prefix s
    */
    uint64_t rank_prefix(const std::vector<size_t> &s, const uint64_t pos)
    {
        uint64_t ans = pos;
        if (pos > length_ || 0 == pos) return EPS;
        if (0 == s.size()) return 0;
        uint64_t word_len = (s.size() - 1) * alphabet_bit_num_;
        link_type cur = root_;
        size_t j = 0;
        uint64_t tmp_bit = 0;
        size_t p = 0;
        size_t q = 0;
        for (size_t i = 0; i < word_len; ++i)
        {
            tmp_bit = ((alphabet_map_[s[p]] - 1) >> (alphabet_bit_num_ - q -1)) & 1LLU;
            if (j == cur->trie_data.len)
            {
                if (!tmp_bit)
                {
                    if (NULL == cur->left) return 0;
                    else
                    {
                        ans = cur->bit_array.Rank(0, ans);
//						ans = cur->bv.rank0(ans);
                        if (0 == ans) return ans;
                        cur = cur->left;
                        j = 0;
                    }
                }
                else
                {
                    if (NULL == cur->right) return 0;
                    else
                    {
                        ans = cur->bit_array.Rank(1, ans);
//						ans = cur->bv.rank1(ans);
                        if (0 == ans) return ans;
                        cur = cur->right;
                        j = 0;
                    }
                }
            }
            else if (get_bit_(cur->trie_data.data, j) != tmp_bit)
            {
                return 0;
            }
            else
            {
                ++j;
            }
            ++q;
            if (q == alphabet_bit_num_)
            {
                q = 0;
                ++p;
            }
        }
        return ans;
    }

    /*
    	select(s,idx) return the position of the idx-th string in S[0..n-1] has prefix s
    */
    uint64_t select_prefix(const std::vector<size_t> &s, const uint64_t idx)
    {
        uint64_t ans = idx;
//        uint64_t ans = idx - 1;
        if (idx > length_ || 0 == idx) return EPS;
        if (0 == s.size()) return 0;
        uint64_t word_len = (s.size() - 1) * alphabet_bit_num_;
        link_type cur = root_;
        size_t j = 0;
        uint64_t tmp_bit = 0;
        size_t p = 0;
        size_t q = 0;
        for (size_t i = 0; i < word_len; ++i)
        {
            tmp_bit = ((alphabet_map_[s[p]] - 1) >> (alphabet_bit_num_ - q -1)) & 1LLU;
            if (j == cur->trie_data.len)
            {
                if (!tmp_bit)
                {
                    if (NULL == cur->left) return EPS;
                    else
                    {
                        cur = cur->left;
                        j = 0;
                    }
                }
                else
                {
                    if (NULL == cur->right) return EPS;
                    else
                    {
                        cur = cur->right;
                        j = 0;
                    }
                }
            }
            else if (get_bit_(cur->trie_data.data, j) != tmp_bit)
            {
                return EPS;
            }
            else
            {
                ++j;
            }
            ++q;
            if (q == alphabet_bit_num_)
            {
                q = 0;
                ++p;
            }
        }

        while (cur != root_)
        {
            if (cur == cur->parent->left)
                ans = cur->parent->bit_array.Select(0, ans) + 1;
//                ans = cur->parent->bv.select0(ans);
            else
                ans = cur->parent->bit_array.Select(1, ans) + 1;
//                ans = cur->parent->bv.select1(ans);
            if (0 == ans) return EPS;//dbitv
            cur = cur->parent;
        }
        return ans;
    }

    /*
    	build wavelet trie
    */
    void build(const std::vector<string_type> &string_set)
    {
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

    void clear_node_(link_type cur)
    {
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

    inline uint64_t get_bit_(const uint64_t x, const size_t i)
    {
        return (x >> (alphabet_bit_num_ - i - 1)) & 1LLU;
    }

    inline uint64_t get_bit_(const std::vector<uint64_t> &x, const size_t i)
    {
        return (x[i >> 6] >> (63 - (i & 63))) & 1LLU;
    }

    /*
    	append x[p..len-1] to y,and increase y.len
    	for example, x=101,y=100,p=1,len=3, then y=10001,y.len=5
    */
    void append_bit_(word_type &y, const word_type &x, const uint64_t p, const uint64_t len)
    {
        if (0 == x.len) return ;
        uint64_t mask = 0x8000000000000000LLU;
        mask >>= (y.len & 63);
        for (size_t i = p; i < len; ++i)
        {
            if ((y.len & 63) == 0)
            {
                mask = 0x8000000000000000LLU;
                y.data.push_back(0);
            }
            if (get_bit_(x.data, i))
                y.data[y.len >> 6] |= mask;
            mask >>= 1;
            ++y.len;
        }
    }

    uint64_t get_bit_len_(uint64_t x)
    {
        size_t i = 0;
        while (x)
        {
            x >>= 1;
            ++i;
        }
        return i;
    }

    string_type unbinarization_(const word_type &x)
    {
        string_type st;
        uint64_t mask = 0;
        uint64_t ans = 0;
        size_t tmp = 0;

        mask = 1LLU << (alphabet_bit_num_ - 1);
        for (size_t i = 0; i < x.len; ++i)
        {
            if (get_bit_(x.data, i))
                ans |= mask;
            ++tmp;
            mask >>= 1;
            if (tmp == alphabet_bit_num_)
            {
                st += static_cast<izenelib::util::UCS2Char>(alphabet_vec_[ans]);
                ans = 0;
                tmp = 0;
                mask = 1LLU << (alphabet_bit_num_ - 1);
            }
        }
        if (st[st.length()-1] == static_cast<izenelib::util::UCS2Char>('\0'))
            st.erase(st.length() - 1, 1);
        return st;
    }

    word_type binarization_(const string_type &x, const uint64_t flag)
    {
        word_type ans;
        uint64_t mask = 0;
        for (size_t i = 0; i < x.length(); ++i)
        {

//            if (alphabet_map_.find(x[i]) == alphabet_map_.end()) {
            if (!alphabet_map_[x[i]])
            {
                ans.len = EPS;
                return ans;
            }

            uint64_t tmp = alphabet_map_[x[i]] - 1;
            for (size_t j = 0; j < alphabet_bit_num_; ++j)
            {
                if (0 == (ans.len & 63))
                {
                    mask = 0x8000000000000000LLU;
                    ans.data.push_back(0);
                }

                if (get_bit_(tmp, j))
                    ans.data[ans.len >> 6] |= mask;
                mask >>= 1;
                ++ans.len;
            }
        }

        if (flag)
        {
            uint64_t tmp = alphabet_map_['\0'] - 1;
            for (size_t j = 0; j < alphabet_bit_num_; ++j)
            {
                if (0 == (ans.len & 63))
                {
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

    void print(word_type x)
    {
        if (0 == x.len)
        {
            std::cout<<"NULL\n";
            return ;
        }
        for (size_t i = 0; i < x.len; ++i)
            std::cout<<get_bit_(x.data,i);
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
    uint8_t select_[65536][16];
    uint8_t popcount16_[65536];    
};

}
}

NS_IZENELIB_AM_END

#endif

