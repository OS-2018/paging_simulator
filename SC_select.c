// second chance uses the zeroth element of TLB->bit as the indicator bit
// NOTE: reference bit is flipped from the specification in the text book
struct Page * SC_select(struct TLB *table) {
    int i = 0;
    while (table->pages[0] != NULL) {
        while (i < table->num_pages) {
            if (table->pages[i] != NULL) {
                if (table->pages[i]->bits[0] == 0) {
                    // giving it a second chance
                    table->pages[i]->bits[0] = 1;
                } else {
                    // if the page hasn't been
                    return table->pages[i];
                }
            }
            i++;
        }
        i=0;
    }
    return NULL;
}
