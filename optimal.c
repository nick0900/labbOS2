#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

typedef struct Page 
    {
        unsigned int lower;
        unsigned int upper;
        struct Page *next;
        unsigned long lable;
    }Page;

unsigned long getNextAcc(FILE *f, Page *ref, unsigned long tick);

int main(int argc, char **argv)
{
    unsigned long ticker = 0;

    unsigned int qSize = 0;
    Page *in = NULL;
    Page *out = NULL;
    long noPages;
    long pageSize;
    FILE *f;
    FILE *fHelp;
    int traceCount = 0;
    int pagefaults = 0;
    char line[32];
    unsigned int trace;

    if (argc != 4)
    {
        printf("Error- faulty number of arguments\n");
        return 0;
    }

    noPages = strtol(argv[1], NULL, 10);
    pageSize = strtol(argv[2], NULL, 10);

    printf("No physical pages = %ld, page size = %ld\n", noPages, pageSize);

    f = fopen(argv[3], "r");
    fHelp = fopen(argv[3], "r");

    if (f == NULL)
    {
        printf("failed to open %s\n", argv[3]);
        return 0;
    }

    printf("Reading memory trace from %s\n", argv[3]);

    while (fgets(line, sizeof(line), f))
    {
        trace = (unsigned int) strtol(line, NULL, 10);

        traceCount++;

        int pageMatch = 0;

        fseek(fHelp, ftell(f), SEEK_SET);

        Page *next = in;
        while (next != NULL)
        {
            if ((trace >= next->lower) && (trace < next->upper))
            {
                pageMatch = 1;
                next->lable = getNextAcc(fHelp, next, ticker);
                break;
            }
            next = next->next;
        }

        if (pageMatch == 0)
        {
            pagefaults++;
            if (qSize < noPages)
            {
                qSize++;
                if (in == NULL)
                {
                    in = malloc(sizeof(struct Page));
                    out = in;

                    in->next = NULL;
                    in->lower = floor(trace / pageSize) * pageSize;
                    in->upper = in->lower + pageSize;
                    in->lable = getNextAcc(fHelp, in, ticker);
                }
                else
                {
                    Page* temp = malloc(sizeof(struct Page));

                    temp->next = in;
                    temp->lower = floor(trace / pageSize) * pageSize;
                    temp->upper = temp->lower + pageSize;
                    temp->lable = getNextAcc(fHelp, temp, ticker);

                    in = temp;
                }
            }
            else
            {
                Page *leastUsed = in;
                Page *walker = in;
                while (walker != NULL)
                {
                    if (walker->lable - ticker > leastUsed->lable - ticker)
                    {
                        leastUsed = walker;
                    }
                    walker = walker->next;
                }

                leastUsed->lower = floor(trace / pageSize) * pageSize;
                leastUsed->upper = leastUsed->lower + pageSize;
                leastUsed->lable = getNextAcc(fHelp, leastUsed, ticker);
            }
        }
        ticker++;
    }

    printf("Read %d memory references => %d pagefaults\n", traceCount, pagefaults);

    while (in != NULL)
    {
        out = in;
        in = in->next;

        free(out);
    } 
    

    fclose(f);
    fclose(fHelp);

    return 1;
}

unsigned long getNextAcc(FILE *f, Page *ref, unsigned long tick)
{
    char line[32];
    unsigned int trace;

    while (fgets(line, sizeof(line), f))
    {
        trace = (unsigned int) strtol(line, NULL, 10);

        tick++;

        if ((trace >= ref->lower) && (trace < ref->upper))
        {
            break;
        }
    }

    return tick;
}