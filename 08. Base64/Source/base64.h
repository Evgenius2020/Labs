#ifndef BASE64
#define BASE64

#define SUCCESS 1
#define FAILURE 0

#define IGNORE_DECODE_MODE 1
#define NORMAL_DECODE_MODE 0

int getSixByChar(char chr);

// Encodes a byte file named inputFileName to a base64-text file named outputFileName.
// (lineSize == 0) for no formatting. Else after every lineSize'th symbol puts '\n'.
// Returns FAILURE on wrong file names or SUCCESS after encoding.
int encode(char* inputFileName, char* outputFileName, unsigned int lineSize);

// Decodes a base64-text file named inputFileName to a byte file named outputFileName.
// Returns FAILURE if inputFile contains mistakes (unknown symbols, less symbols) or
// SUCCESS after decoding. If (ignoreFlag != 0), unknown symbols won't triggering FAILURE.
int decode(char* inputFileName, char* outputFileName, int ignoreFlag);

#endif