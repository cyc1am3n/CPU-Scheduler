#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cpu_scheduler.h"

//Declare constants.
#define MAX_P_NUM 10

int p_num = 0; // Number of process
Process *process[MAX_P_NUM];

// Declare each algorithms' evaluation
Evaluation fcfs, np_sjf, p_sjf, np_pri, p_pri, rr, np_sjf_io, p_sjf_io, p_ljf;

// Define Queue
Queue ready_q, running_q;
int waiting_q[MAX_P_NUM];

// Functions related to Queue
void QueueInit(Queue * pq) {
	pq->front = NULL;
	pq->rear = NULL;
}

int QIsEmpty(Queue * pq) {
	if (pq->front == NULL) return TRUE;
	else return FALSE;
}

void Enqueue(Queue * pq, Data data) {
	Node * newNode = (Node*)malloc(sizeof(Node));
	newNode->next = NULL;
	newNode->data = data;

	if (QIsEmpty(pq)) {
		pq->front = newNode;
		pq->rear = newNode;
	}
	else {
		pq->rear->next = newNode;
		pq->rear = newNode;
	}
}

Data Dequeue(Queue * pq) {
	Node * delNode;
	Data retData;

	if (QIsEmpty(pq)) {
		printf("Queue Memory Error![dequeue]");
		exit(-1);
	}
	delNode = pq->front;
	retData = delNode->data;
	pq->front = pq->front->next;

	free(delNode);
	return retData;
}

Data QPeek(Queue * pq) {
	if (QIsEmpty(pq)) {
		printf("Queue Memory Error![peek]");
		exit(-1);
	}
	return pq->front->data;
}

//Function of creating processes
void create_process() {
	printf("< CPU SCHEDULER >\n");
	printf("* Please enter the number of processes to be created.(1 ~ 10): ");
	while (1) {
		scanf("%d", &p_num);
		if (p_num > MAX_P_NUM || p_num < 0) printf("[ERROR] Number of processes must be 1 to 10. Please enter number of processes again: ");
		else break;
	}

	srand(time(NULL));
	int found[10]; int pri, flag;

	for (int i = 0; i < p_num; i++) {
		Process *p_process = (Process*)malloc(sizeof(Process) * 1);
		p_process->pid = i;														//Process ID: 0 ~ 9(orderly)
		p_process->cpu_burst = (int)(rand() % 8 + 2);							//CPU burst time: 2 ~ 10
		p_process->io_burst = (int)(rand() % 4 + 1);							//I/O burst time: 1 ~ 5
		p_process->arr_time = (int)(rand() % (4 * p_num) + 1);					//Arrival time: 1 ~ 4 x # of processes
		//Duplicate prevention function(priority)
		while (1) {
			pri = (rand() % p_num + 1);
			flag = 0;
			for (int j = 0; j <= i; j++) {
				if (pri != found[j]) { flag++; }
			}
			if (flag == i + 1) { found[i] = pri; break; }
		}
		p_process->priority = found[i];											//Priority: 1 ~ # of processes
		p_process->io_start = (int)(rand() % (p_process->cpu_burst - 1) + 1);	//I/O start time: after 1 ~ n - 1 (randomly)
		p_process->remain_time = p_process->cpu_burst;
		p_process->ent_btime = p_process->cpu_burst + p_process->io_burst;
		p_process->ent_remain_time = p_process->ent_btime;
		p_process->remain_time_io = p_process->io_burst;
		p_process->start = FALSE;
		p_process->comp_time = 0;												//Completion time
		p_process->progress = 0;
		p_process->time_quan = 0;
		p_process->preemptive = 0;												//Indicating that it can be preemptive.
		process[i] = p_process;
	}
	// At least one process can arrive at the start.
	process[(int)(rand() % p_num)]->arr_time = 1;

	printf("----------------------------------------------------------------------------------------\n");
	printf("Process\t\tCPU Burst\tI/O Burst\tI/O Start\tArrival Time\tPriority\n");
	printf("----------------------------------------------------------------------------------------\n");
	for (int i = 0; i < p_num; i++) {
		printf("P%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", process[i]->pid, process[i]->cpu_burst, process[i]->io_burst, process[i]->io_start, process[i]->arr_time - 1, process[i]->priority);
	}
	printf("----------------------------------------------------------------------------------------\n");
	printf("- Finished creating the process.\n\n");
}

//Function of system environment setting (ready queue(queue) & running queue(queue) & waiting queue(array))
void config() {
	QueueInit(&ready_q);
	QueueInit(&running_q);
	for (int i = 0; i < p_num; i++) {
		waiting_q[i] = -1;
	}
	printf("- Finished creating the queue.\n\n");
}

//Initializing processes' environment for scheduling algorithms
void Initalize() {
	for (int i = 0; i < p_num; i++) {
		waiting_q[i] = -1;
		process[i]->progress = 0;
		process[i]->comp_time = 0;
	}
}

//CPU Scheduling algorithms
void FCFS() {
	printf("[FCFS Scheduling]\n");
	int i, pick, time;
	fcfs.idle_time = 0, fcfs.check = 0;
	Initalize();

	for (time = 1; fcfs.check != p_num; time++) {	//Terminate when all processes are finished.
		for (i = 0; i < p_num; i++) {
			if (time == process[i]->arr_time) { Enqueue(&ready_q, process[i]->pid); }
			if (waiting_q[i] > 0) { waiting_q[i]--; }						//Doing I/O operation...
			if (waiting_q[i] == 0) { Enqueue(&ready_q, i); waiting_q[i]--; }//When the process waits for io burst, it re-enters the ready queue.
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick);
			process[pick]->start = TRUE;
		}

		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); fcfs.idle_time++; }
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {
				int waiting = Dequeue(&running_q);
				waiting_q[waiting] = process[waiting]->io_burst + 1;
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				fcfs.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	fcfs.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	fcfs.avg_turnaround = (float)tot_turn / p_num;
	fcfs.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", fcfs.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", fcfs.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void NP_SJF() {
	printf("[Non-preemptive Shortest Job First Scheduling]\n");
	int i, j, tmp, pick, time;
	np_sjf.idle_time = 0;

	Initalize();

	int arr[10][2] = { 11 };
	for (i = 0; i < p_num; i++) {
		arr[i][0] = i;
		arr[i][1] = process[i]->cpu_burst;
	}
	for (i = 0; i < p_num; i++) {
		for (j = i + 1; j < p_num; j++) {
			if (arr[i][1] > arr[j][1]) {
				tmp = arr[i][0]; arr[i][0] = arr[j][0]; arr[j][0] = tmp;
				tmp = arr[i][1]; arr[i][1] = arr[j][1]; arr[j][1] = tmp;
			}
		}
	}
	for (i = 0; i < p_num; i++) {
		for (j = 0; j < p_num; j++) {
			if (process[i]->pid == arr[j][0])
				process[i]->order = j;
		}
	}
	Queue temp; QueueInit(&temp);

	for (time = 1; np_sjf.check != p_num; time++) {
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; }
			if (waiting_q[i] == 0 || time == process[i]->arr_time) {
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q))	Enqueue(&ready_q, process[i]->pid);
				else {
					while (process[QPeek(&ready_q)]->order < process[i]->order) {
						Enqueue(&temp, Dequeue(&ready_q));
						if (QIsEmpty(&ready_q)) break;
					}
					Enqueue(&temp, process[i]->pid);
					while (!QIsEmpty(&ready_q)) {
						Enqueue(&temp, Dequeue(&ready_q));
					}
					while (!QIsEmpty(&temp)) {
						Enqueue(&ready_q, Dequeue(&temp));
					}
				}
			}
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick);
			process[pick]->start = TRUE;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); np_sjf.idle_time++; }
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {
				int waiting = Dequeue(&running_q);
				waiting_q[waiting] = process[waiting]->io_burst + 1;
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				np_sjf.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	np_sjf.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	np_sjf.avg_turnaround = (float)tot_turn / p_num;
	np_sjf.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", np_sjf.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", np_sjf.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void P_SJF() {
	printf("[Preemptive Shortest Job First Scheduling]\n");
	int i, pick, time, preem = 0;
	Queue temp; QueueInit(&temp);
	p_sjf.idle_time = 0, p_sjf.check = 0;

	Initalize();

	for (time = 1; p_sjf.check != p_num; time++) {
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; }
			if (waiting_q[i] == 0 || time == process[i]->arr_time) {
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid);
				else if (!QIsEmpty(&running_q)) {
					if (process[i]->remain_time < process[QPeek(&running_q)]->remain_time) {
						preem = Dequeue(&running_q);
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); }
						Enqueue(&ready_q, preem);
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }
						Enqueue(&running_q, i);
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else {
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i);
						else {
							while (process[QPeek(&ready_q)]->remain_time < process[i]->remain_time) {
								Enqueue(&temp, Dequeue(&ready_q));
								if (QIsEmpty(&ready_q)) break;
							}
							Enqueue(&temp, process[i]->pid);
							while (!QIsEmpty(&ready_q)) {
								Enqueue(&temp, Dequeue(&ready_q));
							}
							while (!QIsEmpty(&temp)) {
								Enqueue(&ready_q, Dequeue(&temp));
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->remain_time < process[i]->remain_time) {
						Enqueue(&temp, Dequeue(&ready_q));
						if (QIsEmpty(&ready_q)) break;
					}
					Enqueue(&temp, process[i]->pid);
					while (!QIsEmpty(&ready_q)) {
						Enqueue(&temp, Dequeue(&ready_q));
					}
					while (!QIsEmpty(&temp)) {
						Enqueue(&ready_q, Dequeue(&temp));
					}
				}
			}
		}
		if (!QIsEmpty(&running_q)) {
			pick = QPeek(&running_q);
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick);
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_sjf.idle_time++; }
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->remain_time--;
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {
				int waiting = Dequeue(&running_q);
				waiting_q[waiting] = process[waiting]->io_burst + 1;
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				p_sjf.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	p_sjf.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	p_sjf.avg_turnaround = (float)tot_turn / p_num;
	p_sjf.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", p_sjf.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", p_sjf.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void NP_PRIORITY() {
	printf("[Non-preemptive Priority Scheduling]\n");
	int i, pick, time;
	Queue temp; QueueInit(&temp);
	np_pri.idle_time = 0, np_pri.check = 0;

	Initalize();

	for (time = 1; np_pri.check != p_num; time++) {
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; }
			if (waiting_q[i] == 0 || time == process[i]->arr_time) {
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q))	Enqueue(&ready_q, process[i]->pid);
				else {
					while (process[QPeek(&ready_q)]->priority < process[i]->priority) {
						Enqueue(&temp, Dequeue(&ready_q));
						if (QIsEmpty(&ready_q)) break;
					}
					Enqueue(&temp, process[i]->pid);
					while (!QIsEmpty(&ready_q)) {
						Enqueue(&temp, Dequeue(&ready_q));
					}
					while (!QIsEmpty(&temp)) {
						Enqueue(&ready_q, Dequeue(&temp));
					}
				}
			}
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick);
			process[pick]->start = TRUE;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); np_pri.idle_time++; }
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {
				int waiting = Dequeue(&running_q);
				waiting_q[waiting] = process[waiting]->io_burst + 1;
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				np_pri.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	np_pri.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	np_pri.avg_turnaround = (float)tot_turn / p_num;
	np_pri.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", np_pri.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", np_pri.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void P_PRIORITY() {
	printf("[Preemptive Priority Scheduling]\n");
	int i, pick, time, preem = 0;
	Queue temp; QueueInit(&temp);
	p_pri.idle_time = 0, p_pri.check = 0;

	Initalize();

	for (time = 1; p_pri.check != p_num; time++) {
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; }
			if (waiting_q[i] == 0 || time == process[i]->arr_time) {
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid);
				else if (!QIsEmpty(&running_q)) {
					if (process[i]->priority < process[QPeek(&running_q)]->priority) {
						preem = Dequeue(&running_q);
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); }
						Enqueue(&ready_q, preem);
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }
						Enqueue(&running_q, i);
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else {
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i);
						else {
							while (process[QPeek(&ready_q)]->priority < process[i]->priority) {
								Enqueue(&temp, Dequeue(&ready_q));
								if (QIsEmpty(&ready_q)) break;
							}
							Enqueue(&temp, process[i]->pid);
							while (!QIsEmpty(&ready_q)) {
								Enqueue(&temp, Dequeue(&ready_q));
							}
							while (!QIsEmpty(&temp)) {
								Enqueue(&ready_q, Dequeue(&temp));
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->priority < process[i]->priority) {
						Enqueue(&temp, Dequeue(&ready_q));
						if (QIsEmpty(&ready_q)) break;
					}
					Enqueue(&temp, process[i]->pid);
					while (!QIsEmpty(&ready_q)) {
						Enqueue(&temp, Dequeue(&ready_q));
					}
					while (!QIsEmpty(&temp)) {
						Enqueue(&ready_q, Dequeue(&temp));
					}
				}
			}
		}
		if (!QIsEmpty(&running_q)) {
			pick = QPeek(&running_q);
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick);
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_pri.idle_time++; }
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {
				int waiting = Dequeue(&running_q);
				waiting_q[waiting] = process[waiting]->io_burst + 1;
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				p_pri.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	p_pri.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	p_pri.avg_turnaround = (float)tot_turn / p_num;
	p_pri.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", p_pri.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", p_pri.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void RR() {
	printf("[Round Robin Scheduling]\n");
	int i, pick, time;
	rr.idle_time = 0, rr.check = 0;

	Initalize();

	int time_quantum = 0;
	printf("* Please enter time quantum.(1 ~ 5): ");
	while (1) {
		scanf("%d", &time_quantum);
		if (time_quantum > 7 || time_quantum < 1) printf("[ERROR] Time Quantum must be 1 to 4. Please enter time quantum again: ");
		else break;
	}
	for (time = 1; rr.check != p_num; time++) {
		for (i = 0; i < p_num; i++) {
			if (time == process[i]->arr_time) { Enqueue(&ready_q, process[i]->pid); }
			if (waiting_q[i] > 0) { waiting_q[i]--; }
			if (waiting_q[i] == 0) { Enqueue(&ready_q, i); waiting_q[i]--; }
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick);
			process[pick]->start = TRUE;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); rr.idle_time++; }
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;
			process[pick]->time_quan++;

			if (process[pick]->progress == process[pick]->io_start) {
				int waiting = Dequeue(&running_q);
				waiting_q[waiting] = process[waiting]->io_burst + 1;
				process[waiting]->time_quan = 0;
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				rr.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
			else if (process[pick]->time_quan == time_quantum) {
				process[pick]->time_quan = 0;
				Enqueue(&ready_q, (Dequeue(&running_q)));
			}
		}
	}
	rr.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	rr.avg_turnaround = (float)tot_turn / p_num;
	rr.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", rr.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", rr.avg_turnaround);	printf("*****************************************************************************\n\n");
}

void NP_SJF_IO() {
	printf("(Additional)[Non-preemptive Shortest Job First Scheduling (I/O + CPU burst)]\n");
	int i, j, tmp, pick, time;
	np_sjf_io.idle_time = 0;

	Initalize();

	int arr[10][2] = { 11 };
	for (i = 0; i < p_num; i++) {
		arr[i][0] = i;
		arr[i][1] = process[i]->ent_btime;
	}
	for (i = 0; i < p_num; i++) {
		for (j = i + 1; j < p_num; j++) {
			if (arr[i][1] > arr[j][1]) {
				tmp = arr[i][0]; arr[i][0] = arr[j][0]; arr[j][0] = tmp;
				tmp = arr[i][1]; arr[i][1] = arr[j][1]; arr[j][1] = tmp;
			}
		}
	}
	for (i = 0; i < p_num; i++) {
		for (j = 0; j < p_num; j++) {
			if (process[i]->pid == arr[j][0])
				process[i]->order = j;
		}
	}
	Queue temp; QueueInit(&temp);

	for (time = 1; np_sjf_io.check != p_num; time++) {
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; }
			if (waiting_q[i] == 0 || time == process[i]->arr_time) {
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q))	Enqueue(&ready_q, process[i]->pid);
				else {
					while (process[QPeek(&ready_q)]->order < process[i]->order) {
						Enqueue(&temp, Dequeue(&ready_q));
						if (QIsEmpty(&ready_q)) break;
					}
					Enqueue(&temp, process[i]->pid);
					while (!QIsEmpty(&ready_q)) {
						Enqueue(&temp, Dequeue(&ready_q));
					}
					while (!QIsEmpty(&temp)) {
						Enqueue(&ready_q, Dequeue(&temp));
					}
				}
			}
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick);
			process[pick]->start = TRUE;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); np_sjf_io.idle_time++; }
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {
				int waiting = Dequeue(&running_q);
				waiting_q[waiting] = process[waiting]->io_burst + 1;
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				np_sjf_io.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	np_sjf_io.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	np_sjf_io.avg_turnaround = (float)tot_turn / p_num;
	np_sjf_io.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", np_sjf_io.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", np_sjf_io.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void P_SJF_IO() {
	printf("(Additional)[Preemptive Shortest Job First Scheduling(I/O + CPU burst)]\n");
	int i, pick, time, preem = 0;
	Queue temp; QueueInit(&temp);
	p_sjf_io.idle_time = 0, p_sjf_io.check = 0;

	Initalize();

	for (time = 1; p_sjf_io.check != p_num; time++) {
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; process[i]->ent_remain_time--; }
			if (waiting_q[i] == 0 || time == process[i]->arr_time) {
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid);
				else if (!QIsEmpty(&running_q)) {
					if (process[i]->ent_remain_time < process[QPeek(&running_q)]->ent_remain_time) {
						preem = Dequeue(&running_q);
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); }
						Enqueue(&ready_q, preem);
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }
						Enqueue(&running_q, i);
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else {
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i);
						else {
							while (process[QPeek(&ready_q)]->ent_remain_time < process[i]->ent_remain_time) {
								Enqueue(&temp, Dequeue(&ready_q));
								if (QIsEmpty(&ready_q)) break;
							}
							Enqueue(&temp, process[i]->pid);
							while (!QIsEmpty(&ready_q)) {
								Enqueue(&temp, Dequeue(&ready_q));
							}
							while (!QIsEmpty(&temp)) {
								Enqueue(&ready_q, Dequeue(&temp));
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->ent_remain_time < process[i]->ent_remain_time) {
						Enqueue(&temp, Dequeue(&ready_q));
						if (QIsEmpty(&ready_q)) break;
					}
					Enqueue(&temp, process[i]->pid);
					while (!QIsEmpty(&ready_q)) {
						Enqueue(&temp, Dequeue(&ready_q));
					}
					while (!QIsEmpty(&temp)) {
						Enqueue(&ready_q, Dequeue(&temp));
					}
				}
			}
		}
		if (!QIsEmpty(&running_q)) {
			pick = QPeek(&running_q);
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick);
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_sjf_io.idle_time++; }
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->ent_remain_time--;
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {
				int waiting = Dequeue(&running_q);
				waiting_q[waiting] = process[waiting]->io_burst + 1;
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				p_sjf_io.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	p_sjf_io.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	p_sjf_io.avg_turnaround = (float)tot_turn / p_num;
	p_sjf_io.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", p_sjf_io.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", p_sjf_io.avg_turnaround);
	printf("*****************************************************************************\n\n");
}
void P_LJF() {
	printf("(Additional)[Preemptive Longest Job First Scheduling(I/O burst)]\n");
	int i, pick, time, preem = 0;
	Queue temp; QueueInit(&temp);
	p_ljf.idle_time = 0, p_ljf.check = 0;

	Initalize();

	for (time = 1; p_ljf.check != p_num; time++) {
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; process[i]->remain_time_io--; }
			if (waiting_q[i] == 0 || time == process[i]->arr_time) {
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid);
				else if (!QIsEmpty(&running_q)) {
					if (process[i]->remain_time_io > process[QPeek(&running_q)]->remain_time_io) {
						preem = Dequeue(&running_q);
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); }
						Enqueue(&ready_q, preem);
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }
						Enqueue(&running_q, i);
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else {
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i);
						else {
							while (process[QPeek(&ready_q)]->remain_time_io > process[i]->remain_time_io) {
								Enqueue(&temp, Dequeue(&ready_q));
								if (QIsEmpty(&ready_q)) break;
							}
							Enqueue(&temp, process[i]->pid);
							while (!QIsEmpty(&ready_q)) {
								Enqueue(&temp, Dequeue(&ready_q));
							}
							while (!QIsEmpty(&temp)) {
								Enqueue(&ready_q, Dequeue(&temp));
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->remain_time_io > process[i]->remain_time_io) {
						Enqueue(&temp, Dequeue(&ready_q));
						if (QIsEmpty(&ready_q)) break;
					}
					Enqueue(&temp, process[i]->pid);
					while (!QIsEmpty(&ready_q)) {
						Enqueue(&temp, Dequeue(&ready_q));
					}
					while (!QIsEmpty(&temp)) {
						Enqueue(&ready_q, Dequeue(&temp));
					}
				}
			}
		}
		if (!QIsEmpty(&running_q)) {
			pick = QPeek(&running_q);
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick);
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_ljf.idle_time++; }
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {
				int waiting = Dequeue(&running_q);
				waiting_q[waiting] = process[waiting]->io_burst + 1;
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				p_ljf.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	p_ljf.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	p_ljf.avg_turnaround = (float)tot_turn / p_num;
	p_ljf.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", p_ljf.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", p_ljf.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void evaluation() {
	printf("[Evaluation of each scheduling algorithms]\n");
	printf("-----------------------------------------------------------------------------\n");
	printf("1. FCFS Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", fcfs.avg_waiting, fcfs.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((fcfs.finish_time - fcfs.idle_time) * 100) / fcfs.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("2. Non-preemtive SJF Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", np_sjf.avg_waiting, np_sjf.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((np_sjf.finish_time - np_sjf.idle_time) * 100) / np_sjf.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("3. Preemptive SJF Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", p_sjf.avg_waiting, p_sjf.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((p_sjf.finish_time - p_sjf.idle_time) * 100) / p_sjf.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("4. Non-preemptive Priority Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", np_pri.avg_waiting, np_pri.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((np_pri.finish_time - np_pri.idle_time) * 100) / np_pri.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("5. Preemptive Priority Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", p_pri.avg_waiting, p_pri.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((p_pri.finish_time - p_pri.idle_time) * 100) / p_pri.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("6. Round-Robin Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", rr.avg_waiting, rr.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((rr.finish_time - rr.idle_time) * 100) / rr.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("7. (Additional) Non-preemtive SJF Scheduling(I/O + CPU burst)\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", np_sjf_io.avg_waiting, np_sjf_io.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((np_sjf_io.finish_time - np_sjf_io.idle_time) * 100) / np_sjf_io.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("8. (Additional) Preemtive SJF Scheduling(I/O + CPU burst)\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", p_sjf_io.avg_waiting, p_sjf_io.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((p_sjf_io.finish_time - p_sjf_io.idle_time) * 100) / p_sjf_io.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("9. (Additional) Preemtive LJF Scheduling(I/O)\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", p_ljf.avg_waiting, p_ljf.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((p_ljf.finish_time - p_ljf.idle_time) * 100) / p_ljf.finish_time);
}
void main() {
	create_process();
	config();
	/*FCFS();
	NP_SJF();
	P_SJF();
	NP_PRIORITY();
	P_PRIORITY();*/
	RR();
	RR();
	RR();
	RR();
	RR();
	RR();
	RR();
	/*NP_SJF_IO();
	P_SJF_IO();
	P_LJF();
	evaluation();*/
}