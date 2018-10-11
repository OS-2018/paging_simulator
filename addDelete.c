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
