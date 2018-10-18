struct Page * EARB(struct TLB * table)
{
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
    while (i < table->num_pages)
    {
        if (table->pages[i] != NULL)
        {
            //dirty pages
            if (table->pages[i]->dirty == 1)
            {
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
    struct Page * dirty_copy = Page_constructor(INT_MAX);
    struct Page * clean_copy = Page_constructor(INT_MAX);
    for (int i = 0; i < 8; i++)
    {
        dirty_copy->bits[i] = LRU_dirty->bits[i];
        clean_copy->bits[i] = LRU_clean->bits[i];
    }
    shiftRegister(clean_copy);
    shiftRegister(clean_copy);
    shiftRegister(clean_copy);
    unsigned int dirty_num = array_to_int(dirty_copy->bits);
    unsigned int clean_num = array_to_int(clean_copy->bits);
    if (dirty_num < clean_num)
    {
        return LRU_dirty;
    }
    return LRU_clean;
}
