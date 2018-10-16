
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

// second chance uses the zeroth element of TLB->bit as the indicator bit
// CHANGED: using SCbit now
struct Page * SC_select(struct TLB *table) {
    while (table->pages[0] != NULL) {
        while (table->counter < table->num_pages) {
            if (table->pages[table->counter] != NULL) {
                if (table->pages[table->counter]->SCbit == 0) {
                    // giving it a second chance
                    table->pages[table->counter]->SCbit = 1;
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
