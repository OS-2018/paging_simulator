#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "limits.h"

//Group members:
//Charles Catt: A1726075
//Andy Yu: A1721071

struct Page
{
  unsigned int bits[8];
  unsigned int SCbit;
  unsigned long reference;
  int dirty;
};

struct Page * Page_constructor(unsigned long reference)
{
  struct Page * page = (struct Page*)calloc(1,sizeof(struct Page));
  for (int i = 0; i < 8; i++)
  {
    page->bits[i] = 0;
  }
  page->SCbit = 1;
  page->dirty = 0;
  page->reference = reference;
  return page;
}

struct TLB {
    struct Page **pages;
    int num_pages;
    int counter;
};

struct TLB * TLB_constructor(int num_pages_given) {
    struct TLB * cache = (struct TLB*)malloc(sizeof(struct TLB));
    // allocate space for enough pointers to pages
    // calloc used to set ptrs to NULL by default
    cache->pages = calloc(num_pages_given , sizeof(struct Page*));
    cache->num_pages = num_pages_given;
    cache->counter = 0;
    return cache;
}

struct Page * TLB_search(struct TLB *cache, unsigned long given_reference) {
    int i = 0;
    while (i < cache->num_pages) {
        if (cache->pages[i] != NULL) {
            // if the reference bit matches the one we're searching for
            if (given_reference == cache->pages[i]->reference) {
                // return the page associated with given reference
                return cache->pages[i];
            }
        }
        i++;
    }
    // return null if page is not in the TLB
    return NULL;
}

void TLB_delete(struct TLB * cache, unsigned long reference, int shift) {
    // we search for a page with matching reference and delete it.
    int i = 0;
    while (i < cache->num_pages) {
        if (cache->pages[i] != NULL) {
            // if the reference bit matches the one we're searching for
            if (reference == cache->pages[i]->reference) {
                break;
            }
        }
        i++;
    }
    // free the associated memory
    free(cache->pages[i]);
    // set the ptr to null
    cache->pages[i] = NULL;

    // if necessary, go through the rest of the array, updating the pointers
    if (shift) {
        while (i < cache->num_pages - 1) {
            cache->pages[i] = cache->pages[i+1];
            i++;
        }
        cache->pages[i] = NULL;
    }
}

void TLB_add(struct TLB * cache, unsigned long reference) {
    int i = 0;
    while (i < cache->num_pages) {
        // if space is not occupied, add it in
        if (cache->pages[i] == NULL) {
            cache->pages[i] = Page_constructor(reference);
            break;
        }
        i++;
    }
}

void TLB_print(struct TLB *cache) {
    int i = 0;
    while (i < cache->num_pages) {
        if (*(cache->pages + i) == NULL) {
            printf("NULL\n");
        } else {
            printf("%lu\n", (*(cache->pages + i))->reference);
        }
        i++;
    }
    printf("~~~~~~~~~~~\n");

}

unsigned int array_to_int(unsigned int array[])
{
    int res = 0;
    for (int i = 0; i < 8; i++)
    {
        res = 10 * res + array[i];
    }
    return res;
}

void printBits(unsigned int array[])
{
    printf("Bits: ");
    for (int i = 0; i < 8; i++)
    {
        printf("%d", array[i]);
    }

}

void TLB_print_bits(struct TLB *cache) {
    int i = 0;
    while (i < cache->num_pages) {
        if (*(cache->pages + i) == NULL) {
            printf("NULL\n");
        } else {
            printf("    Page: %lu    ", cache->pages[i]->reference);
            printBits(cache->pages[i]->bits);
            printf("    Dirty: %d\n", cache->pages[i]->dirty);
        }
        i++;
    }
}


//1 if full, 0 if not full
int isFull(struct TLB * cache)
{
  if (cache->pages[cache->num_pages - 1] != NULL)
  {
    return 1;
  }
  return 0;
}

//reference bit to high order bit and discard low order bit
struct Page * shiftRegister(struct Page * page)
{
  int temp = page->bits[7], temp1;
  for (int i = 0; i < 8; i++) {
    temp1 = page->bits[i];
    page->bits[i] = temp;
    temp = temp1;
  }
  page->bits[0] = 0;
  return page;
}

void shiftcache(struct TLB * cache)
{
    int i = 0;
    while (i < cache->num_pages)
    {
        if (cache->pages[i] != NULL)
        {
            cache->pages[i] = shiftRegister(cache->pages[i]);
        }
        i++;
    }
}

struct Page * ARB(struct TLB * cache)
{
    int i = 0;
    struct Page * LRU = cache->pages[0];
    unsigned int min = array_to_int(cache->pages[0]->bits);
    unsigned int current_val;
    while (i < cache->num_pages)
    {
        if (cache->pages[i] != NULL)
        {
            current_val = array_to_int(cache->pages[i]->bits);
            if (current_val < min)
            {
                min = current_val;
                LRU = cache->pages[i];
            }
        }
        i++;
    }
    return LRU;
}

struct Page * SC_select(struct TLB *cache) {
    while (1) {
        if (cache->pages[cache->counter]->SCbit == 1) {
            // giving it a second chance
            cache->pages[cache->counter]->SCbit = 0;
        } else {
            // choose the FIFO page that has been given second chance
            int temp = cache->counter;
            cache->counter++;
            if (cache->counter == cache->num_pages) {
                cache->counter = 0;
            }
            return cache->pages[temp];
        }
        cache->counter++;
        if (cache->counter == cache->num_pages) {
            cache->counter = 0;
        }
    }
    return NULL;
}

int all_clean_or_dirty(struct TLB * cache)
{
    int i = 0;
    int clean = 0;
    int dirty = 0;
    while (i < cache->num_pages)
    {
        if (cache->pages[i]->dirty == 1)
        {
            dirty++;
        }
        else
        {
            clean++;
        }
        i++;
    }
    if (clean == cache->num_pages || dirty == cache->num_pages)
    {
        return 1;
    }
    return 0;
}

struct Page * EARB(struct TLB * cache)
{
    if (all_clean_or_dirty(cache) == 1)
    {
        return ARB(cache);
    }
    //find LRU
    int i = 0;
    struct Page * LRU = cache->pages[0];
    unsigned int min = array_to_int(cache->pages[0]->bits);
    unsigned int current_val;
    while (i < cache->num_pages)
    {
        if (cache->pages[i] != NULL)
        {
            current_val = array_to_int(cache->pages[i]->bits);
            if (current_val < min)
            {
                min = current_val;
                LRU = cache->pages[i];
            }
        }
        i++;
    }
    //if the LRU has been modified
    if (LRU->dirty == 1)
    {
        int dirty_index = -1;
        //find the interval at which the modified page was last used.
        for (int i = 0; i < 8; i++)
        {
            if (LRU->bits[i] == 1)
            {
                dirty_index = i;
                break;
            }
        }
        //find LRU of clean pages
        int i = 0;
        struct Page * LRU_clean = cache->pages[0];
        unsigned int min = INT_MAX;
        unsigned int current_val;
        while (i < cache->num_pages)
        {
            if (cache->pages[i] != NULL)
            {
                if (cache->pages[i]->dirty == 0)
                {
                    current_val = array_to_int(cache->pages[i]->bits);
                    if (current_val < min)
                    {
                        min = current_val;
                        LRU_clean = cache->pages[i];
                    }
                }
            }
            i++;
        }
        if (dirty_index == -1)
        {
            return LRU_clean;
        }
        else
        {
            //check intervals
            int clean_index = -1;
            for (int i = 0; i < 8; i++)
            {
                if (LRU_clean->bits[i] == 1)
                {
                    clean_index = i;
                    break;
                }
            }
            if (clean_index == -1)
            {
                return LRU_clean;
            }
            if (dirty_index - clean_index <= 3)
            {
                return LRU_clean;
            }
            else
            {
                return LRU;
            }
        }



    }
    return LRU;
}

struct Page * ESC_select(struct TLB *cache) {
    int i = 0;
    while (1) {
        // search through buffer for (0,0) pages
        while (i < cache->num_pages) {
            if (cache->pages[cache->counter]->SCbit == 0 && cache->pages[cache->counter]->dirty == 0) {
                int temp = cache->counter;
                cache->counter++;
                if (cache->counter == cache->num_pages) {
                    cache->counter = 0;
                }
                return cache->pages[temp];
            }
            i++;
            cache->counter++;
            if (cache->counter == cache->num_pages) {
                cache->counter = 0;
            }
        }
        i = 0;
        // search through buffer for (0,1) pages
        while (i < cache->num_pages) {
            if (cache->pages[cache->counter]->SCbit == 0 && cache->pages[cache->counter]->dirty == 1) {
                int temp = cache->counter;
                cache->counter++;
                if (cache->counter == cache->num_pages) {
                    cache->counter = 0;
                }
                return cache->pages[temp];
            }
            cache->pages[cache->counter]->SCbit = 0;
            i++;
            cache->counter++;
            if (cache->counter == cache->num_pages) {
                cache->counter = 0;
            }
        }
        i = 0;
    }
}


int main(int argc, char *argv[]) {
    unsigned long page_size = strtoul(argv[2], NULL, 10);
    FILE *inputfile;
    inputfile = fopen(argv[1], "r"); //file for reading
    size_t length = 10;
    char *line = NULL;
    unsigned long address;
    int read_counter = 0;
    int write_counter = 0;
    int interval = 0;
    //if interval is given
    if (argc == 6)
    {
        interval = atoi(argv[5]);
    }

    struct TLB * cache = TLB_constructor(atoi(argv[3]));

    int i = 0;

    while (getline(&line, &length, inputfile)) { // read line until EOF
        if (feof(inputfile)) {
            break;
        }
        if (line[0] == 'R' || line[0] == 'W')
        {
            if (strcmp(argv[4],"ARB") == 0 || strcmp(argv[4],"EARB") == 0)
            {
                if (i % interval == 0)
                {
                    shiftcache(cache);
                }
            }

            address = strtoul(&line[2], NULL, 16) / page_size;

            // if this page is not in the TLB
            if (TLB_search(cache, address) == NULL)
            {
                read_counter++;
                // printf("MISS:    page %d\n", address);
                if (isFull(cache) == 1)
                {
                    //page replacement algorithm
                    struct Page * replacedPage;
                    if (strcmp(argv[4],"ARB") == 0) {
                        replacedPage = ARB(cache);
                    } else if (strcmp(argv[4],"SC") == 0) {
                        replacedPage = SC_select(cache);
                    } else if (strcmp(argv[4],"EARB") == 0) {
                        replacedPage = EARB(cache);
                    } else if (strcmp(argv[4],"ESC") == 0) {
                        replacedPage = ESC_select(cache);

                    }
                    // printf("REPLACE: page %d", replacedPage->reference);
                    if (replacedPage->dirty == 1)
                    {
                        // printf(" (DIRTY)");
                        write_counter++;
                    }
                    // printf("\n");
                    if (strcmp(argv[4],"SC") == 0 || strcmp(argv[4],"ESC") == 0) {
                        TLB_delete(cache, replacedPage->reference, 0);
                    } else {
                        TLB_delete(cache, replacedPage->reference, 1);
                    }

                }
                // add it in
                TLB_add(cache, address);
                if (line[0] == 'W') {
                    TLB_search(cache, address)->dirty = 1;
                }
                TLB_search(cache, address)->bits[0] = 1;
            }
            else
            {
                // printf("HIT:     page %d\n", address);
                struct Page * currentPage = TLB_search(cache, address);
                currentPage->bits[0] = 1;
                currentPage->SCbit = 1;
                if (line[0] == 'W') {
                    TLB_search(cache, address)->dirty = 1;
                }
            }
            i++;
        }

    }

    printf("events in trace:    %d\n", i);
    printf("total disk reads:   %d\n", read_counter);
    printf("total disk writes:  %d\n", write_counter);

}
