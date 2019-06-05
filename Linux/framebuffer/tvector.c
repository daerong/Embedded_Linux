#include "../include/fpga_test.h"

/*
6�� ���� : ���� ����

C standard library�� �ִ� �Լ��� ���� �ȿ� 0���� 1023���� ���� ���� �����ϴ� ���α׷�
*/

int main(){
	FILE *fstream;
	int i;

	fstream = fopen("mmaptest.tst", "w+b");
	if (fstream == NULL){
		fprintf(stderr, "Cannot open mmaptest.tst\n");		// int fprintf( FILE *stream, const char *format, [, argument].. ); : ���� ���� ���� �����Լ�
															// => stream : ����� FILE ����ü�� ������, format : ���� ���� ���ڿ�, argument : �������� �Ű����� return : ����� ����Ʈ�� ��. ���� �� EOF(-1)
		exit(1);
	}

	for (i = 0; i < 1024; i++) {
		fwrite(&i, sizeof(i), 1, fstream);		// size_t fwrite( const void *buffer, size_t size, size_t count, FILE *stream ); : ����Ʈ ���� ���� ����
												// buffer : ���Ͽ� ������ ������ ������ ������, size_t : ����� �׸��� ������, count : ����� �׸��� ����, stream : ����� FILE ����ü ������, return : ������ �� ������ �׸��� ����(count), ���� �� count���� ���� ��
	}

	fclose(fstream);
	exit(0);
	return 0;
}