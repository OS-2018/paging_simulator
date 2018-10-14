#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct Page
{
  unsigned int bits[8];
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
  page->dirty = 0;
  page->reference = reference;
  return page;
}

struct TLB {
    struct Page **pages;
    int num_pages;
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
            table->pages[i] = shiftRegister(table->pages[i]);
        }
        i++;
    }
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

int main(int argc, char *argv[]) {
    // we'll worry about reading from file later
    FILE *inputfile;
    inputfile = fopen("input3.trace", "r"); //file for reading
    size_t length = 10;
    char *line = NULL;
    int address;
    int read_counter = 0;
    int write_counter = 0;
    int interval = 0;
    //if interval is given
    if (argc == 5)
    {
        interval = atoi(argv[4]);
    }

    struct TLB * cache = TLB_constructor(atoi(argv[2]));

    int i = 0;
    while (getline(&line, &length, inputfile)) { // read line until EOF
        if (feof(inputfile)) {
            break;
        }
        if (line[0] == 'R' || line[0] == 'W')
        {
            if (strcmp(argv[3],"ARB") == 0 || strcmp(argv[3],"EARB") == 0)
            {
                if (i % interval == 0)
                {
                    shiftTable(cache);
                }
            }
            line[7] = '\0';
            // printf("%s\n", &line[2]);
            address = strtoul(&line[2], NULL, 16);
            // printf("%d\n", address);
            //int phys_page_num;
            if (TLB_search(cache, address) == NULL) // if this page is not in the TLB
            {
                if (isFull(cache) == 1)
                {
                    //page replacement algorithm
                    printf("REPLACE:  page %d", address);
                    struct Page * replacedPage;
                    if (strcmp(argv[3],"ARB") == 0)
                    {
                        replacedPage = ARB(cache);
                    } else if (strcmp(argv[3],"SC")) {}
                    // TLB_delete(cache, SC_delete->reference);
                    if (replacedPage->dirty == 1)
                    {
                        printf(" (DIRTY)");
                        write_counter++;
                    }
                    printf("\n");
                    TLB_delete(cache, replacedPage->reference);

                }
                // add it in
                TLB_add(cache, address);
                if (line[0] == 'W') {
                    TLB_search(cache, address)->dirty = 1;
                }
                read_counter++;
                printf("MISS:  page %d\n", address);
                //add tag and physical page number to TLB
            }
            else
            {
                printf("HIT:  page %d\n", address);
                struct Page * currentPage = TLB_search(cache, address);
                currentPage->bits[0] = 1;
            }
            //create physical address from physical page number and page offset


        }
        i++;
        // TLB_print(cache);

    }

    printf("events in trace:  %d\n", i);
    printf("total disk reads:  %d\n", read_counter);
    printf("total disk reads:  %d\n", write_counter);

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
