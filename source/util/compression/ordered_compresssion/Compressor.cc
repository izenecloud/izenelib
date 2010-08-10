#include "Compressor.h"
#include <fstream>
#include <cassert>

using namespace std;

namespace izenelib {
namespace util{
namespace compression {


Compressor::Compressor() {
}

Compressor::~Compressor() {
}

void Compressor::compressString(const string& input, CompressedString& output) {

	unsigned char byte = 0x00;
	char ch;
	unsigned int bitindex = 0;
	ub4 mask = 0x00000000;
	output.bitlength = 0;
	output.bits = "";
	
	map<char, Pattern> pattenMap = bitPattern_.getMap();
	if ( pattenMap.empty() ) {
		throw CompressorException("Compressor: no bitPattern, set bitPattern first\n");
	}	
	
	size_t sz = input.size();	
	for (unsigned int k = 0; k < sz; k++) {
		ch = input[k];
		Pattern bincode = bitPattern_.getPattern(ch);

		mask |= (bincode.bits>>bitindex);
		bitindex += bincode.nbits;
		output.bitlength += bincode.nbits;		

		while (bitindex >= 8) {
			byte |= (mask>>24);
			output.bits.push_back(byte);			
			mask <<= 8;
			bitindex -= 8;
			byte = 0x00;
		}
	}
	if (bitindex > 0) {
		byte |= (mask>>24);
		output.bits.push_back(byte);
	}
}

void Compressor::decompressString(const CompressedString& input, string& output) {

	map<char, Pattern> pattenMap = bitPattern_.getMap();
	if (pattenMap.empty() ) {
		throw CompressorException("Compressor: no bitPattern, set bitPattern first\n");
	}
	
    output = "";
	Tree *root;
	root= pg_.makeTree(bitPattern_);

	Tree *pt;
	pt = root;
	unsigned nbits = input.bitlength;
    int i=0;
	
	while ( nbits>0 ) {
		
		unsigned char byte;
		unsigned char mask;
		unsigned char masked;			
		mask = 0x80;
		masked = 0x00;
		byte = input.bits[i++];

		// split bits and walk on tree 
		while (mask != 0) {
			masked = byte & mask;
			mask >>= 1;
			nbits-- ;

			if (masked == 0) {
				pt = pt -> left;
			} else {
				pt = pt -> right;
			}
			if (pt -> left == NULL && pt -> right == NULL) {
				// OK, a leaf node 
				char ch = pt -> ch;
				output.push_back(ch);
				pt = root; /* back to root */
				if (nbits == 0) {
					break;
				}
			}

		} // end of read a byte, get the next byte
	} // end of read fin, decompression finished

	delete root;

}

void Compressor::showEncode(const string& input, string& output) {
	char ch;
	output = "";
	ub4 mask = 0x80000000;

	size_t sz = input.size();
	for (unsigned int k = 0; k < sz; k++) {
		ch = input[k];
		Pattern bincode = bitPattern_.getPattern(ch);
		for (size_t i=0; i<bincode.nbits; i++) {
			if (bincode.bits & (mask>>i))
				output.push_back('1');
			else
				output.push_back('0');
		}
	}
}

void Compressor::showDecode(const string& input, string& output) {
	output = "";
	
	map<char, Pattern> pattenMap = bitPattern_.getMap();
	if (pattenMap.empty() ) {
		throw CompressorException("Compressor: no bitPattern, set bitPattern first\n");
	}
	Tree *root;
	root= pg_.makeTree(bitPattern_);
	//bitPattern_.display();
	Tree *pt = root;

	for (unsigned int i = 0; i < input.size(); i++) {
		/* split bits and walk on tree */
		if (input[i] == '0') {
			pt = pt -> left;
		} else if (input[i] == '1') {
			pt = pt -> right;
		} else {
			throw CompressorException("show decode: Unknow token\n");
		}
		if (pt -> left == NULL && pt -> right == NULL) {
			/* OK, a leaf node */
			char ch = pt->ch;
			output.push_back(ch);
			pt = root; /* back to root */
		}
	}
}

void Compressor::compressFile(const string& input, const string& output,
		bool UseDefaultPattern) {
	FILE *inf;
	FILE *outf;

	inf = fopen(input.c_str(), "r");
	if (inf == NULL) {
		CompressorException ex("cannot open input file\n");
		throw ex;
	}
	outf = fopen(output.c_str(), "w");
	if (outf == NULL) {
		CompressorException ex("cannot open output file\n");
		throw ex;
	}
	if ( !UseDefaultPattern) {
		PatternGenerator pg(input);
		bitPattern_ = pg.getBitPattern();
	}
	//bitPattern_.display();
	map<char, Pattern> pattenMap = bitPattern_.getMap();
	if (pattenMap.empty() ) {
		throw CompressorException("Compressor: no bitPattern, set bitPattern first\n");
	}
	bitPattern_.toFile(outf);

	// get file length in bytes and save it in compression file 
	int fd = fileno(inf);
	struct stat st;
	fstat(fd, &st);
	long filesize = st.st_size;
	fwrite( &filesize, sizeof(filesize), 1, outf);

	int bitindex = 0;
	char ch;
	unsigned char byte = 0x00;

	ub4 mask = 0x00000000;
	//ub1 byte1 = 0x00;
	//int bitindex1 = 0;

	fscanf(inf, "%c", &ch);
	while ( !feof(inf) ) {
		Pattern bincode = bitPattern_.getPattern(ch);

		/*ub4 mk =0x8000;
		 ub1 tmp = 0x80;
		 for (size_t i=0; i<bincode.nbits; i++) {
		 if (bincode.bits & (mk>>i)) {
		 byte |= (tmp>>bitindex);
		 }
		 bitindex++;
		 if (bitindex== 8) {
		 fwrite(&byte, sizeof(byte), 1, outf);
		 bitindex= 0;
		 byte = 0x00;
		 }
		 }*/

		mask |= (bincode.bits>>bitindex);
		bitindex += bincode.nbits;
		//extract a byte once
		while(bitindex >= 8) {
			byte |= (mask>>24);
			//fwrite(&byte, sizeof(byte), 1, outf);
			fprintf(outf, "%c", byte);
			mask <<= 8;
			bitindex -= 8;
			byte = 0x00;
		}
		fscanf(inf, "%c", &ch);
	} // while: end of compress file char by char

	if (bitindex > 0) {
		byte |= (mask>>24);
		fprintf(outf, "%c", byte);
	}

	fclose(inf);
	fclose(outf);

}

void Compressor::decompressFile(const string& input, const string& output) {
	FILE *inf;
	FILE *outf;

	inf = fopen(input.c_str(), "r");
	if (inf == NULL) {
		CompressorException ex("cannot open input file\n");
		throw ex;
	}

	outf = fopen(output.c_str(), "w");
	if (outf == NULL) {
		CompressorException ex("cannot open output file\n");
		throw ex;
	}

	bitPattern_.fromFile(inf);

	long nbytes;
	//get file size
	fread( &nbytes, sizeof(nbytes), 1, inf);
	//bitPattern_.display();
	Tree *root;
	root= pg_.makeTree(bitPattern_);

	//	BitPattern bp = root->encode();
	//	bp.display();	

	Tree *pt;
	pt = root;

	while (nbytes>0) {
		unsigned char byte;
		unsigned char mask;
		unsigned char masked;
	    fread (&byte, sizeof(byte), 1, inf);
		//if (feof(inf) )
		//	break;
		mask = 0x80;
		masked = 0x00;

		/* split bits and walk on tree */
		while (mask != 0) {
			masked = byte & mask;
			mask >>= 1;

			if (masked == 0) {
				pt = pt -> left;
			} else {
				pt = pt -> right;
			}
			if (pt -> left == NULL && pt -> right == NULL) {
				/* OK, a leaf node */
				char ch = pt -> ch;
				fprintf(outf, "%c", ch);
				//fwrite(&ch, sizeof(char), 1, outf);
				nbytes--;				
				if (nbytes == 0) {
					break;
				}
				pt = root; /* back to root */
			}

		} // end of read a byte, get the next byte
	} // end of read fin, decompression finished

	delete root;
	fclose(inf);
	fclose(outf);
}

void Compressor::showCode(const string& inputFile, const string& outputFile) {

   int totalbits = 0;

	//generate bitPattern from inputFile.	
	PatternGenerator pg(inputFile, true);//'\n' is not encoded.
	bitPattern_ = pg.getBitPattern();
	displayPattern();

	ifstream ifs(inputFile.c_str() );
	ofstream ofs(outputFile.c_str() );
	string inString;
	map<string, string> codeMap;
	map<string, string>::iterator it;
	while (ifs >> inString) {
		string outString;
		showEncode(inString, outString);
		totalbits += outString.size();
		codeMap[inString] = outString;

		string temp;
		showDecode(outString, temp);			
		if (temp != inString) {
			cout<<inString<<"|"<<endl<<temp<<"|"<<endl;
		}

	}
	for (it=codeMap.begin(); it != codeMap.end(); it++) {
		ofs<<it->second<<":"<<it->first<<endl;
	}
	//cout<<"totalbits = "<<totalbits<<endl;
}

}}}
