#ifndef IZENELIB_IR_RANDOMGENERATOR_H_
#define IZENELIB_IR_RANDOMGENERATOR_H_
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>

template <class T>
class RandomGenerator
{
public:

    static void Gen(const T& low, const T& high, T& value)
    {
        static boost::mt19937 gen;
        boost::uniform_int<> dist(low, high);
        value = dist(gen);
    }
    
    static void Gen(T& value)
    {
        T max = std::numeric_limits<T>::max();
        Gen(1, max, value);
    }
    
};

template <>
class RandomGenerator<izenelib::util::UString>
{
public:

    static void Gen(izenelib::util::UString& value)
    {
        static uint32_t max_length = 20;
        static std::string chars("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
        static boost::mt19937 gen;
        uint32_t len;
        RandomGenerator<uint32_t>::Gen(1, max_length, len);
        std::stringstream ss;
        for(uint32_t i=0;i<len;i++)
        {
            boost::uniform_int<> index_dist(0, chars.length()-1);
            std::size_t index = index_dist(gen);
            ss<<chars[index];
        }
        std::string str = ss.str();
        value = izenelib::util::UString(str, izenelib::util::UString::UTF_8);
    }
    
};

#endif
