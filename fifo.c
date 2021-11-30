#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

int main(int argc, char **argv)
{
    typedef struct Page 
    {
        unsigned int lower;
        unsigned int upper;
        struct Page *next;
    }Page;

    unsigned int qSize = 0;
    Page *in = NULL;
    Page *out = NULL;
    long noPages;
    long pageSize;
    FILE *f;
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
        
        Page *next = in;
        while (next != NULL)
        {
            if ((trace >= next->lower) && (trace < next->upper))
            {
                pageMatch = 1;
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
                }
                else
                {
                    Page* temp = malloc(sizeof(struct Page));

                    temp->next = in;
                    temp->lower = floor(trace / pageSize) * pageSize;
                    temp->upper = temp->lower + pageSize;

                    in = temp;
                }
            }
            else
            {
                out->lower = floor(trace / pageSize) * pageSize;
                out->upper = out->lower + pageSize;
                out->next = in;
                in = out;

                do
                {
                    out = out->next;
                } while (out->next != in);

                out->next = NULL;
            }
        }
    }

    printf("Read %d memory references => %d pagefaults\n", traceCount, pagefaults);

    while (in != NULL)
    {
        out = in;
        in = in->next;

        free(out);
    } 
    

    fclose(f);

    return 1;
}