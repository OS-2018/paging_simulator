#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "limits.h"

struct Page
{
  unsigned int bits[8];
  unsigned int SCbit;
  int reference;
  int dirty;
};

struct Page * Page_constructor(int reference)
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
    struct TLB * table = (struct TLB*)malloc(sizeof(struct TLB));
    table->pages = calloc(num_pages_given , sizeof(struct Page**)); // allocate space for enough pointers to pages
    // struct Page *temp = table->pages;
    // for (int i = 0; i < num_pages_given; i++)
    // {
    //   (table->pages + 1) = NULL;
    //   temp = (table->pages + i);
    // }
    table->num_pages = num_pages_given;
    table->counter = 0;
    return table;
}

struct Page * TLB_search(struct TLB *table, int given_reference) {
    int i = 0;
    struct Page *temp = *table->pages; // set indexing page to head of array
    while (i < table->num_pages) {
        if (temp != NULL) {
            if (given_reference == temp->reference) { // if the reference bit matches the one we're searching for
                return temp; // return the page associated with given reference
            }
        }
        i++;
        temp = *(table->pages + i);
    }
    return NULL; // return null if page is not in the TLB
}

void TLB_delete(struct TLB * table, int reference) {
    // we search for a page with mathich reference and delete it.
    int i = 0;
    struct Page *temp = *table->pages; // set indexing page to head of array
    while (i < table->num_pages) {
        if (table->pages[i] != NULL) {
            if (reference == table->pages[i]->reference) { // if the reference bit matches the one we're searching for
            // free((table->pages + i)); // free memory related to the ptr
            // table->pages[i] = NULL; // set the pointer to NULL
            break;
            }
        }
        i++;
    }
    while (i < table->num_pages - 1) {
        table->pages[i] = table->pages[i+1];
        i++;
    }
    table->pages[i] = NULL;
}

void TLB_add(struct TLB * table, int reference) {
    int i = 0; // start at end of array
    while (i < table->num_pages) {
        if (*(table->pages + i) == NULL) { // if space is not occupied, add it in
            table->pages[i] = Page_constructor(reference);
            break;
        }
        i++;
    }
}

void TLB_print(struct TLB *table) {
    int i = 0;
    while (i < table->num_pages) {
        if (*(table->pages + i) == NULL) {
            printf("NULL\n");
        } else {
            printf("%d\n", (*(table->pages + i))->reference);
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

void TLB_print_bits(struct TLB *table) {
    int i = 0;
    while (i < table->num_pages) {
        if (*(table->pages + i) == NULL) {
            printf("NULL\n");
        } else {
            printf("    Page: %d    ", table->pages[i]->reference);
            printBits(table->pages[i]->bits);
            printf("    Dirty: %d\n", table->pages[i]->dirty);
        }
        i++;
    }
}


//1 if full, 0 if not full
int isFull(struct TLB * table)
{
  if (table->pages[table->num_pages - 1] != NULL)
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

void shiftTable(struct TLB * table)
{
    int i = 0;
    while (i < table->num_pages)
    {
        if (table->pages[i] != NULL)
        {
            //printBits(table->pages[i]->bits);
            table->pages[i] = shiftRegister(table->pages[i]);
            //printBits(table->pages[i]->bits);
        }
        i++;
    }
}

struct Page * ARB(struct TLB * table)
{
    int i = 0;
    struct Page * LRU = table->pages[0];
    unsigned int min = array_to_int(table->pages[0]->bits);
    unsigned int current_val;
    while (i < table->num_pages)
    {
        if (table->pages[i] != NULL)
        {
            current_val = array_to_int(table->pages[i]->bits);
            // printf("current val: %d\n", current_val);
            // printf("min%d\n", min);
            if (current_val < min)
            {
                min = current_val;
                //printf("min changed: %d\n", min);
                LRU = table->pages[i];
                //return table->pages[i];
            }
        }
        i++;
    }
    return LRU;
}

// second chance uses the zeroth element of TLB->bit as the indicator bit
// CHANGED: using SCbit now
struct Page * SC_select(struct TLB *table) {
    while (table->pages[0] != NULL) {
        while (table->counter < table->num_pages) {
            if (table->pages[table->counter] != NULL) {
                if (table->pages[table->counter]->SCbit == 1) {
                    // giving it a second chance
                    table->pages[table->counter]->SCbit = 0;
                } else {
                    // choose the FIFO page that HAS been given second chance
                    return table->pages[table->counter];
                }
            }
            table->counter++;
        }
        table->counter=0;
    }
    return NULL;
}

int all_clean_or_dirty(struct TLB * table)
{
    int i = 0;
    int clean = 0;
    int dirty = 0;
    while (i < table->num_pages)
    {
        if (table->pages[i]->dirty == 1)
        {
            dirty++;
        }
        else
        {
            clean++;
        }
        i++;
    }
    if (clean == table->num_pages || dirty == table->num_pages)
    {
        return 1;
    }
    return 0;
}

struct Page * EARB(struct TLB * table)
{
    if (all_clean_or_dirty(table) == 1)
    {
        return ARB(table);
    }
    //find LRU
    int i = 0;
    struct Page * LRU = table->pages[0];
    unsigned int min = array_to_int(table->pages[0]->bits);
    unsigned int current_val;
    while (i < table->num_pages)
    {
        if (table->pages[i] != NULL)
        {
            current_val = array_to_int(table->pages[i]->bits);
            if (current_val < min)
            {
                min = current_val;
                LRU = table->pages[i];
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
        struct Page * LRU_clean = table->pages[0];
        unsigned int min = INT_MAX;
        unsigned int current_val;
        while (i < table->num_pages)
        {
            if (table->pages[i] != NULL)
            {
                if (table->pages[i]->dirty == 0)
                {
                    current_val = array_to_int(table->pages[i]->bits);
                    if (current_val < min)
                    {
                        min = current_val;
                        LRU_clean = table->pages[i];
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
                return cache->pages[cache->counter];
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
                return cache->pages[cache->counter];
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
    // we'll worry about reading from file later
    FILE *inputfile;
    inputfile = fopen(argv[1], "r"); //file for reading
    size_t length = 10;
    char *line = NULL;
    int address;
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
                    // printf("WE ARE SHIFTING\n");
                    // TLB_print_bits(cache);
                    // printf("~~~~~~~~~~~~~~~\n");
                    shiftTable(cache);
                    // TLB_print_bits(cache);
                }
            }
            // line[10 - (int )ceil((log(atoi(argv[2]))/log(16)))] = '\0';
            line[7] = '\0';
            // printf("%s\n", &line[2]);
            address = strtoul(&line[2], NULL, 16);
            // printf("%d\n", address);
            //int phys_page_num;
            if (TLB_search(cache, address) == NULL) // if this page is not in the TLB
            {
                read_counter++;
                // printf("MISS:    page %d\n", address);
                if (isFull(cache) == 1)
                {
                    //page replacement algorithm
                    struct Page * replacedPage;
                    if (strcmp(argv[4],"ARB") == 0)
                    {
                        replacedPage = ARB(cache);
                    } else if (strcmp(argv[4],"SC") == 0) {
                        replacedPage = SC_select(cache);
                    } else if (strcmp(argv[4],"EARB") == 0) {
                        replacedPage = EARB(cache);
                    } else if (strcmp(argv[4],"ESC") == 0) {
                        replacedPage = ESC_select(cache);
                    }
                    // TLB_delete(cache, SC_delete->reference);
                    // printf("REPLACE: page %d", replacedPage->reference);
                    if (replacedPage->dirty == 1)
                    {
                        // printf(" (DIRTY)");
                        write_counter++;
                    }
                    // printf("\n");
                    TLB_delete(cache, replacedPage->reference);
                }
                // add it in
                TLB_add(cache, address);
                if (line[0] == 'W') {
                    TLB_search(cache, address)->dirty = 1;
                }
                //add tag and physical page number to TLB
                TLB_search(cache, address)->bits[0] = 1;
                // struct Page * currentPage = TLB_search(cache, address);
                // currentPage->bits[0] = 1;
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
            //create physical address from physical page number and page offset

            i++;
        }
        // TLB_print(cache);

    }

    printf("events in trace:    %d\n", i);
    printf("total disk reads:   %d\n", read_counter);
    printf("total disk writes:  %d\n", write_counter);

    //TLB_print(cache);

    // right now we need to implement:
        // TLB:
            // max number of pages
            // querying for a page
            // TLB miss and hit cases
    // struct TLB *table = TLB_constructor(8);
    // TLB_add(table, 1);
    // printf("%d\n", TLB_search(table, 1)->reference);
}
