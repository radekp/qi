#include <qi.h>
#include <string.h>

int memory_test_const32(void * start, unsigned int length, u32 value)
{
	int errors = 0;
	u32 * p = (u32 *)start;
	u32 * pend = (u32 *)(start + length);
	int count = length >> 2;

	puts(".");

	while (p < pend)
		*p++ = value;

	p = (u32 *)start;
	count = length >> 2;

	while (count--)
		if (*p++ != value) {
			puts("*** Error ");
			print32((long)p - 4);
			errors++;
		}

	return errors;
}

int memory_test_ads(void * start, unsigned int length, u32 mask)
{
	int errors = 0;
	u32 * p = (u32 *)start;
	u32 * pend = (u32 *)(start + length);

	puts(".");

	while (p < pend)
		if ((u32)p & mask)
			*p++ = 0xffffffff;
		else
			*p++ = 0;

	p = (u32 *)start;

	while (p < pend) {
		if ((u32)p & mask) {
			if (*p++ != 0xffffffff) {
				puts("*** Error ");
				print32((long)p - 4);
				errors++;
			}
		} else {
			if (*p++) {
				puts("*** Error ");
				print32((long)p - 4);
				errors++;
			}
		}
	}
	return errors;
}

int memory_test_walking1(void * start, unsigned int length)
{
	int errors = 0;
	u32 value = 1;

	while (value) {
		errors += memory_test_const32(start, length, value);
		value <<= 1;
	}

	return errors;
}

/* negative runs == run forever */

void __memory_test(void * start, unsigned int length)
{
	int errors = 0;
	int series = 0;
	int mask;

	puts("\nMemory Testing 0x");
	print32((u32)start);
	puts(" length ");
	printdec(length >> 20);
	puts(" MB\n");

	while (1) {
		puts(" Test series ");
		printdec(series + 1);
		puts("  ");

		/* these are looking at data issues, they flood the whole
		 * array with the same data
		 */

		errors += memory_test_const32(start, length, 0x55555555);
		errors += memory_test_const32(start, length, 0xaaaaaaaa);
		errors += memory_test_const32(start, length, 0x55aa55aa);
		errors += memory_test_const32(start, length, 0xaa55aa55);
		errors += memory_test_const32(start, length, 0x00ff00ff);
		errors += memory_test_const32(start, length, 0xff00ff00);
		errors += memory_test_walking1(start, length);

		/* this is looking at addressing issues, it floods only
		 * addresses meeting a walking mask with 0xffffffff (the rest
		 * is zeroed), and makes sure all the bits are only seen where
		 * they were placed
		 */

		mask = 1;
		while (! (length & mask)) {
			errors += memory_test_ads(start, length, mask);
			mask = mask << 1;
		}

		puts("  Total errors: ");
		printdec(errors);
		puts("\n");

		series++;
	}
}


void memory_test(void * start, unsigned int length)
{
	/* it's a small steppingstone stack from start.S */
	extern int _ss_stack;

	/*
	 * we won't be coming back from this, so just force our local stack to
	 * steppingstone out of the way of main memory test action
	 *
	 * then jump into the actual test
	 */
	asm volatile (
		"mov	sp, %0\n"
		: : "r" (&_ss_stack)
	);

	__memory_test(start, length);
}
