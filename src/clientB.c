#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define FILLED 0
#define READY 1
#define NOT_READY -1

// 메모리 구조체
struct memory {
	char buff[100];
	int status, pid1, pid2;
};

// 공유 메모리를 저장할 memory 구조체의 전역변수 생성
struct memory* shmptr;

void handler(int signum)
{
	printf("\nA) ");
	puts(shmptr->buff);

	// 수신한 메시지가 exit일 경우
	if (strcmp(shmptr->buff, "exit\n") == 0) {
		printf("==== 프로그램을 종료합니다. ====\n");
		exit(0);
	}
}

int main()
{
	int pid = getpid();
	int shmid;

	int key = 12345;

	// 공유 메모리 생성
	shmid = shmget(key, sizeof(struct memory), IPC_CREAT | 0666);

	// 공유 메모리를 프로세스의 가상 주소 공간과 연결
	shmptr = (struct memory*)shmat(shmid, NULL, 0);

	// 공유 메모리에 현 프로세스(clientB)의 pid 저장, 준비되지 않은 상태(NOT_READY) 저장
	shmptr->pid2 = pid;
	shmptr->status = NOT_READY;

	signal(SIGUSR2, handler);

	printf("채팅을 시작합니다.\n");

	while (1) {
		sleep(1);

		// 메시지 입력받기
		printf("입력 대기: \n");
		fgets(shmptr->buff, 100, stdin);

		shmptr->status = READY;
		kill(shmptr->pid1, SIGUSR1);

		// 입력받은 메시지가 exit일 때
		if (strcmp(shmptr->buff, "exit\n") == 0) {
			printf("\n==== 프로그램을 종료합니다. ====\n");
			exit(0);
			break;
		}
		while (shmptr->status == READY)
			continue;
	}

	// 사용 종료: 공유 메모리와 연결 해제
	shmdt((void*)shmptr);
	return 0;
}