/* testset.S - */

		.text
		.globl test_and_set

/*------------------------------------------------------------------------
 * test_and_set(*ptr, new_value)                                   
 *------------------------------------------------------------------------
 */
test_and_set:
		pushl	%ebp		/* Push ebp onto stack		*/
		movl	%esp,%ebp	/* Record current SP in ebp	*/
	
		movl    4(%ebp), %eax   /* save the first parameter so it */
					/* can be returned              */
	        movl    8(%ebp), %ecx   /* move the second param to ecx  */
		movl    4(%ebp), %edx   /* should we move the first param too? */
		xchg    %ecx, %edx      /* exchanging these two values  */ 
		                        /* save return value at eax */
		movl	4(%esp),%ebp	/* Pick up ebp before restoring	*/
		popl 	%ebp 		/* restore ebp                  */
		ret			/* Return to new process	*/
