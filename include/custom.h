extern void burst_execution(uint32 number_bursts, uint32 burst_duration, uint32 sleep_duration);

pid32 enqueue_mlfq(pid32 pid);
pid32 dequeue_mlfq();
int nonempty_mlfq();
void boost_mlfq();
