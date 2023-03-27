#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAXLINE 200

void TCP_Connect(int af, char *servip, unsigned int port, int *sock);
int FTP_put(int sock_msg, int sock_file);
int FTP_get(int sock_msg, int sock_file);
int FTP_ls(int sock_msg, int sock_file);
int FTP_pwd(int sock_msg, int sock_file);
int FTP_cd(int sock_msg);
void FTP_quit(int sock_msg);
void FTP_help();
int Client_cd();
int Client_pwd();
int Client_ls();
void FTP_print();

int main(int argc, char *argv[])
{
    struct sockaddr_in server;
    struct stat obj;
    int sock, sock1;
    char bufmsg[MAXLINE];
    int port;
    char ip[MAXLINE];
    int result;
    int pid_number;
    char msg[MAXLINE];
    printf("############################################################################ \n");
    printf("#                                                                          # \n");
    printf("#   #### ##### ###          #####  #        #   #####  ##    #  #######    # \n");
    printf("#   #      #   #  #         #      #        #   #      # #   #     #       # \n");
    printf("#   ###    #   ###  ######  #      #        #   #####  #  #  #     #       # \n");
    printf("#   #      #   #            #      #        #   #      #   # #     #       # \n");
    printf("#   #      #   #            #####  ######   #   #####  #    ##     #       # \n");
    printf("#                                                                          # \n");
    printf("############################################################################ \n");

    printf("\n");
    printf("버전 : Beta_0.0.5\n");
    printf("\n");
    printf("Team : BlueMint\n");
    printf("\n");
    printf("제작일 : 2023-03-25\n");
    printf("\n");
    printf("---------------------------------------------------------------------------- \n");
    printf("명령어 목록\n");
    printf("(1)get, (2)put, (3)pwd, (3)ls, (4)cd, (5)quit,\n (6)client_cd, (7)client_pwd, (8)client_ls, (9)help\n");
    printf("\n");
    printf("\n");

    while (1)
    {
        printf("접속할 FTP Server Port를 입력하세요 : ");
        scanf("%d", &port);
        // sock은 메시지 처리 sock1은 파일 처리
	printf("접속할 FTP Server IP Address를 입력하세요 : ");
	scanf("%s",ip);

	TCP_Connect(AF_INET, ip, port, &sock);
       	TCP_Connect(AF_INET, ip, port+1, &sock1);

        if (sock == -1 && sock1 == -1)
        {
            printf("FTP 연결 실패 !!\n");
            exit(1);
        }
        else
        {
            // 연결 완료메시지 & pid number 받음
	    recv(sock,msg,100,0);
            recv(sock, &pid_number, 100, 0);
            printf("%s\n", &msg);
            printf("pid_number : %d\n", pid_number);
            printf("%d - %d\n", sock, sock1);
            while (1)
            {
                printf("명령어를 입력하세요\n (1)get, (2)put, (3)pwd, (3)ls, (4)cd, (5)quit,\n (6)client_cd, (7)client_pwd, (8)client_ls, (9)help\n");
                FTP_print();
                scanf("%s", bufmsg);

                if ((send(sock, bufmsg, 100, 0)) < 0)
                {
                    perror("send: ");
                }
                if (!strcmp(bufmsg, "get"))
                {
                    printf("get을 입력하였습니다\n");
                    result = FTP_get(sock, sock1);
                    if (result == -1)
                    {
                        printf("get 명령어를 정상적으로 실행시키지 못했습니다.\n");
                    }
                    else
                    {
                        printf("get 명령어를 정상적으로 실행시켰습니다\n");
                    }
                }
                else if (!strcmp(bufmsg, "put"))
                {
                    printf("put을 입력하였습니다\n");
                    result = FTP_put(sock, sock1);
                    if (result == -1)
                    {
                        printf("put 명령어를 정상적으로 실행시키지 못했습니다.\n");
                    }

                    else
                    {
                        printf("put 명령어를 정상적으로 실행시켰습니다.\n");
		    }
		}

		    else if(!strcmp(bufmsg,"pwd"))
		    {
                        printf("pwd을 입력하였습니다\n");
                        result = FTP_pwd(sock, sock1);
                        if (result == -1)
                        {
                            printf("pwd 명령어를 정상적으로 실행시키지 못했습니다.\n");
                        }
                        else
                        {
                            printf("pwd 명령어를 정상적으로 실행하였습니다.\n");
                        }
                    }
		
                    else if (!strcmp(bufmsg, "ls"))
                    {
                        printf("ls을 입력하였습니다\n");
                        result = FTP_ls(sock, sock1);
                        if (result == -1)
                        {
                            printf("ls 명령어를 정상적으로 실행시키지 못했습니다.\n");
                        }
                        else
                        {
                            printf("ls 명령어를 정상적으로 실행했습니다.\n");
                        }
                    }
                    else if (!strcmp(bufmsg, "cd"))
                    {
                        printf("cd을 입력하였습니다\n");
                        result = FTP_cd(sock);
                        if (result == -1)
                        {
                            printf("cd 명령어를 정상적으로 실행시키지 못했습니다.\n");
                        }
                        else
                        {
                            printf("cd 명령어를 정상적으로 실행시켰습니다.\n");
                        }
                    }
                    else if (!strcmp(bufmsg, "quit"))
                    {
                        FTP_quit(sock);
                        if (result == -1)
                        {
                            printf("서버를 종료시키지 못했습니다.\n");
                        }
                        else
                        {
                            printf("서버를 종료합니다.\n");
                            exit(1);
                        }
                    }
                    else if (!strcmp(bufmsg, "client_cd"))
                    {
                        result = Client_cd();
                        if (result == -1)
                        {
                            printf("client_cd 명령어를 정상적으로 실행시키지 못했습니다.\n");
                        }
                        else
                        {
                            printf("client_cd 명령어를 정상적으로 실행시켰습니다.\n");
                        }
                    }
                    else if (!strcmp(bufmsg, "client_pwd"))
                    {
                        result = Client_pwd();
                        if (result == -1)
                        {
                            printf("client_pwd 명령어를 정상적으로 실행시키지 못했습니다.\n");
                        }
                        else
                        {
                            printf("client_pwd 명령어를 정상적으로 실행시켰습니다.\n");
                        }
                    }
                    else if (!strcmp(bufmsg, "client_ls"))
                    {
                        result = Client_ls();
                        if (result == -1)
                        {
                            printf("client_ls 명령어를 정상적으로 실행시키지 못했습니다.\n");
                        }
                        else
                        {
                            printf("client_ls 명령어를 정상적으로 실행시켰습니다.\n");
                        }
                    }
                    else if (!strcmp(bufmsg, "help"))
                    {
                        FTP_help();
                    }
                    else
                    {
                        printf("다시 입력해주세요\n");
                    }
                }
          }
       }
  }

// 연결 함수
void TCP_Connect(int af, char *servip, unsigned int port, int *sock)
{
    struct sockaddr_in servaddr;
    if ((*sock = socket(af, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }
    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = af;
    inet_pton(AF_INET, servip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    if (connect(*sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        return -1;
    }
    // return s;
}
// 업로드 함수
int FTP_put(int sock_msg, int sock_file)
{
    char filename[MAXLINE];
    int filehandle;
    char buf[20];
    struct stat obj;
    int size;
    int value;
    int status;

    send(sock_msg, "put", 100, 0);
    printf("업로드 할 파일을 입력해주세요\n");
    FTP_print();
    scanf("%s", filename);
    // 업로드 할 파일이 있는지 오픈
    filehandle = open(filename, O_RDONLY);
    if (filehandle == -1)
    {
        printf("파일이 없습니다.\n");
        return -1;
    } // 업로드 파일 메시지 전송
    send(sock_msg, filename, 100, 0);
    stat(filename, &obj);
    size = obj.st_size;
    // 업로드 파일 사이즈 전송
    send(sock_msg,&size,sizeof(int), 0);
    // 업로드 파일 내용 전송
    sendfile(sock_file, filehandle, NULL, size);
    if (status)
    {
        printf("업로드 완료\n");
        printf("업로드 실패\n");
    }
    return value;
}
// 파일 다운로드 함수
int FTP_get(int sock_msg, int sock_file)
{
    char buf[50];
    char filename[MAXLINE];
    char temp[MAXLINE];
    int size;
    int value;
    char *f;
    int filehandle;
    // 명령어를 보냄
    send(sock_msg, "get", 100, 0);

    printf("다운로드할 파일을 입력해주세요\n");
    FTP_print();
    scanf("%s", filename);
    printf("%s\n", filename);
    strcpy(temp, filename);
    // 파일 이름을 보냄
    send(sock_msg, temp, 50, 0);
    // 파일 사이즈를 받음
    recv(sock_msg, &size, sizeof(int), 0);
    // 파일 사이즈가 0일 경우
    if (!size)
    {
        printf("파일이 없습니다\n");
        return -1;
    }
    f = malloc(size);
    printf("size value : %d\n", size);
    // 파일을 받음
    recv(sock_file, f, size, 0);
    // 파일 생성
    while (1)
    {
        filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0777);
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
    printf("다운로드 완료!!\n");
    return value;
}
// FTP 경로
int FTP_pwd(int sock_msg, int sock_file)
{
    int value;
    char buf[100];
    int size;
    int filehandle;
    char *f;
    strcpy(buf, "pwd");
    value = send(sock_msg, buf, 100, 0);
    printf("send value : %d\n", value);
    value = recv(sock_msg, &size, sizeof(int), 0);
    f = malloc(size);
    recv(sock_file, f, size, 0);
    filehandle = open("pwd.txt", O_RDWR | O_CREAT, 0666);
    write(filehandle, f, size);
    close(filehandle);
    system("cat pwd.txt");
    return value;
}
// ls 명령어
int FTP_ls(int sock_msg, int sock_file)
{
    char buf[100];
    int size;
    int filehandle;
    int value;
    char *f;
    strcpy(buf, "ls");
    printf("%s\n", buf);
    send(sock_msg, buf, 100, 0);
    recv(sock_msg, &size, sizeof(int), 0);
    f = malloc(size);
    value = recv(sock_file, f, size, 0);
    printf("size : %d\n", size);
    printf("value : %d\n", value);
    filehandle = open("ls.txt", O_RDWR | O_CREAT, 0666);
    write(filehandle, f, size);
    close(filehandle);
    printf("디렉터리 목록\n");
    system("cat ls.txt");
    return value;
}
// cd 명령어
int FTP_cd(int sock_msg)
{
    char buf[100];
    char temp[20];
    int value;
    int status;
    send(sock_msg, "cd", 100, 0);
    strcpy(buf, "cd ");
    printf("이동할 경로 입력\n");
    FTP_print();
    scanf("%s", temp);
    strcat(buf, temp);
    printf("%s\n", buf);
    send(sock_msg, buf, 100, 0);
    recv(sock_msg, &status, sizeof(int), 0);
    printf("%d\n", status);
    if (status)
    {
        printf("경로 변경 완료\n");
    }
    else
    {
        printf("경로 변경 실패\n");
    }
    return value;
}
void FTP_quit(int sock_msg)
{ 
  // int status;
  // char buf[100];
  // strcpy(buf,"quit");
  // send(sock_msg,buf,100,0);
  // status = recv(sock_msg,&status,100,0);
  exit(0);
}
int Client_pwd()
{
    int result;
    result = system("pwd");
    return result;
}
int Client_cd()
{
    int result;
    char input[100];
    strcpy(input, "cd ");
    printf("이동하실 경로명을 입력하세요\n");
    FTP_print();
    scanf("%s", input + 3);
    if (chdir(input + 3) == 0)
    {
        result = 0;
    }
    else
    {
        result = -1;
    }
    return result;
}
void FTP_print()
{
    printf("ftp>");
}
int Client_ls()
{
    int result;
    printf("파일목록 \n");
    result = system("ls");
    return result;
}
void FTP_help()
{
    printf("사용법 : 각 명령어를 먼저 입력 한 후, 명령어 순서에 따라 처리, EX) get 처리 한 후 파일이름.확장자");
    printf("get은 서버에서 클라이언트로 파일을 다운로드 합니다.\n");
    printf("put은 클라이언트에서 서버로 파일을 업로드 합니다.\n");
    printf("pwd은 서버의 경로를 보여줍니다.\n");
    printf("ls는 서버의 파일 목록을 보여줍니다.\n");
    printf("cd는 서버의 경로를 이동합니다.\n");
    printf("quit는 서버를 종료합니다.\n");
    printf("client_cd는 클라이언트의 경로를 이동합니다.\n");
    printf("client_pwd는 클라이언트의 경로를 보여줍니다.\n");
    printf("client_ls는 클라이언트의 파일 목록을 보여줍니다.\n");
}
