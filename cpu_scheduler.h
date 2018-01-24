#ifndef __CPU_SCHEDULER_H__
#define __CPU_SCHEDULER_H__

#define TRUE	1
#define FALSE	0

//Related to Process
typedef struct _process {
	int pid;
	int cpu_burst;
	int io_burst;
	int io_start;
	int ent_btime;
	int arr_time;
	int comp_time;		//completion time
	int remain_time;	//for p_sjf
	int ent_remain_time;//for p_sjf_io
	int remain_time_io;	//for p_ljf
	int progress;
	int priority;
	_Bool start;
	int order;			//for sjf
	int time_quan;		//for RR
	int preemptive;
} Process;

typedef struct _evaluation {
	float avg_turnaround;
	float avg_waiting;
	int idle_time;
	int check;
	int finish_time;
} Evaluation;

//Related to Queue
typedef int Data;

typedef struct _node
{
	Data data;
	struct _node * next;
} Node;

typedef struct _lQueue
{
	Node * front;
	Node * rear;
} LQueue;

typedef LQueue Queue;

void QueueInit(Queue * pq);
int QIsEmpty(Queue * pq);

void Enqueue(Queue * pq, Data data);
Data Dequeue(Queue * pq);
Data QPeek(Queue * pq);

#endif