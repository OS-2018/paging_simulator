#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Page
{
  unsigned int bits[8];
};

struct Page * createPage()
{
  struct Page * page = (struct Page*)malloc(sizeof(struct Page));
  for (int i = 0; i < 8; i++)
  {
    page->bits[i] = 0;
  }
  return page;
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

void printPage (struct Page * page)
{
  for (int i = 0; i < 8; i++)
  {
    printf("%d\n", page->bits[i]);
  }
}

struct Page * ARB(struct Page * newPage)
{
  int numPages = 100;
  int interval = 5;
  int referenceBit = 8;

  //testing//
  // newPage->bits[0] = 1;
  // newPage->bits[7] = 5;
  // shiftRegister(newPage, referenceBit);
  // printPage(newPage);


  int pageIndex = 0;
  int pageUsed = 1;
  struct Page * LRUPage = createPage();

  //placeholder
  struct Page * oldPage = createPage();
  //one loop = one time period?
  for (int i = 0; i < numPages; i++)
  {
    if (pageIndex == 8)
    {
      pageIndex = 0;
    }
    //what determines which page to use?
    if (i % interval == 0)
    {
       shiftRegister(newPage, referenceBit);
    }
    //if page is used, flip bit to 1
    if (pageUsed == 1)
    {
        newPage->bits[pageIndex] = 1;
    }
    //check LRU page
    if (newPage < oldPage)
    {
        LRUPage = newPage;
    }
  }

  printf("The page to be replaced is:\n");
  printPage(newPage);
  return LRUPage;
}

int main()
{
  struct Page * newPage = createPage();

  ARB(newPage);

  return 0;
}
