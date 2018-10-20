#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "limits.h"

//Group members:
//Charles Catt: A1726075
//Andy Yu: A1721071

//storing page information
struct Page
{
  unsigned int bits[8];
  unsigned int SCbit;
  unsigned long reference;
  int dirty;
};

//initialisation for page
struct Page * Page_constructor(unsigned long reference)
{
  //allocate memory
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

//storing TLB information
struct TLB {
    struct Page **pages;
    int num_pages;
    int counter;
};

//initialisation for TLB
struct TLB * TLB_constructor(int num_pages_given) {
    struct TLB * cache = (struct TLB*)malloc(sizeof(struct TLB));
    // allocate space for enough pointers to pages
    // calloc used to set ptrs to NULL by default
    cache->pages = calloc(num_pages_given , sizeof(struct Page*));
    cache->num_pages = num_pages_given;
    cache->counter = 0;
    return cache;
}

//search for a page within the TLB
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

//delete a page within the TLB
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

//add a page to the TLB
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

//print all pages currently in the TLB
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

//helper function for converting array of integers to integer
unsigned int array_to_int(unsigned int array[])
{
    int res = 0;
    for (int i = 0; i < 8; i++)
    {
        res = 10 * res + array[i];
    }
    return res;
}

//utility function for printing all the bits in a page
void printBits(unsigned int array[])
{
    printf("Bits: ");
    for (int i = 0; i < 8; i++)
    {
        printf("%d", array[i]);
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
  //shifting to the right once
  int temp = page->bits[7], temp1;
  for (int i = 0; i < 8; i++) {
    temp1 = page->bits[i];
    page->bits[i] = temp;
    temp = temp1;
  }
  //set reference bit to 0
  page->bits[0] = 0;
  return page;
}

//shifts all pages in the TLB
void shiftCache(struct TLB * cache)
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

//Additonal Reference Bits page replacement algorithm
struct Page * ARB(struct TLB * cache)
{
    int i = 0;
    struct Page * LRU = cache->pages[0];
    unsigned int min = array_to_int(cache->pages[0]->bits);
    unsigned int current_val;
    //find page with the minimum bits
    while (i < cache->num_pages)
    {
        if (cache->pages[i] != NULL)
        {
            //finding minimum
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

//Second Chance page replacement algorithm
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

//check if either all the pages are dirty/if all the pages are clean (1) or not (0)
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

//Enhanced Additional Reference Bits page replacement algorithm
struct Page * EARB(struct TLB * table)
{
    //if all unmodified pages or all modified pages
    if (all_clean_or_dirty(table) == 1)
    {
        return ARB(table);
    }
    //find LRU
    int i = 0;
    struct Page * LRU_dirty = NULL;
    struct Page * LRU_clean = NULL;
    unsigned int min_dirty = INT_MAX;
    unsigned int min_clean = INT_MAX;
    unsigned int current_val_dirty;
    unsigned int current_val_clean;
    //find lowest unmodified page and lowest modified page
    while (i < table->num_pages)
    {
        if (table->pages[i] != NULL)
        {
            //dirty pages
            if (table->pages[i]->dirty == 1)
            {
                //finding minimum
                current_val_dirty = array_to_int(table->pages[i]->bits);
                if (current_val_dirty < min_dirty)
                {
                    min_dirty = current_val_dirty;
                    LRU_dirty = table->pages[i];
                }
            }
            //clean pages
            else
            {
                //finding minimum
                current_val_clean = array_to_int(table->pages[i]->bits);
                if (current_val_clean < min_clean)
                {
                    min_clean = current_val_clean;
                    LRU_clean = table->pages[i];
                }
            }
        }
        i++;
    }
    //copy for shifting
    struct Page * dirty_copy = Page_constructor(INT_MAX);
    struct Page * clean_copy = Page_constructor(INT_MAX);
    for (int i = 0; i < 8; i++)
    {
        dirty_copy->bits[i] = LRU_dirty->bits[i];
        clean_copy->bits[i] = LRU_clean->bits[i];
    }
    //shift unmodified page 3 times
    shiftRegister(clean_copy);
    shiftRegister(clean_copy);
    shiftRegister(clean_copy);
    //convert to int for comparison
    unsigned int dirty_num = array_to_int(dirty_copy->bits);
    unsigned int clean_num = array_to_int(clean_copy->bits);

    //if both have never been used
    if (dirty_num == clean_num)
    {
        return LRU_clean;
    }
    //no unmodified has been used within 3 intervals, replace it
    if (dirty_num < clean_num)
    {
        return LRU_dirty;
    }
    //unmodified page has been used within 3 intervals, replace it
    return LRU_clean;
}

//Enhanced Second Chance page replacement algorithm
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
    //file for reading
    inputfile = fopen(argv[1], "r");
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
    // read line until EOF
    while (getline(&line, &length, inputfile)) {
        if (feof(inputfile)) {
            break;
        }
        //for empty/non-relevant lines
        if (line[0] == 'R' || line[0] == 'W')
        {
            //shift every interval for ARB and EARB
            if (strcmp(argv[4],"ARB") == 0 || strcmp(argv[4],"EARB") == 0)
            {
                if (i % interval == 0)
                {
                    shiftCache(cache);
                }
            }

            //calculating page reference
            address = strtoul(&line[2], NULL, 16) / page_size;

            // if this page is not in the TLB
            if (TLB_search(cache, address) == NULL)
            {
                //read from disk when it is not in the TLB
                read_counter++;
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
                    //write from disk when you replace a dirty page
                    if (replacedPage->dirty == 1)
                    {
                        write_counter++;
                    }
                    if (strcmp(argv[4],"SC") == 0 || strcmp(argv[4],"ESC") == 0) {
                        TLB_delete(cache, replacedPage->reference, 0);
                    } else {
                        TLB_delete(cache, replacedPage->reference, 1);
                    }

                }
                // add page to TLB
                TLB_add(cache, address);
                if (line[0] == 'W') {
                    TLB_search(cache, address)->dirty = 1;
                }
                TLB_search(cache, address)->bits[0] = 1;
            }
            else
            {
                //page is already in TLB, set reference bit to 1 (used)
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
