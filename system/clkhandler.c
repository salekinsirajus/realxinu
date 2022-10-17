/* clkhandler.c - clkhandler */

#include <xinu.h>

/*------------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *------------------------------------------------------------------------
 */
void	clkhandler()
{
	static	uint32	count1000 = 1000;	/* Count to 1000 ms	*/

	/* increasing the runtime by 1 ms for the current process.
	   FIXME: might produce slight deviation in runtime value based on
	   the location of the increament counter */

	proctab[currpid].runtime++;
	proctab[currpid].time_allotment--;
	/* Decrement the ms counter, and see if a second has passed */

	if((++ctr1000) % 1000 == 0) {

		/* One second has passed, so increment seconds count */

		clktime++;

		/* Reset the local ms counter for the next second */

		count1000 = 1000;
	}

	/* Handle sleeping processes if any exist */

	if(!isempty(sleepq)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queuetab[firstid(sleepq)].qkey) <= 0) {
			wakeup();
		}
	}

	/* perform prioerity boosts for all user level processes */
	//FIXME: is this the right place to perform a priority boost?
	//right before a resched call?
	if (ctr1000 % PRIORITY_BOOST_PERIOD == 0){
		boost_mlfq();
	}

	/* Decrement the preemption counter, and reschedule when the */
	/*   remaining time reaches zero			     */

	if((--preempt) <= 0) {
		preempt = QUANTUM;
		resched();
	}
}
