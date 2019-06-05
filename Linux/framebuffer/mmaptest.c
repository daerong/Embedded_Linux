#include "../include/fpga_test.h"

/*
7번 예제 : 일반적인 mmap 시스템 호출

lseek으로 offset을 조정하는 방식은 프로그래밍이 매우 피곤합니다. 좀 더 좋은 방법으로 mmap 시스템 호출이 있습니다. 
"파일 읽기 쓰기를 메모리 읽기 쓰기와 동일하게 할 수 없을까?" 라는 요구에 부응하는 것이 mmap 시스템 호출입니다.
mmap 시스템 호출은 file descriptor로 표현되는 object(개체, 타겟)를 application의 virtual address상에 매핑(mapping)시켜주는 시스템 호출입니다.
쉽게 생각해서 파일을 열어서 mmap하면 read/write를 포인터로 할 수 있다고 알아두면 쉽습니다.

mmap이 무엇을 해주는 가? 
내용을 보면 6번 예제와 동일한 파일(mmaptest.tst)를 open하고 offset 0부터 4096byte만큼을 READ/WRITE가 가능하도록 mmap합니다.
mmap이 리턴한 address는 integer 포인터 pmmap에 assign하고 1024개 만큼 그 포인터에서 읽어서 출력하면서 포인터로 지정된 어드레스에 있는 값을 바꾸는 프로그램.
mmap이 리턴하는 어드레스는 virtual address입니다. 그 address 값 자체는 큰 의미를 지니지 못합니다. CPU의 MMU가 그것을 physical address로 바꿀 때 어떤 일이 일어나겠지만요.

munmap은 무엇인가? 
mmap의 역함수(?)와 같은 함수. mmap()으로 만들어진 매핑을 제거하기 위한 시스템 콜

1. tvector.c를 수행하여 파일을 만들기.
2. mmaptest.c를 수행시키면 0부터 1023까지의 정수를 차례로 출력.
3. 다시 mmaptest.c를 수행하면 이제는 1024부터 2047까지의 값을 출력. => 전에 수행시켰던 mmaptest.c가 pmmap 포인터를 이용하여 파일에 저장된 내용을 바꾸었으니까 당연한 것이죠.

~ 파일 내용을 write 시스템 호출을 이용하지 않고 포인터를 이용하여 바꾸는데 성공

** 여기서 잠깐
file descriptor로 지정될 수 있는 object는 상당히 많이 있습니다. 당연히 일반적인 파일이 지정될 수 있죠. 
또한 socket, IPC(Inter Process Communication)에서 사용하는 pipe, fifo 등도 역시 file descriptor로 지정될 수 있음.
또한 device special file(mknod로 만들어진)을 open하면 device도 지정될 수 있겠죠. 

사실 file descriptor로 지정된 모든 것이 mmap을 지원하는 것은 아닙니다. 
예를 들어 serial port device와 같은 경우는 모든 데이터가 순서대로 읽어지고 써지는 구조를 가지고 있는데 이런 경우에는 lseek도 의미가 없고 mmap도 되지 않을 겁니다. 
Serial device를 mmap하여 접근하는 것 자체가 우스운 일이죠.(혹 가능하면 글쓴이만 우스워지므로 가능하다는 사실을 알고 있으면 조용히 메일을 보내줄 것.) 
mmap을 지원하지 않은 object라면 mmap 시스템 호출에 -1를 리턴하고 errno를 적당한 값으로 채워줄 겁니다.

Frame buffer 이야기를 하다가 왜 일반적인 mmap 이야기가 나왔을까요?
frame buffer device도 file descriptor로 지정될 수 있고 일반적으로 mmap을 지원합니다. 
따라서 lseek/write 시스템 호출에서 벗어나서 그냥 포인터로 접근을 해서 화면에 출력될 pixel 값을 바꾼다던지 화면에 출력된 pixel 값을 읽어 올 수 있습니다. 

lseek/write 시스템 호출을 사용하는 것은 나중에 살펴보겠지만 엄청 무식한 방법입니다. 
단, 모든 frame buffer가 mmap을 지원하지는 않는 것 같습니다. 
커널 소스의 Documentation/fb/tgafb.txt를 보면 DECChip 21030 칩 기반 그래픽 카드는 mmap을 지원하지 않는 것으로 나와 있습니다. 
아마도 시스템의 구조상 버퍼 메모리를 CPU의 virtual address로 매핑할 수 없기 때문일 것으로 생각됩니다. 
하지만 일반적인 frame buffer라면 대부분 mmap을 지원하고 mmap을 해서 사용하는 것이 일반적입니다. 
StrongARM의 LCD도 당연히 mmap을 지원할 겁니다. 왜냐하면 시스템 메모리의 일부를 LCD 버퍼로 이용하므로 virtual address로의 매핑이 쉽게 가능하기 때문이죠.

== 마치며
이번 글부터는 소스코드에 nl을 이용하여 줄 숫자를 적는다거나 하는 작업을 하지 않을 겁니다. 
어짜피 KELP에서 잘 보이지 않네요. 잘 알아서 보길 바랍니다.
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

	pmmap = (int *)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);	// void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset); : start부터 length까지의 영역을 fd에 대응한다.
																				// start : 특별한 경우가 아니면 0, length : 메모리로 매핑할 크기, prot : 맵핑에 원하는 메모리 보호정책, flags : 매핑 유형과 동작 구성 요소, fd : file descriptor, offset : 매핑할 때 length의 시작점을 지정, return : Address
	if ((unsigned)pmmap == (unsigned)-1)
	{
		perror("mmap");
		exit(1);
	}

	for (i = 0; i < 1024; i++)
	{
		printf("%d\n", pmmap[i]); /* 읽어서 출력 */
		pmmap[i] = pmmap[i] + 1024; /* 값을 바꿈 */
	}

	munmap(pmmap, 4096);		// int *munmap(void *addr, size_t length); : mmap()으로 만들어진 매핑을 제거하기 위한 시스템 콜, addr에서 시작하는 length 길이의 매핑을 해제한다.
	close(fd);
	exit(0);
	return 0;
}