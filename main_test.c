#include "stdio.h"
#include "stdlib.h"
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
        if (reference == temp->reference) { // if the reference bit matches the one we're searching for
            // free((table->pages + i)); // free memory related to the ptr
            temp = NULL; // set the pointer to NULL
        }
        i++;
        temp = *(table->pages + i);
    }
}

void TLB_add(struct TLB * table, int reference) {
    // populate the next available space in the pages array
    // with a pointer to a page with given reference
    // struct Page *temp = *table->pages; // set indexing page to head of array
    int i = table->num_pages - 1;
    while (i > 0) {
        if (*(table->pages + i) != NULL) { // if space is unoccupied

            break;
        }
        i--;
        // temp = *(table->pages + i);
    }
    table->pages[0] = Page_constructor(reference);
}

int main(int argc, char *argv[]) {
    // we'll worry about reading from file later
    FILE *inputfile;
    inputfile = fopen("test.trace", "r"); // open file for reading
    size_t length = 10;
    char *line = NULL;
    int address;

    while (getline(&line, &length, inputfile)) { // read line until EOF
        if (feof(inputfile)) {
            break;
        }
        line[7] = '\0';
        // printf("%s\n", &line[2]);
        address = strtoul(&line[2], NULL, 16);
        printf("%d\n", address);
        if (line[0] == 'W') {

        }

    }









    // right now we need to implement:
        // TLB:
            // max number of pages
            // querying for a page
            // TLB miss and hit cases
    // struct TLB *table = TLB_constructor(8);
    // TLB_add(table, 1);
    // printf("%d\n", TLB_search(table, 1)->reference);
}
