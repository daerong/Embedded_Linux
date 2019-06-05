#include "../include/fpga_test.h"

/*
7�� ���� : �Ϲ����� mmap �ý��� ȣ��

lseek���� offset�� �����ϴ� ����� ���α׷����� �ſ� �ǰ��մϴ�. �� �� ���� ������� mmap �ý��� ȣ���� �ֽ��ϴ�. 
"���� �б� ���⸦ �޸� �б� ����� �����ϰ� �� �� ������?" ��� �䱸�� �����ϴ� ���� mmap �ý��� ȣ���Դϴ�.
mmap �ý��� ȣ���� file descriptor�� ǥ���Ǵ� object(��ü, Ÿ��)�� application�� virtual address�� ����(mapping)�����ִ� �ý��� ȣ���Դϴ�.
���� �����ؼ� ������ ��� mmap�ϸ� read/write�� �����ͷ� �� �� �ִٰ� �˾Ƶθ� �����ϴ�.

mmap�� ������ ���ִ� ��? 
������ ���� 6�� ������ ������ ����(mmaptest.tst)�� open�ϰ� offset 0���� 4096byte��ŭ�� READ/WRITE�� �����ϵ��� mmap�մϴ�.
mmap�� ������ address�� integer ������ pmmap�� assign�ϰ� 1024�� ��ŭ �� �����Ϳ��� �о ����ϸ鼭 �����ͷ� ������ ��巹���� �ִ� ���� �ٲٴ� ���α׷�.
mmap�� �����ϴ� ��巹���� virtual address�Դϴ�. �� address �� ��ü�� ū �ǹ̸� ������ ���մϴ�. CPU�� MMU�� �װ��� physical address�� �ٲ� �� � ���� �Ͼ��������.

munmap�� �����ΰ�? 
mmap�� ���Լ�(?)�� ���� �Լ�. mmap()���� ������� ������ �����ϱ� ���� �ý��� ��

1. tvector.c�� �����Ͽ� ������ �����.
2. mmaptest.c�� �����Ű�� 0���� 1023������ ������ ���ʷ� ���.
3. �ٽ� mmaptest.c�� �����ϸ� ������ 1024���� 2047������ ���� ���. => ���� ������״� mmaptest.c�� pmmap �����͸� �̿��Ͽ� ���Ͽ� ����� ������ �ٲپ����ϱ� �翬�� ������.

~ ���� ������ write �ý��� ȣ���� �̿����� �ʰ� �����͸� �̿��Ͽ� �ٲٴµ� ����

** ���⼭ ���
file descriptor�� ������ �� �ִ� object�� ����� ���� �ֽ��ϴ�. �翬�� �Ϲ����� ������ ������ �� ����. 
���� socket, IPC(Inter Process Communication)���� ����ϴ� pipe, fifo � ���� file descriptor�� ������ �� ����.
���� device special file(mknod�� �������)�� open�ϸ� device�� ������ �� �ְ���. 

��� file descriptor�� ������ ��� ���� mmap�� �����ϴ� ���� �ƴմϴ�. 
���� ��� serial port device�� ���� ���� ��� �����Ͱ� ������� �о����� ������ ������ ������ �ִµ� �̷� ��쿡�� lseek�� �ǹ̰� ���� mmap�� ���� ���� �̴ϴ�. 
Serial device�� mmap�Ͽ� �����ϴ� �� ��ü�� �콺�� ������.(Ȥ �����ϸ� �۾��̸� �콺�����Ƿ� �����ϴٴ� ����� �˰� ������ ������ ������ ������ ��.) 
mmap�� �������� ���� object��� mmap �ý��� ȣ�⿡ -1�� �����ϰ� errno�� ������ ������ ä���� �̴ϴ�.

Frame buffer �̾߱⸦ �ϴٰ� �� �Ϲ����� mmap �̾߱Ⱑ ���������?
frame buffer device�� file descriptor�� ������ �� �ְ� �Ϲ������� mmap�� �����մϴ�. 
���� lseek/write �ý��� ȣ�⿡�� ����� �׳� �����ͷ� ������ �ؼ� ȭ�鿡 ��µ� pixel ���� �ٲ۴ٴ��� ȭ�鿡 ��µ� pixel ���� �о� �� �� �ֽ��ϴ�. 

lseek/write �ý��� ȣ���� ����ϴ� ���� ���߿� ���캸������ ��û ������ ����Դϴ�. 
��, ��� frame buffer�� mmap�� ���������� �ʴ� �� �����ϴ�. 
Ŀ�� �ҽ��� Documentation/fb/tgafb.txt�� ���� DECChip 21030 Ĩ ��� �׷��� ī��� mmap�� �������� �ʴ� ������ ���� �ֽ��ϴ�. 
�Ƹ��� �ý����� ������ ���� �޸𸮸� CPU�� virtual address�� ������ �� ���� ������ ������ �����˴ϴ�. 
������ �Ϲ����� frame buffer��� ��κ� mmap�� �����ϰ� mmap�� �ؼ� ����ϴ� ���� �Ϲ����Դϴ�. 
StrongARM�� LCD�� �翬�� mmap�� ������ �̴ϴ�. �ֳ��ϸ� �ý��� �޸��� �Ϻθ� LCD ���۷� �̿��ϹǷ� virtual address���� ������ ���� �����ϱ� ��������.

== ��ġ��
�̹� �ۺ��ʹ� �ҽ��ڵ忡 nl�� �̿��Ͽ� �� ���ڸ� ���´ٰų� �ϴ� �۾��� ���� ���� �̴ϴ�. 
��¥�� KELP���� �� ������ �ʳ׿�. �� �˾Ƽ� ���� �ٶ��ϴ�.
*/

int main()
{
	int fd;
	int *pmmap;
	int i;

	fd = open("mmaptest.tst", O_RDWR);
	if (fd < 0)
	{
		perror("open");
		exit(1);
	}

	pmmap = (int *)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);	// void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset); : start���� length������ ������ fd�� �����Ѵ�.
																				// start : Ư���� ��찡 �ƴϸ� 0, length : �޸𸮷� ������ ũ��, prot : ���ο� ���ϴ� �޸� ��ȣ��å, flags : ���� ������ ���� ���� ���, fd : file descriptor, offset : ������ �� length�� �������� ����, return : Address
	if ((unsigned)pmmap == (unsigned)-1)
	{
		perror("mmap");
		exit(1);
	}

	for (i = 0; i < 1024; i++)
	{
		printf("%d\n", pmmap[i]); /* �о ��� */
		pmmap[i] = pmmap[i] + 1024; /* ���� �ٲ� */
	}

	munmap(pmmap, 4096);		// int *munmap(void *addr, size_t length); : mmap()���� ������� ������ �����ϱ� ���� �ý��� ��, addr���� �����ϴ� length ������ ������ �����Ѵ�.
	close(fd);
	exit(0);
	return 0;
}