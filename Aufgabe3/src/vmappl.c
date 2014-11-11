/* Demo application for virtual memory management model
 * File: vmappl.c
 * Prof. Dr. Wolfgang Fohl, HAW Hamburg
 * 2010
 */

#include "vmappl.h"

int
main(void)
{
    /* Fill memory with pseudo-random data */
    init_data(LENGTH);

    /* Display unsorted */
    printf("\nUnsorted:\n");
    display_data(LENGTH);

    /* Sort */
    printf("\nSorting:\n");
    sort(LENGTH);

    /* Display sorted */
    printf("\nSorted:\n");
    display_data(LENGTH);
    printf("\n");

    return 0;
}

void
init_data(int length)
{
    int i;
    int val;

    /* Init random generator */
    srand(SEED);

    for(i = 0; i < length; i++) {
        val = rand() % RNDMOD;
        vmem_write(i, val);
    }   /* end for */
}

void
display_data(int length)
{
    int i;
    for(i = 0; i < length; i++) {
        printf("%10d", vmem_read(i));
        printf("%c", ((i + 1) % NDISPLAYCOLS) ? ' ' : '\n');
    }   /* end for */
}

void
sort(int length)
{
    /* Quicksort */
    quicksort(0, length - 1);
}

void
quicksort(int l, int r)
{
    if(l < r) {
        int i = l;
        int j = r-1;
        /* Put all elements < [r] to the left */
        while(1) { 
            while( (i <= r) && (vmem_read(i) < vmem_read(r)) ) { i++; }
            while( (j >= l) && (vmem_read(j) > vmem_read(r)) ) { j--; }
            if(i >= j) {
                break;
            }   
            swap(i, j);
	    i++; j--;
        }   /* end while */
        swap(i, r);     /* Put reference element to the boundary */
        /* Recursively sort the left and right half */
        quicksort(l, i - 1);
        quicksort(i + 1, r);
    }   /* end if */
}

void
swap(int addr1, int addr2)
{
    int tmp = vmem_read(addr1);
    vmem_write(addr1, vmem_read(addr2));
    vmem_write(addr2, tmp);
}
