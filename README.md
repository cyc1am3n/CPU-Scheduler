# CPU-Scheduler
![Structure](/asset/structure.png)

* 초기설정

	 프로세스를 생성하기 전에 프로세스의 최대 생성 개수를 10으로 설정하고, Queue의 자료구조를 바탕으로 ready_q(= Ready queue)와 running_q(= Running queue)를 선언하며, waiting_q(= Waiting queue)라는 크기가 10인 array를 선언한다. (Waiting queue 특성상 Queue의 자료구조를 채택할 수 없었다.)   
	 또한 각각의 CPU Scheduling 알고리즘을 평가하기 위해서 Evaluation 구조체인 fcfs, np_sjf, p_sjf, np_pri, p_pri, rr, np_sjf_io, p_sjf_io, p_ljf를 선언한다.

## 1) create_process()

프로세스를 생성한다. 프로세스는 1부터 10중에 입력받은 수만큼 생성하며 각 프로세서에 지정해준 데이터 값의 범위는 다음과 같고 범위 안에서 난수로 지정했다.  
  
● Process ID: 0 ~ 9 (프로세스를 선언한 순서대로)

● CPU Burst Time: 2 ~ 10

● I/O Burst Time: 1 ~ 5

● Arrival Time: 1 ~ 4 x # of processes

● Priority: 1 ~ # of processes (중복 불가)

● I/O Start time: 1 ~ n – 1 (각 CPU Burst Time 내에 발생)  
 Arrival time에 관한 설정 값에서 범위를 정적으로 놓는다면 프로세스의 개수에 따라서 idle되는 시간의 차이가 발생해서 여러 번의 실험을 거쳐서 4와 생성된 프로세스의 개수를 곱한 값만큼 지정했다.  
 또한 Process ID와 Priority를 제외한 값들은 모두 중복을 허용했다.

## 2) config()
 초기 설정에서 선언한 ready_q, waiting_q, running_q를 초기화 한다.  
 ready_q와 running_q에는 QueueInit()을 실행시켜주고, waiting_q에는 배열에 프로세스 개수만큼 –1을 넣어주었다.(waiting_q[i]에 –1이 들어있으면 pid가 i인 프로세스는 Waiting queue에 없다는 말이다.)

## 3) Initialize()
 각 CPU 스케줄링 알고리즘에서 각 프로세스가 작업을 얼마나 수행했는지를 표현하기 위한 progress와 종료 시점을 표현하는 comp_time을 0으로 초기화해준다.(두개 모두 global variable이다.)  
 스케줄링 알고리즘을 실행했을 때 처음으로 이 함수가 실행되어 그 전에 수행했던 알고리즘에 영향을 받지 않게 된다.

## 4) CPU Scheduling
 CPU Scheduling에 해당하는 함수들을 포괄해 이 부분에 넣었다.  
 모든 알고리즘에서는 공통적으로 프로세싱을 할 때 현재 시간을 나타내는 time 변수를 1씩 증가시키는데, 이를 마치는 시기는 Evaluation 구조체에서 각 프로세스가 끝날 때 마다 int형의 변수인 check를 1씩 증가시키며 check가 프로세스의 개수가 될 때이다.  
 또한 프로세스의 진행 중에 progress와 arrival time이 같아질 때 I/O operation이 발생했다고 하며, running queue에 있는 프로세스를 꺼내 Waiting queue에 넣고(waiting_q[i]에 I/O burst 할당) 시간이 지날 때마다 1씩 감소시켜 0이 되면 I/O operation을 종료한다.(waiting_q에서 ready_q나 running_q로 이동)

그림에서도 알 수 있듯이 9개의 알고리즘을 적용했으며, 설명의 편의를 위해서 선언한 순서를 조금 변형해 ⑥을 ① 다음으로 옮겨 소개하겠다. 

### ① FCFS (First Come, First Served)
 이 함수는 위에서도 설명했듯이 먼저 들어오는 프로세스를 먼저 수행하는 알고리즘이다.  
 각 프로세스는 arrival time이 time과 같아질 때 ready queue에 순차적으로 들어간다.  
 그리고 running queue가 비어있으면 ready queue가 비어있는지 확인해서 비어있다면 idle을 출력하고, 그렇지 않다면 queue의 가장 앞에 있는 프로세스를 꺼내 running queue에 넣는다.

### ⑥ RR (Round–Robin)
 이 함수는 FCFS에서 time quantum값만 추가한 알고리즘이라고 볼 수 있다.  
 이 함수가 실행되면 먼저 time quantum값을 사용자에게 입력받는다.(범위는 1~5)  
 그리고 각 프로세스의 progress값을 1씩 증가시키면서 동시에 time_quan값을 1씩 증가시키는데(time_quan은 Process 구조체의 변수), time_quan이 입력받은 time quantum과 같아지면 ready queue가 비어있다면 time_quan을 0으로 초기화만 하고, ready queue에 다른 프로세스가 존재한다면 현재 실행되고 있는 프로세스를 꺼내 ready queue에 넣고 ready queue의 프로세스를 running queue에 넣는다.

### ② NP_SJF (Non-preemtive Shortest Job First)
 여기에서는 프로세스 생성 시 할당해준 cpu burst time을 이용하는데, 임시 행렬 arr을 만들어서 cpu burst를 넣어준 뒤 bubble sort를 통해 cpu burst time이 적은 순서대로 정렬한다.  
 이를 통해 프로세스가 arrive할 때마다 ready queue에서 자신의 cpu burst에 맞는 위치에 들어가게 되며 running queue에 있는 프로세스가 끝나 running queue가 비면 그때 ready queue에 있는 프로세스가 running으로 들어간다.

### ③ P_SJF (Preemptive Shortest Job First)
 NP_SJF에서는 초기에 cpu burst를 sort한 반면에 여기에는 각 프로세스에 remain_time값을 넣어서 현재 프로세스가 끝날 때 까지 남은 시간을 나타내준다.  
 그리고 프로세스가 arrive할 때마다 현재 running queue에서 실행되고 있는 프로세스와 remain time을 비교해 새 프로세스의 remain time이 더 작다면(혹은 같으면서 새 프로세스의 pid가 작다면) running queue에 들어가게 되고(preemptive 발생) 거기에 있던 프로세스는 ready queue의 최상단으로 가게 되며, 그렇지 않다면 새 프로세스의 cpu burst 가 작다면 ready queue에서 자신의 cpu burst에 맞는 위치에 들어가게 된다.(temp Queue를 정의해서 구현)

### ④ NP_PRIORITY (Non-preemptive Priority) ⑤ P_PRIORITY (Preemptive Priority)
 기본적으로 이 두 함수는 작동방식이 유사하므로 같이 소개하겠다.  
 P_SJF에서 프로세스의 위치를 정하는 기준이 remain time이었다면, 여기서는 그 값이 priority로 대체된 것 밖에 달라진 것이 없다.  
 또한 프로세스가 들어왔을 때 곧장 running queue와 비교하면 preemptive인 것이고 ready queue에 들어가면 non-preemptive인 것이라고 볼 수 있을 것이다.

### ⑦ NP_SJF_IO  ⑧ P_SJF_IO
 작동방식은 여기에 쓰인 알고리즘 NP_SJF, P_SJF와 같다.  
 하지만 NP_SJF와 P_SJF에서의 판단기준이 cpu_burst, remain_time이었다면 여기에서는 I/O burst time까지 함께 고려했다는 차이점이 있다.
 
### ⑨ P_LJF
 지금까지 구현했던 SJF에서는 CPU burst time을 고려해주었다면 여기에서는 CPU Utilization을 높이는 것에 집중해서 I/O burst time이 짧은 순서대로 먼저 프로세싱을 진행한다는 특징이 있다.  
 이 알고리즘도 P_SJF와 작동방식은 흡사하다.  

## 5) Evaluation()

 각 스케줄링 알고리즘을 평가한다.  
 초기설정에서 선언한 Evaluation 구조체는 각각 스케줄링 알고리즘 ① ~ ⑨를 수행하면서 total turnaround time과 total waiting time을 저장하고, 전체 프로세스가 모두 수행되는 시간과 idle한 시간의 비율 또한 저장시켜놓는다.  
 그 값을 이 함수를 통해 CPU Utilization(추가구현)으로 변환시키는데 그 식은 다음과 같다.  
 
	“ (finish_time – idle_time) * 100) / finish_time ”  

이렇게 각 구조체에 저장된 total turnaround, waiting time과 finish time, idle time을 이용해 CPU 스케줄링 알고리즘의 평가기준인 ATT(Average Turnaround Time)와 AWT(Average Waiting Time), CPU Utilization을 계산해 출력하는 기능을 한다.
