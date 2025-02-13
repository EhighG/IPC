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

// �޸� ����ü
struct memory {
	char buff[100];
	int status, pid1, pid2;
};

// ���� �޸𸮸� ������ memory ����ü�� �������� ����
struct memory* shmptr;

void handler(int signum)
{
	// clientA�κ��� �޽��� ����
	if (signum == SIGUSR2) {
		printf("\n���� ���... \n");
		printf("A) ");
		puts(shmptr->buff);

		// ���Ź��� �޽����� exit�� ���
		if (strcmp(shmptr->buff, "exit\n") == 0) {
			printf("==== ���α׷��� �����մϴ�. ====\n");
			exit(0);
		}
	}
}

int main()
{
	int pid = getpid();
	int shmid;

	int key = 12345;

	// ���� �޸� ����
	shmid = shmget(key, sizeof(struct memory), IPC_CREAT | 0666);

	// ���� �޸𸮸� ���μ����� ���� �ּ� ������ ����
	shmptr = (struct memory*)shmat(shmid, NULL, 0);

	// ���� �޸𸮿� �� ���μ���(clientB)�� pid ����, �غ���� ���� ����(NOT_READY) ����
	shmptr->pid2 = pid;
	shmptr->status = NOT_READY;

	signal(SIGUSR2, handler);

	while (1) {
		sleep(1);

		// �޽��� �Է¹ޱ�
		printf("�Է� ���: \n");
		fgets(shmptr->buff, 100, stdin);

		shmptr->status = READY;
		kill(shmptr->pid1, SIGUSR1);

		// �Է¹��� �޽����� exit�� ��
		if (strcmp(shmptr->buff, "exit\n") == 0) {
			printf("==== ���α׷��� �����մϴ�. ====\n");
			exit(0);
			break;
		}
		while (shmptr->status == READY)
			continue;
	}

	// ��� ����: ���� �޸𸮿� ���� ����
	shmdt((void*)shmptr);
	return 0;
}