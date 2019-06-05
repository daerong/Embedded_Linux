#include "../include/fpga_test.h"

/*
6번 예제 : 파일 쓰기

C standard library에 있는 함수로 파일 안에 0부터 1023까지 정수 값을 저장하는 프로그램
*/

int main(){
	FILE *fstream;
	int i;

	fstream = fopen("mmaptest.tst", "w+b");
	if (fstream == NULL){
		fprintf(stderr, "Cannot open mmaptest.tst\n");		// int fprintf( FILE *stream, const char *format, [, argument].. ); : 변수 단위 파일 쓰기함수
															// => stream : 개방된 FILE 구조체의 포인터, format : 형식 제어 문자열, argument : 가변적인 매개변수 return : 출력한 바이트의 수. 에러 시 EOF(-1)
		exit(1);
	}

	for (i = 0; i < 1024; i++) {
		fwrite(&i, sizeof(i), 1, fstream);		// size_t fwrite( const void *buffer, size_t size, size_t count, FILE *stream ); : 바이트 단위 파일 쓰기
												// buffer : 파일에 저장할 데이터 버퍼의 포인터, size_t : 출력할 항목의 사이즈, count : 출력할 항목의 개수, stream : 개방된 FILE 구조체 포인터, return : 실제로 쓴 데이터 항목의 개수(count), 에러 시 count보다 작은 수
	}

	fclose(fstream);
	exit(0);
	return 0;
}