//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "paragraphvec.h"

#define MAX_STRING 100
#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6
#define MAX_SENTENCE_LENGTH 1000000
#define MAX_CODE_LENGTH 40

static const int vocab_hash_size = 30000000;  // Maximum 30 * 0.7 = 21M words in the vocabulary


struct vocab_word
{
    long long cn;
    int *point;
    char *word, *code, codelen;
};

static char train_file[MAX_STRING] = {0}, output_file[MAX_STRING] = {0};
static char save_vocab_file[MAX_STRING] = {0}, read_vocab_file[MAX_STRING] = {0};
struct vocab_word *vocab;
static int binary = 1, cbow = 0, debug_mode = 2, window = 5, min_count = 5, num_threads = 1, min_reduce = 1;
static int *vocab_hash;
static long long vocab_max_size = 1000, vocab_size = 0, layer1_size = 100;
static long long train_words = 0, word_count_actual = 0, file_size = 0;
static real alpha = 0.025, starting_alpha, sample = 0;
static real *syn0, *syn1, *syn1neg, *expTable;
static clock_t start;

static int hs = 1, negative = 0;
static const int table_size = 1e8;
static int *table;

static void InitUnigramTable()
{
    int a, i;
    long long train_words_pow = 0;
    real d1, power = 0.75;
    table = (int *)malloc(table_size * sizeof(int));
    for (a = 0; a < vocab_size; a++) train_words_pow += pow(vocab[a].cn, power);
    i = 0;
    d1 = pow(vocab[i].cn, power) / (real)train_words_pow;
    for (a = 0; a < table_size; a++)
    {
        table[a] = i;
        if (a / (real)table_size > d1)
        {
            i++;
            d1 += pow(vocab[i].cn, power) / (real)train_words_pow;
        }
        if (i >= vocab_size) i = vocab_size - 1;
    }
}

// Reads a single word from a file, assuming space + tab + EOL to be word boundaries
static void ReadWord(char *word, FILE *fin)
{
    int a = 0, ch;
    while (!feof(fin))
    {
        ch = fgetc(fin);
        if (ch == 13) continue;
        if ((ch == ' ') || (ch == '\t') || (ch == '\n'))
        {
            if (a > 0)
            {
                if (ch == '\n') ungetc(ch, fin);
                break;
            }
            if (ch == '\n')
            {
                strcpy(word, (char *)"</s>");
                return;
            }
            else continue;
        }
        word[a] = ch;
        a++;
        if (a >= MAX_STRING - 1) a--;   // Truncate too long words
    }
    word[a] = 0;
}

// Returns hash value of a word
static int GetWordHash(char *word)
{
    unsigned long long a, hash = 0;
    for (a = 0; a < strlen(word); a++) hash = hash * 257 + word[a];
    hash = hash % vocab_hash_size;
    return hash;
}

// Returns position of a word in the vocabulary; if the word is not found, returns -1
static int SearchVocab(char *word)
{
    unsigned int hash = GetWordHash(word);
    while (1)
    {
        if (vocab_hash[hash] == -1) return -1;
        if (!strcmp(word, vocab[vocab_hash[hash]].word)) return vocab_hash[hash];
        hash = (hash + 1) % vocab_hash_size;
    }
    return -1;
}

// Reads a word and returns its index in the vocabulary
static int ReadWordIndex(FILE *fin)
{
    char word[MAX_STRING];
    ReadWord(word, fin);
    if (feof(fin)) return -1;
    return SearchVocab(word);
}

// Adds a word to the vocabulary
static int AddWordToVocab(char *word)
{
    unsigned int hash, length = strlen(word) + 1;
    if (length > MAX_STRING) length = MAX_STRING;
    vocab[vocab_size].word = (char *)calloc(length, sizeof(char));
    strcpy(vocab[vocab_size].word, word);
    vocab[vocab_size].cn = 0;
    vocab_size++;
    // Reallocate memory if needed
    if (vocab_size + 2 >= vocab_max_size)
    {
        vocab_max_size += 1000;
        vocab = (struct vocab_word *)realloc(vocab, vocab_max_size * sizeof(struct vocab_word));
    }
    hash = GetWordHash(word);
    while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
    vocab_hash[hash] = vocab_size - 1;
    return vocab_size - 1;
}

// Used later for sorting by word counts
static int VocabCompare(const void *a, const void *b)
{
    return ((struct vocab_word *)b)->cn - ((struct vocab_word *)a)->cn;
}

// Sorts the vocabulary by frequency using word counts
static void SortVocab()
{
    int a, size;
    unsigned int hash;
    // Sort the vocabulary and keep </s> at the first position
    qsort(&vocab[1], vocab_size - 1, sizeof(struct vocab_word), VocabCompare);
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    size = vocab_size;
    train_words = 0;
    for (a = 0; a < size; a++)
    {
        // Words occuring less than min_count times will be discarded from the vocab
        if (vocab[a].cn < min_count)
        {
            vocab_size--;
            free(vocab[vocab_size].word);
        }
        else
        {
            // Hash will be re-computed, as after the sorting it is not actual
            hash=GetWordHash(vocab[a].word);
            while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
            vocab_hash[hash] = a;
            train_words += vocab[a].cn;
        }
    }
    vocab = (struct vocab_word *)realloc(vocab, (vocab_size + 1) * sizeof(struct vocab_word));
    // Allocate memory for the binary tree construction
    for (a = 0; a < vocab_size; a++)
    {
        vocab[a].code = (char *)calloc(MAX_CODE_LENGTH, sizeof(char));
        vocab[a].point = (int *)calloc(MAX_CODE_LENGTH, sizeof(int));
    }
}

// Reduces the vocabulary by removing infrequent tokens
static void ReduceVocab()
{
    int a, b = 0;
    unsigned int hash;
    for (a = 0; a < vocab_size; a++) if (vocab[a].cn > min_reduce)
        {
            vocab[b].cn = vocab[a].cn;
            vocab[b].word = vocab[a].word;
            b++;
        }
        else free(vocab[a].word);
    vocab_size = b;
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    for (a = 0; a < vocab_size; a++)
    {
        // Hash will be re-computed, as it is not actual
        hash = GetWordHash(vocab[a].word);
        while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
        vocab_hash[hash] = a;
    }
    fflush(stdout);
    min_reduce++;
}

// Create binary Huffman tree using the word counts
// Frequent words will have short uniqe binary codes
static void CreateBinaryTree()
{
    long long a, b, i, min1i, min2i, pos1, pos2, point[MAX_CODE_LENGTH];
    char code[MAX_CODE_LENGTH];
    long long *count = (long long *)calloc(vocab_size * 2 + 1, sizeof(long long));
    long long *binary = (long long *)calloc(vocab_size * 2 + 1, sizeof(long long));
    long long *parent_node = (long long *)calloc(vocab_size * 2 + 1, sizeof(long long));
    for (a = 0; a < vocab_size; a++) count[a] = vocab[a].cn;
    for (a = vocab_size; a < vocab_size * 2; a++) count[a] = 1e15;
    pos1 = vocab_size - 1;
    pos2 = vocab_size;
    // Following algorithm constructs the Huffman tree by adding one node at a time
    for (a = 0; a < vocab_size - 1; a++)
    {
        // First, find two smallest nodes 'min1, min2'
        if (pos1 >= 0)
        {
            if (count[pos1] < count[pos2])
            {
                min1i = pos1;
                pos1--;
            }
            else
            {
                min1i = pos2;
                pos2++;
            }
        }
        else
        {
            min1i = pos2;
            pos2++;
        }
        if (pos1 >= 0)
        {
            if (count[pos1] < count[pos2])
            {
                min2i = pos1;
                pos1--;
            }
            else
            {
                min2i = pos2;
                pos2++;
            }
        }
        else
        {
            min2i = pos2;
            pos2++;
        }
        count[vocab_size + a] = count[min1i] + count[min2i];
        parent_node[min1i] = vocab_size + a;
        parent_node[min2i] = vocab_size + a;
        binary[min2i] = 1;
    }
    // Now assign binary code to each vocabulary word
    for (a = 0; a < vocab_size; a++)
    {
        b = a;
        i = 0;
        while (1)
        {
            code[i] = binary[b];
            point[i] = b;
            i++;
            b = parent_node[b];
            if (b == vocab_size * 2 - 2) break;
        }
        vocab[a].codelen = i;
        vocab[a].point[0] = vocab_size - 2;
        for (b = 0; b < i; b++)
        {
            vocab[a].code[i - b - 1] = code[b];
            vocab[a].point[i - b] = point[b] - vocab_size;
        }
    }
    free(count);
    free(binary);
    free(parent_node);
}

static void LearnVocabFromTrainFile()
{
    char word[MAX_STRING];
    FILE *fin;
    long long a, i;
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    fin = fopen(train_file, "rb");
    if (fin == NULL)
    {
        printf("ERROR: training data file not found!\n");
        exit(1);
    }
    vocab_size = 0;
    AddWordToVocab((char *)"</s>");
    while (1)
    {
        ReadWord(word, fin);
        if (feof(fin)) break;
        train_words++;
        if ((debug_mode > 1) && (train_words % 100000 == 0))
        {
            printf("%lldK%c", train_words / 1000, 13);
            fflush(stdout);
        }
        i = SearchVocab(word);
        if (i == -1)
        {
            a = AddWordToVocab(word);
            vocab[a].cn = 1;
        }
        else vocab[i].cn++;
        if (vocab_size > vocab_hash_size * 0.7) ReduceVocab();
    }
    SortVocab();
    if (debug_mode > 0)
    {
        printf("Vocab size: %lld\n", vocab_size);
        printf("Words in train file: %lld\n", train_words);
    }
    fclose(fin);
}

static void SaveVocab()
{
    long long i;
    FILE *fo = fopen(save_vocab_file, "wb");
    for (i = 0; i < vocab_size; i++) fprintf(fo, "%s %lld\n", vocab[i].word, vocab[i].cn);
    fclose(fo);
}

static void ReadVocab()
{
    long long a, i = 0;
    char c;
    char word[MAX_STRING];
    FILE *fin = fopen(read_vocab_file, "rb");
    if (fin == NULL)
    {
        printf("Vocabulary file not found\n");
        exit(1);
    }
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    vocab_size = 0;
    while (1)
    {
        ReadWord(word, fin);
        if (feof(fin)) break;
        a = AddWordToVocab(word);
        fscanf(fin, "%lld%c", &vocab[a].cn, &c);
        i++;
    }
    SortVocab();
    if (debug_mode > 0)
    {
        printf("Vocab size: %lld\n", vocab_size);
        printf("Words in train file: %lld\n", train_words);
    }
    fclose(fin);
}

static void InitNet()
{
    long long a, b;
    a = posix_memalign((void **)&syn0, 128, (long long)vocab_size * layer1_size * sizeof(real));
    if (syn0 == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    if (hs)
    {
        a = posix_memalign((void **)&syn1, 128, (long long)vocab_size * layer1_size * sizeof(real));
        if (syn1 == NULL)
        {
            printf("Memory allocation failed\n");
            exit(1);
        }
        for (b = 0; b < layer1_size; b++) for (a = 0; a < vocab_size; a++)
                syn1[a * layer1_size + b] = 0;
    }
    if (negative>0)
    {
        a = posix_memalign((void **)&syn1neg, 128, (long long)vocab_size * layer1_size * sizeof(real));
        if (syn1neg == NULL)
        {
            printf("Memory allocation failed\n");
            exit(1);
        }
        for (b = 0; b < layer1_size; b++) for (a = 0; a < vocab_size; a++)
                syn1neg[a * layer1_size + b] = 0;
    }
    for (b = 0; b < layer1_size; b++) for (a = 0; a < vocab_size; a++)
            syn0[a * layer1_size + b] = (rand() / (real)RAND_MAX - 0.5) / layer1_size;
    CreateBinaryTree();
}

static void *TrainModelThread(void *id)
{
    long long a, b, d, word, last_word, sentence_length = 0, sentence_position = 0;
    long long word_count = 0, last_word_count = 0;
    long long *sen = (long long*)calloc(MAX_SENTENCE_LENGTH + 1, sizeof(long long));
    long long l1, l2, c, target, label;
    unsigned long long next_random = (long long)id;
    real f, g;
    clock_t now;
    real *neu1 = (real *)calloc(layer1_size, sizeof(real));
    real *neu1e = (real *)calloc(layer1_size, sizeof(real));
    real *paraV = (real *)calloc(layer1_size, sizeof(real));
    FILE *fi = fopen(train_file, "rb");
    fseek(fi, file_size / (long long)num_threads * (long long)id, SEEK_SET);
    while (1)
    {
        if (word_count - last_word_count > 10000)
        {
            word_count_actual += word_count - last_word_count;
            last_word_count = word_count;
            if ((debug_mode > 1))
            {
                now=clock();
                printf("%cAlpha: %f  Progress: %.2f%%  Words/thread/sec: %.2fk  ", 13, alpha,
                       word_count_actual / (real)(train_words + 1) * 100,
                       word_count_actual / ((real)(now - start + 1) / (real)CLOCKS_PER_SEC * 1000));
                fflush(stdout);
            }
            alpha = starting_alpha * (1 - word_count_actual / (real)(train_words + 1));
            if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;
        }
        if (sentence_length == 0)
        {
            while (1)
            {
                word = ReadWordIndex(fi);
                if (feof(fi)) break;
                if (word == -1) continue;
                word_count++;
                if (word == 0) break;
                // The subsampling randomly discards frequent words while keeping the ranking same
                if (sample > 0)
                {
                    real ran = (sqrt(vocab[word].cn / (sample * train_words)) + 1) * (sample * train_words) / vocab[word].cn;
                    next_random = next_random * (unsigned long long)25214903917 + 11;
                    if (ran < (next_random & 0xFFFF) / (real)65536) continue;
                }
                sen[sentence_length] = word;
                sentence_length++;
                if (sentence_length >= MAX_SENTENCE_LENGTH)
                {
                    //printf("Paragraph is too long, more than %d words.\n", MAX_SENTENCE_LENGTH);
                    break;
                }
                if (word_count > train_words / num_threads) break;
            }
            sentence_position = 0;
            for (b = 0; b < layer1_size; b++)
                paraV[b] = (rand() / (real)RAND_MAX - 0.5) / layer1_size;
        }
        if (feof(fi)) break;
        if (word_count > train_words / num_threads) break;
        word = sen[sentence_position];
        if (word == -1) continue;
        for (c = 0; c < layer1_size; c++) neu1[c] = 0;
        for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
        next_random = next_random * (unsigned long long)25214903917 + 11;
        b = next_random % window;
        if (cbow)    //train the cbow architecture
        {
            // in -> hidden
            for (a = b; a < window * 2 + 1 - b; a++) if (a != window)
                {
                    c = sentence_position - window + a;
                    if (c < 0) continue;
                    if (c >= sentence_length) continue;
                    last_word = sen[c];
                    if (last_word == -1) continue;
                    for (c = 0; c < layer1_size; c++) neu1[c] += syn0[c + last_word * layer1_size];
                }

            //add para vector
            for (c = 0; c < layer1_size; c++)
            {
                neu1[c] += paraV[c];
            }


            if (hs) for (d = 0; d < vocab[word].codelen; d++)
                {
                    f = 0;
                    l2 = vocab[word].point[d] * layer1_size;
                    // Propagate hidden -> output
                    for (c = 0; c < layer1_size; c++) f += neu1[c] * syn1[c + l2];
                    if (f <= -MAX_EXP) continue;
                    else if (f >= MAX_EXP) continue;
                    else f = expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
                    // 'g' is the gradient multiplied by the learning rate
                    g = (1 - vocab[word].code[d] - f) * alpha;
                    // Propagate errors output -> hidden
                    for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1[c + l2];
                    // Learn weights hidden -> output
                    for (c = 0; c < layer1_size; c++) syn1[c + l2] += g * neu1[c];
                }
            // NEGATIVE SAMPLING
            if (negative > 0) for (d = 0; d < negative + 1; d++)
                {
                    if (d == 0)
                    {
                        target = word;
                        label = 1;
                    }
                    else
                    {
                        next_random = next_random * (unsigned long long)25214903917 + 11;
                        target = table[(next_random >> 16) % table_size];
                        if (target == 0) target = next_random % (vocab_size - 1) + 1;
                        if (target == word) continue;
                        label = 0;
                    }
                    l2 = target * layer1_size;
                    f = 0;
                    for (c = 0; c < layer1_size; c++) f += neu1[c] * syn1neg[c + l2];
                    if (f > MAX_EXP) g = (label - 1) * alpha;
                    else if (f < -MAX_EXP) g = (label - 0) * alpha;
                    else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                    for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
                    for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * neu1[c];
                }
            // hidden -> in
            for (a = b; a < window * 2 + 1 - b; a++) if (a != window)
                {
                    c = sentence_position - window + a;
                    if (c < 0) continue;
                    if (c >= sentence_length) continue;
                    last_word = sen[c];
                    if (last_word == -1) continue;
                    for (c = 0; c < layer1_size; c++) syn0[c + last_word * layer1_size] += neu1e[c];
                }
            for (c = 0; c < layer1_size; c++) paraV[c] += neu1e[c];

        }
        else      //train skip-gram
        {
            for (a = b; a < window * 2 + 1 - b; a++) if (a != window)
                {
                    c = sentence_position - window + a;
                    if (c < 0) continue;
                    if (c >= sentence_length) continue;
                    last_word = sen[c];
                    if (last_word == -1) continue;
                    l1 = last_word * layer1_size;
                    for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
                    // HIERARCHICAL SOFTMAX
                    if (hs) for (d = 0; d < vocab[word].codelen; d++)
                        {
                            f = 0;
                            l2 = vocab[word].point[d] * layer1_size;
                            // Propagate hidden -> output
                            for (c = 0; c < layer1_size; c++) f += (syn0[c + l1] + paraV[c]) * syn1[c + l2];
                            if (f <= -MAX_EXP) continue;
                            else if (f >= MAX_EXP) continue;
                            else f = expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
                            // 'g' is the gradient multiplied by the learning rate
                            g = (1 - vocab[word].code[d] - f) * alpha;
                            // Propagate errors output -> hidden
                            for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1[c + l2];
                            // Learn weights hidden -> output
                            for (c = 0; c < layer1_size; c++) syn1[c + l2] += g * (syn0[c + l1] + paraV[c]);
                        }
                    // NEGATIVE SAMPLING
                    if (negative > 0) for (d = 0; d < negative + 1; d++)
                        {
                            if (d == 0)
                            {
                                target = word;
                                label = 1;
                            }
                            else
                            {
                                next_random = next_random * (unsigned long long)25214903917 + 11;
                                target = table[(next_random >> 16) % table_size];
                                if (target == 0) target = next_random % (vocab_size - 1) + 1;
                                if (target == word) continue;
                                label = 0;
                            }
                            l2 = target * layer1_size;
                            f = 0;
                            for (c = 0; c < layer1_size; c++) f += (syn0[c + l1] + paraV[c]) * syn1neg[c + l2];
                            if (f > MAX_EXP) g = (label - 1) * alpha;
                            else if (f < -MAX_EXP) g = (label - 0) * alpha;
                            else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
                            for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * (syn0[c + l1] + paraV[c]);
                        }
                    // Learn weights input -> hidden
                    for (c = 0; c < layer1_size; c++)
                    {
                        syn0[c + l1] += neu1e[c];
                        paraV[c] += neu1e[c];
                    }
                }
        }
        sentence_position++;
        if (sentence_position >= sentence_length)
        {
            sentence_length = 0;

            //output para vector.
            /*for (b = 0; b < layer1_size; b++) printf("%lf ", paraV[b]);
            printf("\n");*/

            continue;
        }
    }
    fclose(fi);
    free(neu1);
    free(neu1e);
    free(sen);
    free(paraV);
    pthread_exit(NULL);
}

void TrainMLModel()
{
    if (train_file[0] == 0 || output_file[0] == 0) return;

    long a, b;
    FILE *fo;
    pthread_t *pt = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    printf("Starting training using file %s\n", train_file);

    FILE* fin = fopen(train_file, "rb");
    if (fin == NULL)
    {
        printf("ERROR: training data file not found!\n");
        exit(1);
    }
    fseek(fin, 0, SEEK_END);
    file_size = ftell(fin);
    fclose(fin);

    start = clock();
    for (a = 0; a < num_threads; a++) pthread_create(&pt[a], NULL, TrainModelThread, (void *)a);
    for (a = 0; a < num_threads; a++) pthread_join(pt[a], NULL);
    fo = fopen(output_file, "wb");

    // Save the word vectors
    fprintf(fo, "%lld %lld\n", vocab_size, layer1_size);
    for (a = 0; a < vocab_size; a++)
    {
        fprintf(fo, "%s ", vocab[a].word);
        if (binary) for (b = 0; b < layer1_size; b++) fwrite(&syn0[a * layer1_size + b], sizeof(real), 1, fo);
        else for (b = 0; b < layer1_size; b++) fprintf(fo, "%lf ", syn0[a * layer1_size + b]);
        fprintf(fo, "\n");
    }


    fclose(fo);
}


void setParameter(const ModelArgument& ma)
{
    layer1_size = ma.layer1_size;
    strcpy(train_file, ma.train_file.c_str());
    strcpy(save_vocab_file, ma.save_vocab_file.c_str());
    strcpy(read_vocab_file, ma.read_vocab_file.c_str());
    cbow = ma.cbow;
    alpha = ma.alpha;
    strcpy(output_file, ma.output_file.c_str());
    window = ma.window;
    sample = ma.sample;
    hs = ma.hs;
    negative = ma.negative;
    num_threads = ma.num_threads;
    min_count = ma.min_count;
}


bool initML()
{
    vocab = (struct vocab_word *)calloc(vocab_max_size, sizeof(struct vocab_word));
    vocab_hash = (int *)calloc(vocab_hash_size, sizeof(int));
    expTable = (real *)malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
    for (int i = 0; i < EXP_TABLE_SIZE; i++)
    {
        expTable[i] = exp((i / (real)EXP_TABLE_SIZE * 2 - 1) * MAX_EXP); // Precompute the exp() table
        expTable[i] = expTable[i] / (expTable[i] + 1);                   // Precompute f(x) = x / (x + 1)
    }

    starting_alpha = alpha;
    if (read_vocab_file[0] != 0) ReadVocab();
    else LearnVocabFromTrainFile();
    if (save_vocab_file[0] != 0) SaveVocab();

    InitNet();
    if (negative > 0) InitUnigramTable();

    return true;
}


static bool readWordVector()
{
    FILE* fwv = fopen(output_file, "wb");
    if (!fwv)
    {
        printf("Word vector input file not found\n");
        return false;
    }
    fprintf(fwv, "%lld %lld\n", vocab_size, layer1_size);

    char word[MAX_STRING], ch;
    for (long long b = 0; b < vocab_size; b++)
    {
        fscanf(fwv, "%s%c", word, &ch);
        for (long long a = 0; a < layer1_size; a++) fread(&syn0[a + b * layer1_size], sizeof(real), 1, fwv);
    }

    fclose(fwv);
    return true;
}




// Reads a word and returns its index in the vocabulary
static int ReadWordIndex(const std::string& str, long long& start)
{
    char word[MAX_STRING] = {0};

    int a = 0, ch;
    while (start < (long long)str.length())
    {
        ch = str[start++];
        if (ch == 13) continue;
        if ((ch == ' ') || (ch == '\t') || (ch == '\n'))
        {
            if (a > 0)
            {
                if (ch == '\n') --start;
                break;
            }
            if (ch == '\n')
            {
                strcpy(word, (char *)"</s>");
                a += 4;
                break;
            }
            else continue;
        }
        word[a] = ch;
        a++;
        if (a >= MAX_STRING - 1) a--;   // Truncate too long words
    }
    word[a] = 0;


    if (start >= (long long )str.length()) return -1;
    return SearchVocab(word);
}

std::vector<real> Predict(const std::string& para)
{
    long long a, b, d, word, last_word, sentence_length = 0, sentence_position = 0;
    long long word_count = 0, last_word_count = 0;
    long long *sen = (long long*)calloc(MAX_SENTENCE_LENGTH + 1, sizeof(long long));
    long long l1, l2, c, target, label;
    long long paraIndex = 0;
    unsigned long long next_random = (long long)time(NULL);
    real f, g;
    real *neu1 = (real *)calloc(layer1_size, sizeof(real));
    real *neu1e = (real *)calloc(layer1_size, sizeof(real));
    real *paraV = (real *)calloc(layer1_size, sizeof(real));



    while (1)
    {
        if (word_count - last_word_count > 10000)
        {
            word_count_actual += word_count - last_word_count;
            last_word_count = word_count;
            alpha = starting_alpha * (1 - word_count_actual / (real)(train_words + 1));
            if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;
        }
        if (sentence_length == 0)
        {
            while (1)
            {
                word = ReadWordIndex(para, paraIndex);
                if (paraIndex >= (long long )para.length()) break;
                if (word == -1) continue;
                word_count++;
                if (word == 0) break;
                // The subsampling randomly discards frequent words while keeping the ranking same
                if (sample > 0)
                {
                    real ran = (sqrt(vocab[word].cn / (sample * train_words)) + 1) * (sample * train_words) / vocab[word].cn;
                    next_random = next_random * (unsigned long long)25214903917 + 11;
                    if (ran < (next_random & 0xFFFF) / (real)65536) continue;
                }
                sen[sentence_length] = word;
                sentence_length++;
                if (sentence_length >= MAX_SENTENCE_LENGTH)
                {
                    printf("Paragraph is too long, more than %d words.\n", MAX_SENTENCE_LENGTH);
                    break;
                }
            }
            sentence_position = 0;
            for (b = 0; b < layer1_size; b++)
                paraV[b] = (rand() / (real)RAND_MAX - 0.5) / layer1_size;
        }
        word = sen[sentence_position];
        if (word == -1) continue;
        for (c = 0; c < layer1_size; c++) neu1[c] = 0;
        for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
        next_random = next_random * (unsigned long long)25214903917 + 11;
        b = next_random % window;
        if (cbow)    //train the cbow architecture
        {
            // in -> hidden
            for (a = b; a < window * 2 + 1 - b; a++) if (a != window)
                {
                    c = sentence_position - window + a;
                    if (c < 0) continue;
                    if (c >= sentence_length) continue;
                    last_word = sen[c];
                    if (last_word == -1) continue;
                    for (c = 0; c < layer1_size; c++) neu1[c] += syn0[c + last_word * layer1_size];
                }

            //add para vector
            for (c = 0; c < layer1_size; c++)
            {
                neu1[c] += paraV[c];
            }


            if (hs) for (d = 0; d < vocab[word].codelen; d++)
                {
                    f = 0;
                    l2 = vocab[word].point[d] * layer1_size;
                    // Propagate hidden -> output
                    for (c = 0; c < layer1_size; c++) f += neu1[c] * syn1[c + l2];
                    if (f <= -MAX_EXP) continue;
                    else if (f >= MAX_EXP) continue;
                    else f = expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
                    // 'g' is the gradient multiplied by the learning rate
                    g = (1 - vocab[word].code[d] - f) * alpha;
                    // Propagate errors output -> hidden
                    for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1[c + l2];
                    // Learn weights hidden -> output
                    for (c = 0; c < layer1_size; c++) syn1[c + l2] += g * neu1[c];
                }
            // NEGATIVE SAMPLING
            if (negative > 0) for (d = 0; d < negative + 1; d++)
                {
                    if (d == 0)
                    {
                        target = word;
                        label = 1;
                    }
                    else
                    {
                        next_random = next_random * (unsigned long long)25214903917 + 11;
                        target = table[(next_random >> 16) % table_size];
                        if (target == 0) target = next_random % (vocab_size - 1) + 1;
                        if (target == word) continue;
                        label = 0;
                    }
                    l2 = target * layer1_size;
                    f = 0;
                    for (c = 0; c < layer1_size; c++) f += neu1[c] * syn1neg[c + l2];
                    if (f > MAX_EXP) g = (label - 1) * alpha;
                    else if (f < -MAX_EXP) g = (label - 0) * alpha;
                    else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                    for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
                    for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * neu1[c];
                }
            // hidden -> in
            for (a = b; a < window * 2 + 1 - b; a++) if (a != window)
                {
                    c = sentence_position - window + a;
                    if (c < 0) continue;
                    if (c >= sentence_length) continue;
                    last_word = sen[c];
                    if (last_word == -1) continue;
                    for (c = 0; c < layer1_size; c++) syn0[c + last_word * layer1_size] += neu1e[c];
                }
            for (c = 0; c < layer1_size; c++) paraV[c] += neu1e[c];

        }
        else      //train skip-gram
        {
            for (a = b; a < window * 2 + 1 - b; a++) if (a != window)
                {
                    c = sentence_position - window + a;
                    if (c < 0) continue;
                    if (c >= sentence_length) continue;
                    last_word = sen[c];
                    if (last_word == -1) continue;
                    l1 = last_word * layer1_size;
                    for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
                    // HIERARCHICAL SOFTMAX
                    if (hs) for (d = 0; d < vocab[word].codelen; d++)
                        {
                            f = 0;
                            l2 = vocab[word].point[d] * layer1_size;
                            // Propagate hidden -> output
                            for (c = 0; c < layer1_size; c++) f += (syn0[c + l1] + paraV[c]) * syn1[c + l2];
                            if (f <= -MAX_EXP) continue;
                            else if (f >= MAX_EXP) continue;
                            else f = expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
                            // 'g' is the gradient multiplied by the learning rate
                            g = (1 - vocab[word].code[d] - f) * alpha;
                            // Propagate errors output -> hidden
                            for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1[c + l2];
                            // Learn weights hidden -> output
                            for (c = 0; c < layer1_size; c++) syn1[c + l2] += g * (syn0[c + l1] + paraV[c]);
                        }
                    // NEGATIVE SAMPLING
                    if (negative > 0) for (d = 0; d < negative + 1; d++)
                        {
                            if (d == 0)
                            {
                                target = word;
                                label = 1;
                            }
                            else
                            {
                                next_random = next_random * (unsigned long long)25214903917 + 11;
                                target = table[(next_random >> 16) % table_size];
                                if (target == 0) target = next_random % (vocab_size - 1) + 1;
                                if (target == word) continue;
                                label = 0;
                            }
                            l2 = target * layer1_size;
                            f = 0;
                            for (c = 0; c < layer1_size; c++) f += (syn0[c + l1] + paraV[c]) * syn1neg[c + l2];
                            if (f > MAX_EXP) g = (label - 1) * alpha;
                            else if (f < -MAX_EXP) g = (label - 0) * alpha;
                            else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
                            for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * (syn0[c + l1] + paraV[c]);
                        }
                    // Learn weights input -> hidden
                    for (c = 0; c < layer1_size; c++)
                    {
                        syn0[c + l1] += neu1e[c];
                        paraV[c] += neu1e[c];
                    }
                }
        }
        sentence_position++;
        if (sentence_position >= sentence_length)
        {
            break;
        }
    }


    std::vector<real> res(paraV, paraV + layer1_size);

    free(neu1);
    free(neu1e);
    free(sen);
    free(paraV);


    return res;
}
