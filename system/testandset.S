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

		/* body */
		movl	8(%ebp), %eax    /* moving param 1 to eax *ptr */
		movl	8(%ebp), %ecx    /* taking *ptr1 and putting it a eax */
		movl	(%eax), %eax     /* getting the value at *ptr addr */

		movl	12(%ebp), %edx   /* taking new_value and putting it at edx */
		xchg	(%ecx), %edx     /* moving edx=new_value at the location pointed by *ptr */

		/* epilogue */
        movl %ebp, %esp      /* moving ebp to esp */ 
        popl %ebp            /* pop ebp off the stack */ 
        ret                  /* return */ 
