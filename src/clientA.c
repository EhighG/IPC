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

// clientB�κ��� ���� �޽����� ����ϴ� �Լ�
void handler(int signum)
{
	// user1�� clientB�κ��� �޽����� ���� ���
	if (signum == SIGUSR1) {
		// shmptr �����Ͱ� ����Ű�� ����ü�� ������� buff�� ����� �޽����� ���
		printf("\nclientB�κ����� �޽���: %s\n", shmptr->buff);

		// ������ ���ڿ��� exit�� ��
		if (strcmp(shmptr->buff, "exit\n") == 0) {
			printf("==== ���α׷��� �����մϴ�. ====\n");
			exit(0);
		}
	}
}

int main()
{
	int pid = getpid(); // ���� ���μ���(user1)�� pid
	int shmid;
	int key = 12345; // ���� �޸��� Ű ��

	// ���� �޸� ����
	shmid = shmget(key, sizeof(struct memory), IPC_CREAT | 0666);

	// ���� �޸𸮸� ���� ���μ����� ���� �ּ� ������ ����
	// ������ ���� 0�� �ǹ� : �÷��׸� ������� ����
	shmptr = (struct memory*)shmat(shmid, NULL, 0);

	// ���� �޸𸮿� ���μ��� id ����, �غ���� ���� ����(NOT_READY) ����
	shmptr->pid1 = pid;
	shmptr->status = NOT_READY;

	// handler �̿��� �ñ׳� ó��
	signal(SIGUSR1, handler);

	while (1) {
		while (shmptr->status != READY)
			continue;
		sleep(1);
		// �޽��� �Է¹ޱ�
		printf("�Է� ���: \n");
		// ���ۿ� �޽��� ����, �ִ����, ���ڿ� �о���� ���� ������
		fgets(shmptr->buff, 100, stdin);

		shmptr->status = FILLED;

		// kill�Լ��� clientB���� �޽��� ����
		kill(shmptr->pid2, SIGUSR2);

		if (strcmp(shmptr->buff, "exit\n") == 0) {
			printf("==== ���α׷��� �����մϴ�. ====\n");
			exit(0);
			break;
		}
	}

	// ��� ����: ���� �޸𸮿� ���� ����
	shmdt((void*)shmptr);

	// ���� �޸� ���׸�Ʈ ����
	shmctl(shmid, IPC_RMID, NULL);
	return 0;
}