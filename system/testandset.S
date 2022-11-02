/* testset.S - */

		.text
		.globl test_and_set

/*------------------------------------------------------------------------
 * test_and_set(*ptr, new_value)                                   
 *------------------------------------------------------------------------
 */
test_and_set:
		/* Prolog  */
		pushl	%ebp		/* Push ebp onto stack		*/
		movl	%esp,%ebp	/* Record current SP in ebp	*/

		/* Body    */
		movl   8(%ebp), %ecx    /* get the address pointed by *ptr */
		movl   (%ecx),  %eax    /*get the content of addr pointed by ptr */
		
		/* get the params and swap */
 		movl  12(%ebp), %edx	/* get the new_value, place it on edx */
		xchg   %edx, (%ecx)       /* swap new_value and *ptr */
		                        /* save return value at eax */
		/* epilogue */
		movl	4(%esp),%ebp	/* Pick up ebp before restoring	*/

		popl 	%ebp 		/* restore ebp                  */
		ret			/* Return to new process	*/
