#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#define AAD "aaaaaaaaaaaaaaa"
#define rkey "ddddddddddddddd"
#define riv "iiiiiiiiiii"
void set_enc(int sock_msg,int sock_file,unsigned char* plaintext);
void set_dec(int sock_msg,int sock_file,unsigned char* buf);
int aes_gcm_encrypt(const unsigned char *plaintext, int plaintext_len,
		const unsigned char *key, const unsigned char *iv, int iv_len,
		unsigned char *ciphertext, unsigned char *tag,const unsigned char *aad);
int aes_gcm_decrypt(const unsigned char *ciphertext, int ciphertext_len,
		const unsigned char *key, const unsigned char *iv, int iv_len,
		const unsigned char *tag, unsigned char *plaintext,const unsigned char* aad);
int tcp_listen(int host, int port, int backlog);
int tcp_ls(int sock_msg, int sock_file);
int tcp_get(int sock_msg, int sock_file);
int tcp_put(int sock_msg, int sock_file);
int tcp_pwd(int sock_msg, int sock_file);
int tcp_cd(int sock_msg);
void exitClient();
void main_select(int sock_msg, int sock_file);

// 파일디스크립터 기록 
int filehandle;

// 유저수 카운트
int num_user = 0;

pid_t pid;
pid_t pid1;
struct sigaction sact;
sigset_t mask;
struct sockaddr_in servaddr;

int main(int argc, char *argv[])
{
	struct sockaddr_in server, client;
	int sock1, sock2, sock3, sock4;
	int input, child, status;
	int len, n, result;
	char buf[100];
	int host;

	printf("################################################################### \n");
	printf("#                                                                 # \n");
	printf("#   #### ##### ###            ###  ####  ###   #  #  ####  ###    # \n");
	printf("#   #      #   #  #          #     #     #  #  #  #  #     #  #   # \n");
	printf("#   ###    #   ###  ######    ##   ###   ###   #  #  ###   ###    # \n");
	printf("#   #      #   #                #  #     # #    ##   #     # #    # \n");
	printf("#   #      #   #             ###   ####  #  #   ##   ####  #  #   # \n");
	printf("#                                                                 # \n");	
	printf("################################################################### \n");

	printf("\n");
	printf("버전 : Beta_0.0.5\n");
	printf("\n");
	printf("Team : BlueMint\n");
	printf("\n");
	printf("제작일 : 2023-03-25\n");
	printf("\n");
	printf("-------------------------------------------------------------------\n");
	printf("\n");
	printf("\n");
	printf("Port 번호를 입력하세요 : ");
	scanf("%d", &input);
	host = htonl(203<<24|230<<16|102<<8|41);

	// sock1,sock3는 메시지 전송 sock2, sock4는 파일 전송
	// tcp_listen은 소켓을 반환함. 
	sock1 = tcp_listen(host, input,5);
	sock2 = tcp_listen(host, input + 1, 6);

	//sock1 = tcp_listen(INADDR_ANY, input,5);
	//sock2 = tcp_listen(INADDR_ANY, input + 1, 6);
	len = sizeof(client);
	if ((pid = fork()) != 0)
	{
		exit(0);
	}
	else
	{
		while (1)
		{
			sock3 = accept(sock1, (struct sockaddr *)&client, &len);
			sock4 = accept(sock2, (struct sockaddr *)&client, &len);

			if(sock3 < 0)
				continue;

			//연결이 유지되면 유저 수 증가
			num_user++;
			pid1 = fork();

			if (pid1 < 0)
			{
				printf("자식 프로세스 생성 실패\n");
				return -1;
			}
			else if (pid1 > 0)
			{
				// 연결 되었다 메시지 보내기 
				send(sock3,"FTP가 연결되었습니다!",100,0); 
				child = getpid();
				printf("pid number : %d\n", child);

				// 현재 연결된 PID 번호 전송 
				send(sock3,&child,100,0);
				printf("user count %d\n", num_user);
				printf("%d Client main 들어옴!\n", num_user);
				main_select(sock3, sock4);
			}
		}
	}
	return 0;
}

int tcp_listen(int host, int port, int backlog)
{
	int sd = 0;
	sd = socket(AF_INET, SOCK_STREAM, 0);

	if (sd == -1)
	{
		perror("socket fail!!");
		exit(1);
	}

	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = host;
	servaddr.sin_port = htons(port);

	if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("bind fail!!");
		exit(1);
	}

	listen(sd, backlog);
	return sd;
}

// ls 명령어 함수
int tcp_ls(int sock_msg, int sock_file)
{
	int fd;
	unsigned char plaintext[1024];
	
	memset(plaintext,0x00,sizeof(plaintext));
	system("ls > ls.txt");
	fd = open("ls.txt",O_RDONLY);
	read(fd,plaintext,sizeof(plaintext));
	close(fd);
	set_enc(sock_msg,sock_file,plaintext);
	printf("\n");

	return 0;
}

// 파일 다운로드
int tcp_get(int sock_msg, int sock_file)
{
	char filename[50];
	struct stat obj;
	int filehandle;
	int result;
	char buffer[2048];
	int size;

	// 파일 이름을 받음 
	recv(sock_msg,filename,50,0);
	printf("file name : %s\n", filename);
	stat(filename, &obj);
	filehandle = open(filename, O_RDONLY);
	size = obj.st_size;
	printf("파일 이름 받음!!\n");

	// 파일이 없을 경우 
	if(filehandle == -1)
	{
		size = 0;
		printf("파일 size 0\n");   
		return -1;
	}
	// 파일이 있을 경우 파일 사이즈를 보냄
	send(sock_msg, &size, sizeof(int),0);
	printf("파일 사이즈 보내기 완료!!\n");
	printf("file size : %d\n,size");
	// 파일이 0이 아닐 경우 파일을 보냄   
	if (size)
	{
		result = sendfile(sock_file, filehandle, NULL, size);
		printf("파일 보내기 완료!!\n");
	}

	// 파일을 닫음 
	close(filehandle);

	return 0;
}

// 파일 업로드
int tcp_put(int sock_msg, int sock_file)
{
	int result;
	int c = 0;
	int len;
	char *f;
	int size;
	int filehandle;
	char filename[50];

	sleep(5);
	// 업로드 할 파일 이름을 받음 
	recv(sock_msg, filename, 100,0);

	// 파일 이름명
	printf("file name : %s\n", filename);

	// 파일 사이즈를 받음 
	recv(sock_msg, &size, sizeof(int),0);

	if (!size)
	{
		printf("파일이 없습니다\n");
		return -1;
	}

	f = malloc(size); // 파일을 받음 
	recv(sock_file, f, size,0);

	// 파일 기록 
	while(1)
	{
		filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);

		if (filehandle == -1)
		{
			sprintf(filename + strlen(filename), "_1");
		}
		else
		{
			break;
		}
	}
	write(filehandle, f, size);
	close(filehandle);

	printf("업로드 완료!!\n");

	return result;
}

// FTP Server 경로
int tcp_pwd(int sock_msg, int sock_file)
{
	int fd;
	unsigned char plaintext[1024];
	
	memset(plaintext,0x00,sizeof(plaintext));
	system("pwd>pwd.txt");
	fd = open("pwd.txt",O_RDONLY);
	read(fd,plaintext,sizeof(plaintext));
	close(fd);
	set_enc(sock_msg,sock_file,plaintext);
	printf("\n");
	return 0;
}

void set_enc(int sock_msg,int sock_file,unsigned char* plaintext){
	int plaintext_len;
	int ciphertext_len;
	// 암호문과 태그 저장할 변수
	unsigned char ciphertext[1024];
	// 추가로 16바이트의 MAC(인증 태그) 공간
	unsigned char tag[16];
	// 인증 태그 길이는 128비트(16바이트)
	
	plaintext_len = strlen(plaintext);
	// AES-GCM 암호화
	ciphertext_len = aes_gcm_encrypt(plaintext, plaintext_len,rkey, riv, sizeof(riv),ciphertext, tag,AAD);
	send(sock_msg,&ciphertext_len,sizeof(int),0);
	send(sock_msg,ciphertext,ciphertext_len,0);
	send(sock_msg,tag,sizeof(tag),0);
}

// cd 명령어
int tcp_cd(int sock_msg)
{
	int value;
	int c;
	char buf_value[100];

	sleep(5);
	recv(sock_msg, buf_value, 100, 0);

	printf("%s\n", buf_value);

	if (chdir(buf_value + 3) == 0)
	{
		c = 1;
	}
	else
	{
		c = 0;
	}

	value = send(sock_msg, &c, sizeof(int), 0);

	return value;
}

// 나간 클라이언트 
void exitClient()
{
	num_user--;
	exit(0);
}

void main_select(int sock_msg, int sock_file)
{
	unsigned char buf[1024];
	int result = 0;
	char result1[15];
	char client_char[100];
	char msg[50];

	sprintf(result1, "%d", num_user);

	// 접속 기록 쓰기 
	strcpy(msg,"Client_Number "); 
	strcat(msg,result1); 
	strcat(msg, " Connect!! , ");
	memset(buf,0x00,sizeof(buf));
	filehandle = open("Story.txt", O_CREAT | O_APPEND | O_RDWR, 0666);
	write(filehandle, msg, strlen(msg));
	close(filehandle);

	while (1)
	{
		//recv(sock_msg, buf, 100, 0);
		set_dec(sock_msg,sock_file,buf);
		//recv(sock_msg, buf, 100, 0);
		printf("입력받은 명령어 : %s\n", buf);
		printf("명령어 size %d\n", strlen(buf));

		if (!strcmp(buf, "ls"))
		{ // ls
			result = tcp_ls(sock_msg, sock_file);

			if (result == -1)
			{
				printf("ls 명령어 실패 !!\n");
			}
			else
			{
				printf("ls 명령어 성공 !!\n");
			}
		}
		else if (!strcmp(buf, "get"))
		{ // get
			result = tcp_get(sock_msg, sock_file);
			if (result == -1)
			{
				printf("파일 다운로드 실패!!\n");
			}
			else
			{
				printf("파일 다운로드 성공!!\n");
			}
		}
		else if (!strcmp(buf, "put"))
		{ 
			// put
			result = tcp_put(sock_msg, sock_file);

			if (result == -1)
			{
				printf("파일 업로드 실패!!\n");
			}
			else
			{
				printf("파일 업로드 성공!!\n");
			}
		}
		else if (!strcmp(buf, "pwd"))
		{ 
			// pwd
			result = tcp_pwd(sock_msg, sock_file);
			if (result == -1)
			{
				printf("pwd 명령어 실행 실패 !!\n");
			}
			else
			{
				printf("pwd 명령어 실행 성공 !!\n");
			}
		}
		else if (!strcmp(buf, "cd"))
		{ 
			// cd
			result = tcp_cd(sock_msg);

			if (result == -1)
			{
				printf("cd 명령어 실행 실패 !!\n");
			}
			else
			{
				printf("cd 명령어 실행 성공 !!\n");
			}
		}
		else if (!strcmp(buf, "quit"))
		{ 
			// quit
			exitClient();
		}
		strcpy(client_char, "Client_Number");
		strcat(client_char, result1);
		strcat(client_char, " : ");
		strcat(client_char, buf);
		strcat(client_char, " success , ");

		//패킷이 오류 여부 기록 
		if(sock_msg==-1)
		{
			strcat(client_char,"fail!!, ");
		}
		else
		{
			strcat(client_char,"success, ");
		}

		// 명령어 기록 쓰기
		filehandle = open("Story.txt", O_CREAT | O_APPEND | O_RDWR, 0666);
		write(filehandle, client_char, strlen(client_char));
		close(filehandle);
		memset(buf,0x00,sizeof(buf));
	}
}
// AES-GCM 암호화 함수
int aes_gcm_encrypt(const unsigned char *plaintext, int plaintext_len,
		const unsigned char *key, const unsigned char *iv, int iv_len,
		unsigned char *ciphertext, unsigned char *tag,const unsigned char* aad) {
	EVP_CIPHER_CTX *ctx;
	int len, ciphertext_len;

	// Context 생성 및 초기화
	if (!(ctx = EVP_CIPHER_CTX_new())) {
		return -1;
	}

	// 암호화 키, 암호화 알고리즘 및 모드 설정
	if (EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return -1;
	}

	// 암호화 키와 IV 설정
	if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return -1;
	}
	EVP_EncryptUpdate(ctx, NULL, &len, aad, strlen(aad));

	// 평문 암호화
	if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return -1;
	}
	ciphertext_len = len;

	// 인증 태그 생성
	if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return -1;
	}
	ciphertext_len += len;

	// 인증 태그 추출
	if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return -1;
	}

	// Context 해제
	EVP_CIPHER_CTX_free(ctx);

	return ciphertext_len;
}

// AES-GCM 복호화 함수
int aes_gcm_decrypt(const unsigned char *ciphertext, int ciphertext_len,
		const unsigned char *key, const unsigned char *iv, int iv_len,
		const unsigned char *tag, unsigned char *plaintext,const unsigned char* aad) {
	EVP_CIPHER_CTX *ctx;
	int len, plaintext_len;
	int ret;

	// Context 생성 및 초기화
	if (!(ctx = EVP_CIPHER_CTX_new())) {
		return -1;
	}

	// 복호화 키, 암호화 알고리즘 및 모드 설정
	if (EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return -1;
	}

	// 복호화 키와 IV 설정
	if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return -1;
	}
	EVP_DecryptUpdate(ctx, NULL, &len, aad, strlen(aad));

	// 복호화 수행
	if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return -1;
	}
	plaintext_len = len;

	// 인증 태그 설정
	if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void *)tag) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return -1;
	}

	// 복호화 완료 여부 확인
	ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

	// Context 해제
	EVP_CIPHER_CTX_free(ctx);

	if (ret > 0) {
		plaintext_len += len;
		return plaintext_len;
	} else {
		return -1; // 인증 실패
	}
}
void set_dec(int sock_msg,int sock_file,unsigned char* buf){
	unsigned char ciphertext[1024]; //암호문이 담길 문자열 배열
	unsigned char tag[16]; //태그가 담길 문자열 배열
	unsigned char decrypted_text[1024]; //복호화된 평문이 담길 배열
	int ciphertext_len; //암호문 길이
	int decrypted_len; //복호화된 평문 길이

	recv(sock_msg,&ciphertext_len,sizeof(int),0); //서버에서 암호문의 길이 받음
	recv(sock_msg,ciphertext,ciphertext_len,0); //서버에서 암호문을 받음
	recv(sock_msg,tag,sizeof(tag),0); //서버에서 태그를 받음
	decrypted_len = aes_gcm_decrypt(ciphertext, ciphertext_len,
			rkey,riv, sizeof(riv),
			tag,decrypted_text,AAD); //복호화
	decrypted_text[decrypted_len] = '\0'; //문장끝을 알림
	printf("복호화 완료\n");
	strcpy(buf,decrypted_text);
	printf("buf -> %s\n",buf);
}
