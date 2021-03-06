/*
 * =====================================================================================
 *
 *	Filename:  		sma.c
 *
 *  Description:	Base code for Assignment 3 for ECSE-427 / COMP-310
 *
 *  Version:  		1.0
 *  Created:  		6/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify 
 * 					them accordingly. Feel free to modify the "PRIVATE" functions.
 * 					Don't modify the "PUBLIC" functions (except the TODO part), unless
 * 					you find a bug! Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include "sma.h" // Please add any libraries you plan to use inside this file

/* Definitions*/
#define ROLL 32 // Max top free block size = 128 Kbytes
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes
//	TODO: Change the Header size if required
#define FREE_BLOCK_HEADER_SIZE (2 * sizeof(int)) // Size of the Header in a free memory block
//	TODO: Add constants here
// #define BLOCK_SIZE (128) //size of block allocated everytime brk or sbrk is called

typedef enum //	Policy type definition
{
	WORST,
	NEXT
} Policy;

typedef struct Node { 
    struct Node* next; // Pointer to next node in DLL 
    struct Node* prev; // Pointer to previous node in DLL 
};

char *sma_malloc_error;
struct Node *freeListHead = NULL;			  //	The pointer to the HEAD of the doubly linked free memory list
struct Node *freeListTail = NULL;			  //	The pointer to the TAIL of the doubly linked free memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;	  //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;		  //	Current Policy
//	TODO: Add any global variables here
int tag = 0;
int unallocated_length = 0;
int allocated_length = 0;
struct Node* saveBlock;
int count = 0;



/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: sma_malloc
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates a memory block of input size from the heap, and returns a 
 * 					pointer pointing to it. Returns NULL if failed and sets a global error.
 */
void *sma_malloc(int size)
{
	void *pMemory = NULL;

	// Checks if the free list is empty
	if (freeListHead == NULL)
	{
		// Allocate memory by increasing the Program Break
		pMemory = allocate_pBrk(size);
	}
	// If free list is not empty
	else
	{
		// Allocate memory from the free memory list
		pMemory = allocate_freeList(size);
		
		// If a valid memory could NOT be allocated from the free memory list
		if (pMemory == (void *)-2)
		{
			// Allocate memory by increasing the Program Break
			pMemory = allocate_pBrk(size);
		}
	}

	// Validates memory allocation
	if (pMemory < 0 || pMemory == NULL)
	{
		sma_malloc_error = "Error: Memory allocation failed!";
		return NULL;
	}

	// Updates SMA Info
	totalAllocatedSize += size;
	return pMemory;
}

/*
 *	Funcation Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{

	//	Checks if the ptr is NULL
	if (ptr == NULL)
	{
		puts("Error: Attempting to free NULL!");
	}
	//	Checks if the ptr is beyond Program Break
	else if (ptr > sbrk(0))
	{
		puts("Error: Attempting to free unallocated space!");
	}
	else
	{
		//	Adds the block to the free memory list
		add_block_freeList(ptr);
	}
}

/*
 *	Funcation Name: sma_mallopt
 *	Input type:		int
 * 	Output type:	void
 * 	Description:	Specifies the memory allocation policy
 */
void sma_mallopt(int policy)
{
	// Assigns the appropriate Policy
	if (policy == 1)
	{
		currentPolicy = WORST;
	}
	else if (policy == 2)
	{
		currentPolicy = NEXT;
	}
}

/*
 *	Funcation Name: sma_mallinfo
 *	Input type:		void
 * 	Output type:	void
 * 	Description:	Prints statistics about current memory allocation by SMA.
 */
void sma_mallinfo()
{
	//	Finds the largest Contiguous Free Space (should be the largest free block)
	int largestFreeBlock = get_largest_freeBlock();
	char str[60];

	//	Prints the SMA Stats
	sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
	puts(str);
	sprintf(str, "Total free space: %lu", totalFreeSize);
	puts(str);
	sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
	puts(str);
}

/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size)
{
	// TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address
	//			had been previously allocated.
	// Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
	//			chop off the current allocated memory and add to the free list. If new size is bigger
	//			then check if there is sufficient adjacent free space to expand, otherwise find a new block
	//			like sma_malloc.
	//			Should not accept a NULL pointer, and the size should be greater than 0.
}

/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */

struct Node* get_largest_freeBlock_ptr()
{
	struct Node* largestBlock = 0;
	struct Node* temp = freeListHead;
	if(temp->next == NULL)
	{
		largestBlock = temp;
		return largestBlock;
	}
	while(temp->next != NULL)
	{
		if(get_blockSize(temp) > get_blockSize(largestBlock))
		{
			largestBlock = temp;
		}
		temp = temp->next; 
	}
	return largestBlock;
}
/*
 *	Funcation Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break
 */
void *allocate_pBrk(int size)
{
	void *newBlock = NULL;
	int excessSize;

	//	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
	//	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
	//			Also, if you are getting a larger memory, you need to put the excess in the free list

	//I allocate a fixed amount of 128 bytes every time i call sbrk()
	//memory allocated to tag included in the allocated memory block
	newBlock = sbrk(MAX_TOP_FREE);
	//the excess size would be the allocated size - the size we want
	excessSize = MAX_TOP_FREE - size;

	//	Allocates the Memory Block
	allocate_block(newBlock, size, excessSize, 0);

	return newBlock;
}

/*
 *	Funcation Name: allocate_freeList
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory from the free memory list
 */
void *allocate_freeList(int size)
{
	void *pMemory = NULL;

	if (currentPolicy == WORST)
	{		
		// Allocates memory using Worst Fit Policy
		pMemory = allocate_worst_fit(size);
		count++;
		if(count == ROLL){
			pMemory = (void*)saveBlock;
		}
	}
	else if (currentPolicy == NEXT)
	{
		// Allocates memory using Next Fit Policy
		pMemory = allocate_next_fit(size);
	}
	else
	{
		pMemory = NULL;
	}

	return pMemory;
}

/*
 *	Funcation Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size)
{
	void *worstBlock = NULL;
	int worstBlockSize = 0;
	int excessSize;
	int blockFound = 0;

	//if get_largest_freeBlock_ptr() returns something, means there is an available 

	worstBlock = (void*) get_largest_freeBlock_ptr();
	if(get_blockSize(worstBlock) >= size)
	{
		blockFound = 1;
	}
	//	Checks if appropriate block is found.
	if (blockFound)
	{
		excessSize = *((int*)(worstBlock + 1)) + 2*FREE_BLOCK_HEADER_SIZE - size;
		//	Allocates the Memory Block
		allocate_block(worstBlock, size, excessSize, 1);
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		worstBlock = (void *)-2;
	}

	return worstBlock;
}

/*
 *	Funcation Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size)
{
	void *nextBlock = NULL;
	int excessSize;
	int blockFound = 0;

	//	TODO: 	Allocate memory by using Next Fit Policy
	//	Hint:	You should use a global pointer to keep track of your last allocated memory address, and 
	//			allocate free blocks that come after that address (i.e. on top of it). Once you reach 
	//			Program Break, you start from the beginning of your heap, as in with the free block with
	//			the smallest address)

	//	Checks if appropriate found is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(nextBlock, size, excessSize, 1);
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		nextBlock = (void *)-2;
	}
	return nextBlock;
}

/*
 *	Funcation Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 */
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList)
{
	void* excessFreeBlock; //	pointer for any excess free block
	int addFreeBlock;
	addFreeBlock = excessSize > 2*FREE_BLOCK_HEADER_SIZE;
	//	If excess free size is big enough
	if (addFreeBlock)
	{
		//my excessFreeBlock points at the start of the free block, so since newBlock points at the start of size, 
		//newBlock - size would give me the pointer at the start of the free block
		excessFreeBlock = newBlock - size;

		//add tag and length on both end of the memory block
		unallocated_length = excessSize - 2*FREE_BLOCK_HEADER_SIZE;
		allocated_length = size - 2*FREE_BLOCK_HEADER_SIZE;

		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
				/*----------set used memory header and tail------------*/

			tag = 1;	// in use, cannot merge
			//Used memory header
			*((int*) newBlock) = tag;
			*((int*) newBlock + 1) = allocated_length;

			//Used memory tail
			//get current program break 
			*((int*) excessFreeBlock - 1) = tag;
			*((int*) excessFreeBlock - 2) = allocated_length;

				/*----------set free block header and tail------------*/

			tag = 0;	// free, can merge
			//Free Block Header
			*((int*) excessFreeBlock) = tag;
			*((int*) excessFreeBlock + 1) = unallocated_length;

			//Free Block tail
			//get current program break 
			void *programBreak = sbrk(0);
			*((int*) programBreak - 1) = tag;
			*((int*) programBreak - 2) = unallocated_length;
			//	Removes new block and adds the excess free block to the free list
			replace_block_freeList(newBlock, excessFreeBlock);
		}
		else
		{
				/*----------set free block header and tail------------*/

			tag = 0;	// free, can merge
			//Free Block Header
			*((int*) excessFreeBlock) = tag;
			*((int*) excessFreeBlock + 1) = unallocated_length;

			//Free Block tail
			//get current program break 
			void *programBreak = sbrk(0);
			*((int*) programBreak - 1) = tag;
			*((int*) programBreak - 2) = unallocated_length;

				/*----------set used memory header and tail------------*/

			tag = 1;	// in use, cannot merge
			//Used memory header
			*((int*) newBlock) = tag;
			*((int*) newBlock + 1) = allocated_length;

			//Used memory tail
			//get current program break 
			*((int*) excessFreeBlock - 1) = tag;
			*((int*) excessFreeBlock - 2) = allocated_length;
			//	Adds excess free block to the free list
			add_block_freeList(excessFreeBlock);
		}
	}

	//	Otherwise add the excess memory to the new block
	else
	{
		size += excessSize;
		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes the new block from the free list
			remove_block_freeList(newBlock);
		}
	}
}

/*
 *	Funcation Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list
 */
void replace_block_freeList(void *oldBlock, void *newBlock)
{
	struct Node* temp = freeListHead;
	if(temp = (struct Node*) oldBlock)
	{
		replaceNode(temp, (struct Node*) newBlock);
	}
	while(temp->next != (struct Node*) oldBlock) temp = temp->next;
	replaceNode(temp->next, (struct Node*) newBlock);

	//	Updates SMA info
	totalAllocatedSize += (get_blockSize(oldBlock) - get_blockSize(newBlock));
	totalFreeSize += (get_blockSize(newBlock) - get_blockSize(oldBlock));
}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(void *block)
{
	//	TODO: 	Add the block to the free list
	//	Hint: 	You could add the free block at the end of the list, but need to check if there
	//			exits a list. You need to add the TAG to the list.
	//			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
	//			Merging would be tideous. Check adjacent blocks, then also check if the merged
	//			block is at the top and is bigger than the largest free block allowed (128kB).
	struct Node* convertedBlock = block;
	convertedBlock->next = NULL;
	convertedBlock->prev = NULL;

	if(freeListHead == NULL){
		freeListHead = convertedBlock;
		freeListTail = convertedBlock;
		totalAllocatedSize -= get_blockSize(block);
		totalFreeSize += get_blockSize(block);
		saveBlock = freeListHead;
		// convertedBlock = NULL;
		return;
	}
	// struct Node* temp = freeListHead;

	*((int*)freeListHead+2) += 1024;
	
	//	Updates SMA info
	totalAllocatedSize -= get_blockSize(block);
	totalFreeSize += get_blockSize(block);
	convertedBlock = NULL;

}

/*
 *	Funcation Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void* block)
{
	//	TODO: 	Remove the block from the free list
	//	Hint: 	You need to update the pointers in the free blocks before and after this block.
	//			You also need to remove any TAG in the free block.

	if (freeListHead == NULL || block == NULL)
        return;
 
    /* If node to be deleted is head node */
    if (freeListHead == block)
        freeListHead = ((struct Node*)block)->next;
 
    /* Change next only if node to be deleted is NOT the last node */
    if (((struct Node*)block)->next != NULL)
        ((struct Node*)block)->next->prev = ((struct Node*)block)->prev;
 
    /* Change prev only if node to be deleted is NOT the first node */
    if (((struct Node*)block)->prev != NULL)
        ((struct Node*)block)->prev->next = ((struct Node*)block)->next;

	//	Updates SMA info
	totalAllocatedSize += get_blockSize(block);
	totalFreeSize -= get_blockSize(block);
}

/*
 *	Funcation Name: get_blockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	Extracts the Block Size
 */
int get_blockSize(void *ptr)
{
	int *pSize;

	//	Points to the address where the Length of the block is stored
	pSize = (int *)ptr;
	pSize+2;

	//	Returns the deferenced size
	return *(int *)pSize;
}

/*
 *	Funcation Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock()
{
	int largestBlockSize = 0;
	struct Node *temp = freeListHead;
	if(temp->next == NULL)
	{
		largestBlockSize = get_blockSize(temp);
		return largestBlockSize;
	}
	while(temp->next != NULL)
	{
		if(get_blockSize(temp) > largestBlockSize)
		{
			largestBlockSize = get_blockSize(temp);
		}
		temp = temp->next; 
	}
	return largestBlockSize;
}

void replaceNode(struct Node* oldNode, struct Node* newNode)
{
	// if node to be replaced is the only one in the linked list, assign newNode to freeListHead
	if(oldNode->next == NULL){
		freeListHead = newNode;
		return;
	}

    /* Change next only if node to be deleted is NOT the last node */
    if (oldNode->next != NULL)
	{
        oldNode->next->prev = newNode;
		newNode->next = oldNode->next;

		/* If node to be deleted is head node (first node)*/
		if(freeListHead = oldNode) freeListHead = newNode;
	}

    /* Change prev only if node to be deleted is NOT the first node */
    if (oldNode->prev != NULL)
	{
        oldNode->prev->next = newNode;
		newNode->prev = oldNode->prev;

		if(freeListTail = oldNode) freeListTail = newNode;
	}
    
	return;
}
