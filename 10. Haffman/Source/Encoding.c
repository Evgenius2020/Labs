#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../Common/PriorityQueue/Source/PriorityQueue.c"
#include "BiteWriter.h"
#include "Encoding.h"

#define exit return;

typedef struct Node {
	struct Node* parent;
	char sgn; /* 0 - if left leaf, 1 - if right leaf*/
	short freq;
}Node;

Node* createEncodeNode(short freq) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->parent = NULL;
	node->freq = freq;

	return node;
}

/* Running a file, inserts their code to outputfile */
void encodeText(FILE* in, BiteWriter* writer, char** codes) {
	short chr, i;
	while ((chr = fgetc(in)) != EOF) {
		for (i = 0; i < strlen(codes[chr]); i++) {
			biteWriterEnqueue(writer, 1, codes[chr][i]); // TODO: whole code must be inserted into queue
		}
	}
}

/* Generates codes for each char to encode */
char** generateCodes(int* freq) {
	PriorQueue* queue = priorQueueCreate(256);
	Node** leaves = (Node**)calloc(256, sizeof(Node*)); /* char-ended leaves for navigation.*/
	char** codes = (char**)calloc(sizeof(char*), 256);
	short i;
	for (i = 0; i < 256; i++) {
		if (freq[i]) {
			leaves[i] = createEncodeNode(freq[i]);
			codes[i] = (char*)malloc(sizeof(char) * 256);
			priorQueueInsert(queue, leaves[i], freq[i]);
		}
	}

	Node *newNode, *left, *right;
	/* Coding tree building */
	while (!priorQueueIsEmpty(queue)) {
		left = priorQueueExtractMin(queue);
		right = priorQueueExtractMin(queue);
		if (right) {
			newNode = createEncodeNode(left->freq + right->freq);
			left->parent = newNode;
			left->sgn = 0;
			right->parent = newNode;
			right->sgn = 1;
			priorQueueInsert(queue, newNode, newNode->freq);
		}
	}

	Node* buf;
	int strIndex;
	for (i = 0; i < 256; i++) {
		if (freq[i]) {
			strIndex = 0;
			buf = leaves[i];
			while (buf->parent) {
				codes[i][strIndex++] = buf->sgn + '0';
				buf = buf->parent;
			}
			codes[i][strIndex] = '\0';
			codes[i] = strrev(codes[i]);
		}
	}

	Node* parent;
	for (i = 0; i < 256; i++) {
		buf = leaves[i];
		if (buf) {
			parent = buf->parent;
			free(buf);
			buf = parent;
		}
	}
	priorQueueDestroy(queue);
	free(leaves);
	return codes;
}

char serializeCodes(char** codes, BiteWriter* writer) {
	short i, j;
	for (i = 0; i < 256; i++) {
		if (codes[i]) {
			biteWriterEnqueue(writer, 8, i);
			biteWriterEnqueue(writer, 8, strlen(codes[i]));
			for (j = 0; j < strlen(codes[i]); j++) {
				biteWriterEnqueue(writer, 1, codes[i][j]); // TODO: NO WAY MAN!
			}
		}
	}
}

void oneCharAlphabetCase(FILE* in, FILE* out, char chr, int freq) {
	BiteWriter* writer = biteWriterCreate(out);
	biteWriterEnqueue(writer, 8, 1); /* AlphabetSize*/
	biteWriterEnqueue(writer, 8, chr); /* Char*/
	biteWriterEnqueue(writer, 8, 1); /* Code length*/
	biteWriterEnqueue(writer, 1, 1); /* Code*/
	char codeLength = (8 + 8 + 8 + 1 + 3 + freq) % 8;
	biteWriterEnqueue(writer, 3, (8 - codeLength) % 8); /* Number of fakes*/
	biteWriterEnqueue(writer, (8 - codeLength) % 8, 0); /* Fakes*/
	while(freq--) {
		biteWriterEnqueue(writer, 1, 1); // TODO: TO SLOW!
	}
}

void manyCharsAlphabetCase(FILE* in, FILE* out, int* freq, char alphabetSize) {
	BiteWriter* writer = biteWriterCreate(out);
	biteWriterEnqueue(writer, 8, alphabetSize); /* Alphabet size */
	char** codes = generateCodes(freq);
	short freeBites = serializeCodes(codes, writer); /* Serialized codes */
	short i;
	for (i = 0; i < 256; i++) {
		if (freq[i]) {
			freeBites += strlen(codes[i]) * freq[i];
			freeBites %= 8;
		}
	}
	freeBites = (freeBites + 3) % 8;
	biteWriterEnqueue(writer, 3, (8 - freeBites) % 8); /* Number of fakes*/
	biteWriterEnqueue(writer, (8 - freeBites) % 8, 0); /* Fakes*/
	encodeText(in, writer, codes); /* Encoded text */
}

void encode(FILE* in, FILE* out) {
	int fileStart = ftell(in);
	unsigned char alphabetSize = 0; /* Number of unique chars in the text */
	short chr; /* EOF-handling */
	int* freq = calloc(256, sizeof(int));
	while ((chr = fgetc(in)) != EOF) {
		if (!freq[chr]) {
			alphabetSize++;
		}
		freq[chr]++;
	}
	fseek(in, fileStart, SEEK_SET);

	short i;
	if (alphabetSize == 1) {
		for (i = 0; i < 256, !freq[i]; i++);
		oneCharAlphabetCase(in, out, i, freq[i]);
	}
	if (alphabetSize > 1) {
		manyCharsAlphabetCase(in, out , freq, alphabetSize);
	}
	exit;
}