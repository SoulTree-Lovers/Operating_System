#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "slab.h"

#define TESTSIZE 2048 
#define TESTSLABID 8
#define MAXTEST 200 

/* 
NOTE:
We will grade your submission based on the original slabtest() codes as follows.
Thereby, you should pass all tests in the slabtest() function as is.
Note that, you can edit the test function only for the debugging purpose. 
*/
int* t[NSLAB][MAXTEST] = {};

void slabtest(){
	int counter = 1;
	int start;
	int pass;
	int* tmp;

	/* NOTE: 
	 * The current version of xv6 contains an error on aligning memory segments of the kernel,
	 * especially due to the literals of this source code. 
	 * More specifically, the '.stab' section should be aligned 
	 * to locate the VMA (virtual memory address) of an multiple of 4 bytes. 
	 * You can check the VMA of .stab by executing 'objdump -h kernel' in your xv6 source dir.
	 * If the VMA is not aligned to the multiple of 4B, then adjust (add/del chars) 
	 * the literal string in the cprintf function
	 */
	cprintf("====   SLAB TEST   ====\n");
	
	slabdump();
	/* TIPS:
	 *	You may debug your result with 
	 * cprintf();
	 * slabdump();
	 */

	/* TEST1: Single slab alloc */
	cprintf("==== TEST1 =====\n");
	start = counter;
	t[0][0] = (int*) kmalloc (TESTSIZE); 
	*(t[0][0]) = counter;
	counter++;
	
	slabdump();

	cprintf( (*(t[0][0]) == start && numobj_slab(TESTSLABID) == 1) ? "OK\n":"WRONG\n");
	kmfree ((char*) t[0][0], TESTSIZE);

	
	slabdump();
	
	/* TEST2: Single slab alloc: the size not equal to a power of 2. */
	cprintf("==== TEST2 =====\n");
	start = counter;
	t[0][0] = (int*) kmalloc (TESTSIZE-10); 
	*(t[0][0]) = counter;
	counter++;

	slabdump();

	cprintf( (*(t[0][0]) == start && numobj_slab(TESTSLABID) == 1) ? "OK\n":"WRONG\n");
	kmfree ((char*) t[0][0], TESTSIZE);

	
	slabdump();

	/* TEST3: Multiple slabs alloc */
	cprintf("==== TEST3 =====\n");
	start = counter;
	for (int i=0; i<NSLAB; i++)
	{
		int slabsize = 1 << (i+3); 
		t[i][0]	= (int*) kmalloc (slabsize); 
		for (int j=0; j<slabsize/sizeof(int); j++)
		{
			memmove (t[i][0]+j, &counter, sizeof(int));
			counter++;
		}
	}
	


	// CHECK 
	pass = 1;
	for (int i=0; i<NSLAB; i++)
	{
		int slabsize = 1 << (i+3); 
		for (int j=0; j < slabsize/sizeof(int); j++)
		{
//			cprintf("i=%d, j=%d, *(t[i][j]+k)=%d, start=%d\n", i, j, *(t[i][0]+j), start);		//YOU MAY USE THIS
			if ( *(t[i][0]+j) != start )
			{
				pass = 0;
				break;
			}
			start++;
		}
	}

	slabdump();

	cprintf( pass ? "OK\n" : "WRONG\n");	
	
	
	
	for (int i=0; i<NSLAB; i++)
	{
		int slabsize = 1 << (i+3); 
		kmfree((char*) t[i][0], slabsize);
	}

	slabdump();

	/* TEST4: Multiple slabs alloc2 */
	cprintf("==== TEST4 =====\n");
	start = counter;
	for (int i=0; i<NSLAB; i++) // i: 각 slab index
	{
		int slabsize = 1 << (i+3); 
		for (int j=0; j<MAXTEST; j++) // j: 200개 object 생성
		{
			t[i][j]	= (int*) kmalloc (slabsize); 
			for (int k=0; k<slabsize/sizeof(int); k++) 
			{
				memmove (t[i][j]+k, &counter, sizeof(int));
				counter++; // 총 slab object의 개수 추가
			}
		}
	}
	
	// CHECK
	pass = 1;
	for (int i=0; i<NSLAB; i++)
	{
		int slabsize = 1 << (i+3); 
		for (int j=0; j<MAXTEST; j++)
		{
			for (int k=0; k<slabsize/sizeof(int); k++)
			{
//				for (int l=0; l<slabsize/sizeof(int); l++)
//				cprintf("i=%d, j=%d, *(t[i][j]+k)=%d, start=%d\n", i, j, *(t[i][j]+k), start);
				if (*(t[i][j]+k) != start)
				{
					pass = 0;
					
					//cprintf("%d, %d, %d, %d\n", i, j, *(t[i][0]+j), start);		//YOU MAY USE THIS
					
					break;
				}
				start++;
				
			}
			
		}
	}

	slabdump();

	cprintf( pass ? "OK\n" : "WRONG\n");	

	for (int i=0; i<NSLAB; i++)
	{
		int slabsize = 1 << (i+3); 
		for (int j=0; j<MAXTEST; j++)
			kmfree((char*) t[i][j], slabsize);
	}

	slabdump();

	/* TEST5: ALLOC MORE THAN 100 PAGES */
	cprintf("==== TEST5 =====\n");
	start = counter;
	for (int j=0; j<MAXTEST; j++)
	{
		t[0][j]	= (int*) kmalloc (TESTSIZE); 
		for (int k=0; k<TESTSIZE/sizeof(int); k++)
		{
			memmove (t[0][j]+k, &counter, sizeof(int));
			counter++;
		}
	}
	tmp = (int*) kmalloc (TESTSIZE);
	cprintf( (!tmp && numobj_slab (TESTSLABID) == MAXTEST) ? "OK\n" : "WRONG\n");	

	slabdump();

	/* TEST6: ALLOC AFTER FREE */
	cprintf("==== TEST6 =====\n");
	for (int j=0; j<MAXTEST; j++)
	{
		kmfree((char*) t[0][j], TESTSIZE);
	}

	start = counter;
	for (int j=0; j<MAXTEST; j++)
	{
		t[0][j]	= (int*) kmalloc (TESTSIZE); 
		for (int k=0; k<TESTSIZE/sizeof(int); k++)
		{
			memmove (t[0][j]+k, &counter, sizeof(int));
			counter++;
		}
	}
	
	slabdump();

	// CHECK 
	pass = 1;
	for (int j=0; j<MAXTEST; j++)
	{
		for (int k=0; k<TESTSIZE/sizeof(int); k++)
		{
			//cprintf("%d, %d, %d, %d\n", j, k, *(t[0][j]+k), start);		//YOU MAY USE THIS
			if (*(t[0][j]+k) != start)
	
			{
				pass = 0;
				break;
			}
			start++;
		}
	}
	cprintf( pass ? "OK\n" : "WRONG\n");	
	for (int j=0; j<MAXTEST; j++)
		kmfree((char*) t[0][j], TESTSIZE);

	slabdump();
}

