#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "slab.h"
#include <stdbool.h>
//#define true 1
//#define false 0

struct {
	struct spinlock lock;
	struct slab slab[NSLAB];
} stable;

// 제곱 함수 구현
float my_pow(float a, float b)
{
	int count;
	float result=1;

	for (count=0; count <(int)b; count++)
	{
		result *= a;
	}

	return result;
}

// 현재 값보다 큰 값 중 가장 작은 2의 제곱 수 구하는 함수
unsigned int next_power_of_2(unsigned int n)
{
	unsigned int power = 8;

	while (power < n)
	{
		power = power << 1;
	}

	return power;



//	if (n && !(n & (n - 1)))
//		return n ;

//	while (n != 0)
//	{
//		n >>= 1;
//		count += 1;
//	}

//	return 1 << count;
}



// get_bit 함수: 해당 bit 사용 여부 확인 (미사용: false, 사용: true)
bool get_bit(char *bitmap, int i)
{
	// ex) i=5
	//   11001001
	// & 00100000
	// ----------
	//   00000000 -> 0 (비어있음: false)
	int index = (i / 8);
	int offset = (i % 8);
	bitmap += index;
	return ((*bitmap & (1 << offset)) != 0);
}

// set_bit 함수 (해당 비트 1로 변환)
void set_bit(char *bitmap, int i)
{
	int index = (i / 8);
	int offset = (i % 8);
	bitmap += index;
	*bitmap = *bitmap | (1 << offset);

//	return num | (1 << i);
}


// clear_bit 함수 (해당 비트 0으로 변환)
void clear_bit(char *bitmap, int i)
{
	int index = (i / 8);
	int offset = (i % 8);

	bitmap += index;
	*bitmap = *bitmap & ~(1 << offset);

	//int mask = ~(1 << i);
	//return num & mask;
}


void slabinit(){
	/* fill in the blank */

	initlock(&stable.lock, "stable lock");
	acquire(&stable.lock);

	// slab 초기화
	for (int i=0; i<NSLAB; i++)
	{
		// 각 slab cache마다 8, 16, ... , 2024 크기 설정
		stable.slab[i].size = 8 * my_pow(2.0, i);

		// page 개수 초기값 0으로 설정 
		stable.slab[i].num_pages = 0;

		// 각 free_objects 개수는: page 크기(4KB) / slab size (object 크기)
		stable.slab[i].num_free_objects = 4096 / stable.slab[i].size;

		// 초기에는 사용 중인 object 없음
		stable.slab[i].num_used_objects = 0;

		// 초기 free_objects 개수와 동일
		stable.slab[i].num_objects_per_page = 4096 / stable.slab[i].size;

		// bitmap은 kalloc()으로 할당
		stable.slab[i].bitmap = kalloc();

		// 각 slab마다 slab cache 하나씩 할당 (최초 1 page는 있어야 작동 가능)
		// stable.slab[i].page[0] = kalloc();
		// stable.slab[i].num_pages += 1;

		stable.slab[i].page[stable.slab[i].num_pages++] = kalloc();
	}	
	release(&stable.lock);

}

char *kmalloc(int size){
	/* fill in the blank */
	
	// 범위 밖의 크기는 작동 불가
	if (size < 0 || size > 2048)
		return 0;
	
	struct slab *s = 0;

	acquire(&stable.lock);
	
	// 들어갈 object 크기
	unsigned int required_size = next_power_of_2(size);

	// 들어갈 적당한 크기의 공간 찾기 (할당량보다 큰 것 중 최소 object) 
	for (s=stable.slab; s<&stable.slab[NSLAB]; s++)
	{
		// size: 할당받고 싶은 크기
		// s->size: 8, 16, 32, ..., 2024
		if (required_size == s->size)
			break;
	}
	
	


	// 빈 공간이 없는 경우 새로운 페이지 추가
	if (s->num_free_objects == 0)
	{
		// 이미 최대 생성 가능한 페이지 개수를 초과한 경우 종료
		if (s->num_pages >= MAX_PAGES_PER_SLAB)
		{
			release(&stable.lock);
			return 0;
		}

		// 1개 page의 bitmap이 가진 비트 수: 32768개를 초과하면 안됨
		if (s->size == 8 && s->num_pages > 64)
		{
			release(&stable.lock);
			return 0;
		}
		
		// 새로운 페이지 할당
		s->page[s->num_pages] = kalloc();

		// kalloc 실패 시
//		if (s->page[s->num_pages] == 0)
//		{
//			release(&stable.lock);
//			return 0;
//		}

		
		// slab 구조체 변수 수정
		s->num_pages += 1;
		s->num_free_objects += s->num_objects_per_page;
	}

	int max_len = s->num_pages * s->num_objects_per_page;
	char *addr = 0; // 반환할 주소

	for (int i=0; i<max_len; i++)
	{
		// 해당 비트(object)가 미사용 중이라면 할당
 		if (get_bit(s->bitmap, i) == false)
		{
			// 몇 번째 page인가?
			int page_index = i / s->num_objects_per_page;

			// 몇 번째 object인가?
			int page_offset = i % s->num_objects_per_page;

			// 주소값 설정
			addr = s->page[page_index] + (page_offset * s->size);
			
			set_bit(s->bitmap, i);

			s->num_used_objects += 1;
			s->num_free_objects -= 1;

			break;
		}
	}

	release(&stable.lock);

//	return 0x00;
	return addr;
}

void kmfree(char *addr, int size){ // addr: free할 object의 주소값, size: object 크기
	/* fill in the blank */
	struct slab *s;

	acquire(&stable.lock);
	
	// free할 object 크기 찾기
	for (s=stable.slab; s<&stable.slab[NSLAB]; s++)
	{
		if (size <= s->size)
			break;
	}

	int max_len = s->num_pages * s->num_objects_per_page;

	for (int i=0; i<max_len; i++)
	{
		// page 위치와 오프셋
		int page_index = i / s->num_objects_per_page;
		int page_offset = i % s->num_objects_per_page;

		if (addr == (s->page[page_index] + (page_offset * s->size)))
		{
			// 기존 데이터 삭제를 위해 1로 채우기
			memset(addr, 1, s->size);

			s->num_free_objects += 1;
			s->num_used_objects -= 1;

			// bitmap 초기화
			clear_bit(s->bitmap, i);

			// free page
			// int page_num = s->num_pages;
			// int object_num = s->num_objects_per_page;

			// if (page_num > 1)
			// {
			// 	for (int j=0, k=0; j<max_len; j+=object_num, k++)
			// 	{

			// 		bool is_clean = true;

			// 		for (int l=0; l<object_num; l++)
			// 		{
			// 			if (get_bit(s->bitmap, j) == true)
			// 			{
			// 				is_clean = false;
			// 			}
			// 		}
					
			// 		if (is_clean == true)
			// 		{
			// 			for (int iter=0; iter<object_num; iter++)
			// 			{
			// 				for (int l=j; l<max_len-1; l++)
			// 				{
			// 					if (get_bit(s->bitmap, i+1))
			// 					{
			// 						set_bit(s->bitmap, i);
			// 					}
			// 					else
			// 					{
			// 						clear_bit(s->bitmap, i);
			// 					}

			// 				}

			// 				clear_bit(s->bitmap, max_len-1);

			// 			}

			// 			kfree(s->page[page_index]);
						
			// 			for (int l=j; l<page_num-1; l++)
			// 			{
			// 				s->page[l] = s->page[l+1];
			// 			}

			// 			s->num_pages -= 1;
			// 			s->num_free_objects -= object_num;

			// 			break;
			// 		}
			// 	}

			// 	release(&stable.lock);
			// 	return;
			// }
		}
	}
	release(&stable.lock);
	return;						
}

void slabdump(){
	cprintf("__slabdump__\n");

	struct slab *s;

	cprintf("size\tnum_pages\tused_objects\tfree_objects\n");

	for(s = stable.slab; s < &stable.slab[NSLAB]; s++){
		cprintf("%d\t%d\t\t%d\t\t%d\n", 
			s->size, s->num_pages, s->num_used_objects, s->num_free_objects);
	}
}

int numobj_slab(int slabid)
{
	return stable.slab[slabid].num_used_objects;
}

