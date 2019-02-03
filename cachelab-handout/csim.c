#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

typedef struct cache_line
{
    long tagbit;
    bool is_dirty;
} cache_line_t;

typedef struct cache_set
{
    cache_line_t *cache;
    size_t lines;
} cache_set_t;

static void initCache(cache_set_t **cache, size_t sets, size_t lines);
static void freeCache(cache_set_t *cache, size_t sets);

static void analyze_op(const char * const operation, char * const op_type, void **addr, size_t *op_size, bool *is_valid);
static void access_cache(cache_set_t *cache, long tag, long set_index, int *hit, int *miss, int *evict, char op_type, bool verbose, char *op);

int main(int argc, char * const * argv)
{
    int set_bits, lines, block_bits;
    bool verbose = false;
    const char *trace;
    cache_set_t *cache = NULL;

    static int hit = 0;
    static int miss = 0;
    static int evict = 0;

    const char *usage = "Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>";

    int opt;
    while ((opt = getopt(argc, argv, "vhs:E:b:t:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                printf("%s", usage);
                exit(EXIT_SUCCESS);

            case 'v':
                verbose = true;
                break;

            case 's':
                set_bits = atoi(optarg);
                break;

            case 'E':
                lines = atoi(optarg);
                break;

            case 'b':
                block_bits = atoi(optarg);
                break;

            case 't':
                trace = optarg;
                break;
        
            default:
                fprintf(stderr, "%s", usage);
                exit(EXIT_FAILURE);
        }
    }

    int set_size = 1 << set_bits;
    // int block_size = 1 << block_bits;

    // printf("sets : %d, lines: %d, block size: %d, verbose = %d, trace: %s\n", set_size, lines, block_size, verbose, trace);

    if (optind > argc)
    {
        exit(EXIT_FAILURE);
    }

    long set_mask = 1;
    long block_mask = 1;

    for (int i = 0; i < set_bits - 1; i++)
    {
        set_mask = (set_mask << 1) + 1;
    }
    for (int i = 0; i < block_bits - 1; i++)
    {
        block_mask = (block_mask << 1) + 1;
    }

    initCache(&cache, set_size, lines);

    FILE *stream = fopen(trace, "r");
    char line[256];
    while (fgets(line, sizeof(line), stream))
    {
        char op_type[1];
        void *addr = NULL;
        size_t op_size;
        bool is_valid = false;

        analyze_op(line, op_type, &addr, &op_size, &is_valid);

        if (is_valid)
        {
            long tag = (intptr_t)addr >> (block_bits + set_bits);
            long set = ((intptr_t)addr >> block_bits) & set_mask;
            // long block = (intptr_t)addr & block_mask;

            access_cache(cache, tag, set, &hit, &miss, &evict, *op_type, verbose, line);
        }
    }

    fclose(stream);
    freeCache(cache, set_size);
    printSummary(hit, miss, evict);
    exit(EXIT_SUCCESS);
}

static void initCache(cache_set_t **cache, size_t sets, size_t lines)
{
    *cache = (cache_set_t *)malloc(sizeof(cache_set_t) * sets);
    memset(*cache, 0, sizeof(cache_set_t) * sets);

    for (int i = 0; i < sets; i++)
    {
        (*cache + i)->cache = (cache_line_t *)malloc(sizeof(cache_line_t) * lines);
        memset((*cache + i)->cache, 0, sizeof(cache_line_t) * lines);
        (*cache + i)->lines = lines;
    }
}

static void freeCache(cache_set_t *cache, size_t sets)
{
    for (int i = 0; i < sets; i++)
    {
        free((cache + i)->cache);
    }
    free(cache);
}

static void analyze_op(const char * const operation, char * const op_type, void **addr, size_t *op_size, bool *is_valid)
{
    if (!isspace(*operation))
    {
        printf("Ignored instruction operation: %s\n", operation);
        return;
    }
    *is_valid = true;
    if (sscanf(operation, " %s %p,%lu", op_type, addr, op_size) != EOF)
    {
        // printf("type: %s address %p, size %lu\n", op_type, *addr, *op_size);
    }
}

static void access_cache(cache_set_t *cache, long tag, long set_index, int *hit, int *miss, int *evict, char op_type, bool verbose, char *op)
{
    cache_line_t *lines = (cache + set_index)->cache;
    size_t num = (cache + set_index)->lines;

    if (lines->is_dirty)
    {
        if (lines->tagbit == tag)
        {
            // hit
            *hit += 1;
            *hit += (op_type == 'M');
        }
        else if (num > 1)
        {
            for (int i = 1; i < num; i++)
            {
                if ((lines + i)->tagbit == tag)
                {
                    // hit
                    *hit += 1;
                    break;
                }
                else if (!((lines + i)->is_dirty))
                {
                    *miss += 1;
                    // cold miss
                    (lines + i)->is_dirty = true;
                    (lines + i)->tagbit = tag;
                }
            }
            // evict with LRU algorithm
        }
        else
        {
            // evict this line directly 
            lines->is_dirty = true;
            lines->tagbit = tag;

            *miss += 1;
            *evict += 1;
            *hit += (op_type == 'M');
        }
    }
    else
    {
        // cold miss 
        lines->is_dirty = true;
        lines->tagbit = tag;

        *miss += 1;
        *hit += (op_type == 'M');
    }
    if (verbose)
    {
        printf("%s hits: %d, miss: %d, evict: %d\n", op, *hit, *miss, *evict);
    }
}
