#ifndef STRING_TOKENIZER_H
#define STRING_TOKENIZER_H

#include <string>
#include <vector>


namespace izenelib{namespace osgi{

using namespace std;

/**
 * The <code>StringTokenizer</code> class is a helper class
 * for dividing a string into several strings.
 *
 * @author magr74
 */
class StringTokenizer
{
private:

    /**
     * Creates instances of class <code>StringTokenizer</code>.
     */
    StringTokenizer();

public:

    /**
     * Divides the passed string into several strings.
     *
     * @param str
     *			The string which is divided.
     *
     * @param tokens
     *			The result vector which contains the string parts.
     *
     * @param delimiters
     *			The separator string.
     */
    static void tokenize( const string& str, vector<string>& tokens,
                          const string& delimiters = " " );
};

}}
#endif

