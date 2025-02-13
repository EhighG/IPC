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

struct memory {
	char buff[100];
	int status, pid1, pid2;
};

struct memory* shmptr;

// clientB로부터 받은 메시지를 출력하는 함수
void handler(int signum)
{
	// user1이 clientB로부터 메시지를 받은 경우
	if (signum == SIGUSR1) {
		// shmptr 포인터가 가리키는 구조체의 멤버변수 buff에 저장된 메시지를 출력
		printf("\nclientB로부터의 메시지: %s\n", shmptr->buff);

		// 수신한 문자열이 exit일 때
		if (strcmp(shmptr->buff, "exit\n") == 0) {
			printf("==== 프로그램을 종료합니다. ====\n");
			exit(0);
		}
	}
}

int main()
{
	int pid = getpid(); // 현재 프로세스(user1)의 pid
	int shmid;
	int key = 12345; // 공유 메모리의 키 값

	// 공유 메모리 생성
	shmid = shmget(key, sizeof(struct memory), IPC_CREAT | 0666);

	// 공유 메모리를 현재 프로세스의 가상 주소 공간과 연결
	// 마지막 인자 0의 의미 : 플래그를 사용하지 않음
	shmptr = (struct memory*)shmat(shmid, NULL, 0);

	// 공유 메모리에 프로세스 id 저장, 준비되지 않은 상태(NOT_READY) 저장
	shmptr->pid1 = pid;
	shmptr->status = NOT_READY;

	// handler 이용한 시그널 처리
	signal(SIGUSR1, handler);

	while (1) {
		while (shmptr->status != READY)
			continue;
		sleep(1);
		// 메시지 입력받기
		printf("입력 대기: \n");
		// 버퍼에 메시지 저장, 최대길이, 문자열 읽어들일 파일 포인터
		fgets(shmptr->buff, 100, stdin);

		shmptr->status = FILLED;

		// kill함수로 clientB에게 메시지 전송
		kill(shmptr->pid2, SIGUSR2);

		if (strcmp(shmptr->buff, "exit\n") == 0) {
			printf("==== 프로그램을 종료합니다. ====\n");
			exit(0);
			break;
		}
	}

	// 사용 종료: 공유 메모리와 연결 해제
	shmdt((void*)shmptr);

	// 공유 메모리 세그먼트 제거
	shmctl(shmid, IPC_RMID, NULL);
	return 0;
}