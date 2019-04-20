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
    struct cache_line *next;
    long tagbit;
    bool is_dirty;
} cache_line_t;

typedef struct cache_set
{
    cache_line_t *cache;
    size_t lines;
} cache_set_t;

static void initCache(cache_set_t **cache, size_t sets, size_t lines);
static void initCacheLines(cache_line_t **ptr, size_t lines);
static void freeCache(cache_set_t *cache, size_t sets);
static void freeCacheLine(cache_line_t *ptr);

static void analyze_op(const char *const operation, char *const op_type, void **addr, size_t *op_size, bool *is_valid);
static void access_cache(cache_set_t *cache, long tag, long set_index, int *hit, int *miss, int *evict, char op_type, bool verbose, char *op);

static void bring_to_head(cache_line_t **head, cache_line_t *node);

int main(int argc, char *const *argv)
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
        char op_type;
        void *addr = NULL;
        size_t op_size;
        bool is_valid = false;

        analyze_op(line, &op_type, &addr, &op_size, &is_valid);

        if (is_valid)
        {
            long tag = (intptr_t)addr >> (block_bits + set_bits);
            long set = ((intptr_t)addr >> block_bits) & set_mask;

            access_cache(cache, tag, set, &hit, &miss, &evict, op_type, verbose, line);
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
        initCacheLines(&(*cache + i)->cache, lines);
        (*cache + i)->lines = lines;
    }
}

static void initCacheLines(cache_line_t **ptr, size_t lines)
{
    *ptr = (cache_line_t *)malloc(sizeof(cache_line_t));
    memset(*ptr, 0, sizeof(cache_line_t));

    cache_line_t *copy = *ptr;
    for (int i = 0; i < lines - 1; i++)
    {
        copy->next = (cache_line_t *)malloc(sizeof(cache_line_t));
        memset(copy->next, 0, sizeof(cache_line_t));
        copy = copy->next;
    }
}

static void freeCacheLine(cache_line_t *ptr)
{
    if (!ptr)
        return;
    if (ptr->next)
    {
        freeCacheLine(ptr->next);
        ptr->next = NULL;
    }
    free(ptr);
}

static void freeCache(cache_set_t *cache, size_t sets)
{
    for (int i = 0; i < sets; i++)
    {
        freeCacheLine((cache + i)->cache);
    }
    free(cache);
}

static void analyze_op(const char *const operation, char *const op_type, void **addr, size_t *op_size, bool *is_valid)
{
    if (!isspace(*operation))
    {
        printf("Ignored instruction operation: %s\n", operation);
        return;
    }

    *is_valid = true;
    if (sscanf(operation, " %c %p,%lu", op_type, addr, op_size) == 3)
    {
        printf("type: %s address %p, size %lu\n", op_type, *addr, *op_size);
    }
}

/** 
 * every time hit occurs we update our head pointer
 */
static void bring_to_head(cache_line_t **head, cache_line_t *node)
{
    cache_line_t *prev = NULL;
    for (cache_line_t *found = *head; found != node; found = found->next)
    {
        prev = found;
    }

    if (prev != NULL)
    {
        prev->next = node->next;
        node->next = *head;
        *head = node;
    }
}

static void access_cache(cache_set_t *cache, long tag, long set_index, int *hit, int *miss, int *evict, char op_type, bool verbose, char *op)
{
    printf("set index: %ld\n", set_index);
    int verbose_hits = 0;
    int verbose_misses = 0;
    int verbose_evictions = 0;

    cache_line_t *lines = (cache + set_index)->cache;
    size_t num = (cache + set_index)->lines;

    if (lines->is_dirty)
    {
        if (lines->tagbit == tag)
        {
            // hit
            *hit += 1;
            *hit += (op_type == 'M');

            verbose_hits += 1;
            verbose_hits += (op_type == 'M');
        }
        else if (num > 1)
        {
            bool need_eviction = true;
            for (int i = 1; i < num; i++)
            {
                // set associative cache
                if (lines->next->tagbit == tag)
                {
                    // hit
                    *hit += 1;
                    *hit += (op_type == 'M');

                    verbose_hits += 1;
                    verbose_hits += (op_type == 'M');
                    need_eviction = false;
                    bring_to_head(&((cache + set_index)->cache), lines->next);
                    break;
                }
                else if (!lines->next->is_dirty)
                {
                    // cold miss
                    lines->next->is_dirty = true;
                    lines->next->tagbit = tag;

                    *miss += 1;
                    *hit += (op_type == 'M');

                    verbose_misses += 1;
                    verbose_hits += (op_type == 'M');
                    need_eviction = false;
                    bring_to_head(&((cache + set_index)->cache), lines->next);
                    break;
                }
                lines = lines->next;
            }

            if (need_eviction)
            {
                // evict with LRU algorithm
                cache_line_t *head = (cache + set_index)->cache;
                while (head->next)
                {
                    head = head->next;
                }
                head->tagbit = tag;

                *miss += 1;
                *evict += 1;
                *hit += (op_type == 'M');

                verbose_misses += 1;
                verbose_hits += (op_type == 'M');
                bring_to_head(&((cache + set_index)->cache), head);
            }
        }
        else
        {
            // evict this line directly
            lines->is_dirty = true;
            lines->tagbit = tag;

            *miss += 1;
            *evict += 1;
            *hit += (op_type == 'M');

            verbose_misses += 1;
            verbose_evictions += 1;
            verbose_hits += (op_type == 'M');
        }
    }
    else
    {
        // cold miss
        lines->is_dirty = true;
        lines->tagbit = tag;

        *miss += 1;
        *hit += (op_type == 'M');

        verbose_misses += 1;
        verbose_hits += (op_type == 'M');
    }

    if (verbose)
    {
        printf("%s hits: %d, miss: %d, evict: %d\n", op, verbose_hits, verbose_misses, verbose_evictions);
    }
}