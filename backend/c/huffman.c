#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_TREE_HT 256

// --- Data Structures ---

struct MinHeapNode {
    unsigned char data;
    unsigned freq;
    struct MinHeapNode *left, *right;
};

struct MinHeap {
    unsigned size;
    unsigned capacity;
    struct MinHeapNode** array;
};

struct Code {
    char bits[256];
    int len;
};

struct Code codeTable[256];

// --- Helper Functions ---

struct MinHeapNode* newNode(unsigned char data, unsigned freq) {
    struct MinHeapNode* temp = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode));
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
    return temp;
}

struct MinHeap* createMinHeap(unsigned capacity) {
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (struct MinHeapNode**)malloc(minHeap->capacity * sizeof(struct MinHeapNode*));
    return minHeap;
}

void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b) {
    struct MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(struct MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

int isSizeOne(struct MinHeap* minHeap) {
    return (minHeap->size == 1);
}

struct MinHeapNode* extractMin(struct MinHeap* minHeap) {
    struct MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode) {
    ++minHeap->size;
    int i = minHeap->size - 1;
    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

void buildMinHeap(struct MinHeap* minHeap) {
    int n = minHeap->size - 1;
    int i;
    for (i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
}

int isLeaf(struct MinHeapNode* root) {
    return !(root->left) && !(root->right);
}

struct MinHeap* createAndBuildMinHeap(unsigned char data[], int freq[], int size) {
    struct MinHeap* minHeap = createMinHeap(size);
    for (int i = 0; i < size; ++i)
        minHeap->array[i] = newNode(data[i], freq[i]);
    minHeap->size = size;
    buildMinHeap(minHeap);
    return minHeap;
}

struct MinHeapNode* buildHuffmanTree(unsigned char data[], int freq[], int size) {
    struct MinHeapNode *left, *right, *top;
    struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);

    while (!isSizeOne(minHeap)) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);
        top = newNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);
    }
    return extractMin(minHeap);
}

void generateCodes(struct MinHeapNode* root, char* code, int top) {
    if (root->left) {
        code[top] = '0';
        generateCodes(root->left, code, top + 1);
    }
    if (root->right) {
        code[top] = '1';
        generateCodes(root->right, code, top + 1);
    }
    if (isLeaf(root)) {
        code[top] = '\0';
        strcpy(codeTable[root->data].bits, code);
        codeTable[root->data].len = top;
    }
}

// --- File I/O ---

void writeHeader(FILE* out, int freq[]) {
    // Simple header: 256 integers for frequencies
    fwrite(freq, sizeof(int), 256, out);
}

void readHeader(FILE* in, int freq[]) {
    fread(freq, sizeof(int), 256, in);
}

void compressFile(const char* inputFile, const char* outputFile) {
    FILE *in = fopen(inputFile, "rb");
    if (!in) { perror("Error opening input file"); exit(1); }

    int freq[256] = {0};
    unsigned char buffer[1024];
    size_t n;
    long originalSize = 0;

    while ((n = fread(buffer, 1, 1024, in)) > 0) {
        originalSize += n;
        for (size_t i = 0; i < n; i++)
            freq[buffer[i]]++;
    }

    // Collect valid characters
    unsigned char data[256];
    int validFreq[256];
    int size = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            data[size] = (unsigned char)i;
            validFreq[size] = freq[i];
            size++;
        }
    }

    struct MinHeapNode* root = buildHuffmanTree(data, validFreq, size);
    char code[MAX_TREE_HT];
    generateCodes(root, code, 0);

    FILE *out = fopen(outputFile, "wb");
    if (!out) { perror("Error opening output file"); exit(1); }

    writeHeader(out, freq);
    
    // Write original size (needed for decompression to know when to stop)
    fwrite(&originalSize, sizeof(long), 1, out);

    fseek(in, 0, SEEK_SET);
    
    unsigned char outBuffer = 0;
    int bitCount = 0;

    while ((n = fread(buffer, 1, 1024, in)) > 0) {
        for (size_t i = 0; i < n; i++) {
            char* c = codeTable[buffer[i]].bits;
            for (int j = 0; c[j] != '\0'; j++) {
                if (c[j] == '1')
                    outBuffer |= (1 << (7 - bitCount));
                bitCount++;
                if (bitCount == 8) {
                    fputc(outBuffer, out);
                    outBuffer = 0;
                    bitCount = 0;
                }
            }
        }
    }
    if (bitCount > 0) {
        fputc(outBuffer, out);
    }

    long compressedSize = ftell(out);
    
    // Calculate Entropy
    double entropy = 0.0;
    if (originalSize > 0) {
        for(int i=0; i<256; i++) {
            if(freq[i] > 0) {
                double p = (double)freq[i] / originalSize;
                entropy -= p * log2(p);
            }
        }
    }

    printf("{\"originalSize\": %ld, \"compressedSize\": %ld, \"ratio\": %.2f, \"entropy\": %.4f}\n", 
           originalSize, compressedSize, (double)originalSize/compressedSize, entropy);

    fclose(in);
    fclose(out);
}

void decompressFile(const char* inputFile, const char* outputFile) {
    FILE *in = fopen(inputFile, "rb");
    if (!in) { perror("Error opening input file"); exit(1); }

    // Get compressed size
    fseek(in, 0, SEEK_END);
    long compressedSize = ftell(in);
    fseek(in, 0, SEEK_SET);

    int freq[256];
    readHeader(in, freq);

    long originalSize;
    fread(&originalSize, sizeof(long), 1, in);

    unsigned char data[256];
    int validFreq[256];
    int size = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            data[size] = (unsigned char)i;
            validFreq[size] = freq[i];
            size++;
        }
    }

    struct MinHeapNode* root = buildHuffmanTree(data, validFreq, size);
    struct MinHeapNode* curr = root;

    FILE *out = fopen(outputFile, "wb");
    if (!out) { perror("Error opening output file"); exit(1); }

    unsigned char buffer[1024];
    size_t n;
    long decodedBytes = 0;

    while ((n = fread(buffer, 1, 1024, in)) > 0 && decodedBytes < originalSize) {
        for (size_t i = 0; i < n; i++) {
            for (int j = 0; j < 8; j++) {
                if (buffer[i] & (1 << (7 - j)))
                    curr = curr->right;
                else
                    curr = curr->left;

                if (isLeaf(curr)) {
                    fputc(curr->data, out);
                    decodedBytes++;
                    curr = root;
                    if (decodedBytes == originalSize) break;
                }
            }
            if (decodedBytes == originalSize) break;
        }
    }

    // Calculate Entropy from the recovered frequency table
    double entropy = 0.0;
    if (originalSize > 0) {
        for(int i=0; i<256; i++) {
            if(freq[i] > 0) {
                double p = (double)freq[i] / originalSize;
                entropy -= p * log2(p);
            }
        }
    }

    printf("{\"originalSize\": %ld, \"compressedSize\": %ld, \"ratio\": %.2f, \"entropy\": %.4f, \"decodedBytes\": %ld}\n", 
           originalSize, compressedSize, (double)originalSize/compressedSize, entropy, decodedBytes);

    fclose(in);
    fclose(out);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <command> <input> <output>\n", argv[0]);
        return 1;
    }

    clock_t start = clock();

    if (strcmp(argv[1], "compress") == 0) {
        compressFile(argv[2], argv[3]);
    } else if (strcmp(argv[1], "decompress") == 0) {
        decompressFile(argv[2], argv[3]);
    } else {
        fprintf(stderr, "Invalid command\n");
        return 1;
    }

    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    // We append time to the JSON output in the wrapper or just print it here to stderr
    // But for simplicity, let's just print it to stderr so it doesn't mess up JSON parsing if we capture stdout
    // Or we can include it in the JSON.
    // The compress function prints the JSON. We can't easily append to it without buffering.
    // Let's just rely on the compress function's output for now.
    
    return 0;
}
