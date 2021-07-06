// server
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <string.h>

#define MAX 1024  // max client data length
#define PORT 2224 // server port #
#define BACKLOG 5 // queue length

void *threadHandle(void *arg);
pthread_mutex_t mutex;

/* 스레드 전달인자 구조체*/
struct multipleArg
{
    int m_nsd; // new socket descriptor
    int m_num; // client number
};

/* 공유 데이터 */
char kv[100][2][51] = {
    0,
};
int datanum = 0; // 공유 데이터 갯수, 100이 되면 꽉찬 것

int main(int argc, char *argv[])
{
    pthread_t p[50]; // 50개 스레드 선언
    void *result_t = NULL;

    int sd, nsd, pid, bytes, cliaddrsize;
    struct sockaddr_in cliaddr, servaddr;
    char data[MAX];

    struct multipleArg *multiple_arg;
    multiple_arg = malloc(sizeof(struct multipleArg));

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "can’t open socket.\n");
        exit(1);
    }
    // to bind the server itself to the socket
    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "can’t bind to socket.\n");
        exit(1);
    } // bind itself to the socket

    listen(sd, BACKLOG); // declare the client-queue length

    /* 멀티 스레드*/
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < 50; i++)
    {
        cliaddrsize = sizeof(cliaddr);
        if ((nsd = accept(sd, (struct sockaddr *)&cliaddr, &cliaddrsize)) < 0)
        {
            fprintf(stderr, "can’t accept connection.\n");
            exit(1);
        } // upon return: client addr. is known and a new socket is created

        /* 스레드 생성, 구조체 보냄 */
        multiple_arg->m_nsd = nsd;
        multiple_arg->m_num = i;
        pthread_create(&p[i], NULL, threadHandle, (void *)multiple_arg);
    }
    for (int i = 0; i < 50; i++)
    {
        /* 스레드 종료 */
        pthread_join(p[i], &result_t);
    }

    free(multiple_arg);
    pthread_mutex_destroy(&mutex);
    close(sd); // listen socket 종료

    return 0;
}
void *threadHandle(void *arg)
{
    struct multipleArg *my_multiple_arg = (struct multipleArg *)arg;
    int bytes, nsd, num;
    char data[MAX];
    char tp[MAX];

    // 전달받은 소켓 구조체
    nsd = my_multiple_arg->m_nsd;
    ;
    num = my_multiple_arg->m_num;
    char op[50] = {
        0,
    },
         key[51] = {
             0,
         },
         value[51] = {
             0,
         };

    while (1)
    {
        /* memory 초기화*/
        for (int i = 0; i < 50; i++)
            op[i] = key[i] = value[i] = 0;
        memset(tp, 0, MAX);
        memset(data, 0, MAX);

        /* 클라이언트에게서 데이터 받아옴 */
        bytes = recv(nsd, data, MAX, 0); // from client

        if (bytes == 0) // client quit
            break;
        else if (bytes < 0)
        { // error
            fprintf(stderr, "can’t receive data.\n");
            exit(1);
        }

        /* 데이터 분할*/
        sscanf(data, "%s %s %s", op, key, value);
        printf("client %d : %s %s %s\n", num, op, key, value);
        fflush(NULL);

        if (!strcmp("quit", op))
        { // 0. quit을 받은 경우 quit_ack 전달
            char quit_ack[20] = "quit_ack";
            if (send(nsd, quit_ack, strlen(quit_ack), 0) != strlen(quit_ack))
            { // echo back
                fprintf(stderr, "can’t send data.\n");
                exit(1);
            }
        }

        pthread_mutex_lock(&mutex);
        /*************** critical section ****************/
        /* 요청 처리*/
        if (!strcmp("read", op))
        { // 1. read
            int find = 0;
            for (int i = 0; i < 100; i++)
            {
                if (!strcmp(kv[i][0], key))
                {
                    sprintf(tp, "%d read: %s %s\n", num, key, kv[i][1]);
                    find = 1;
                    break;
                }
            }
            if (find != 1)
            {
                sprintf(tp, "%d read failed\n", num);
            }
        }
        else if (!strcmp("update", op))
        { // 2. update
            int find = 0;
            for (int i = 0; i < 100; i++)
            {
                if (!strcmp(kv[i][0], key))
                {
                    sprintf(kv[i][1], "%s", value);
                    sprintf(tp, "%d update OK: %s %s\n", num, kv[i][0], kv[i][1]);
                    find = 1;
                    break;
                }
                if (find != 1)
                {
                    sprintf(tp, "%d update Failed\n", num);
                }
            }
        }
        else if (!strcmp("insert", op))
        { // 3. insert
            if (datanum == 100)
            { // 꽉 찬 경우
                sprintf(tp, "%d insert Failed", num);
            }
            else
            {
                sprintf(kv[datanum][0], "%s", key);
                sprintf(kv[datanum][1], "%s", value);
                sprintf(tp, "%d insert OK: %s %s\n", num, kv[datanum][0], kv[datanum][1]);
                datanum += 1;
            }
        }
        /*************** critical section ****************/
        pthread_mutex_unlock(&mutex);

        /* 클라이언트에게 데이터 전송*/
        if (send(nsd, tp, strlen(tp), 0) != strlen(tp))
        { // echo back
            fprintf(stderr, "can’t send data.\n");
            exit(1);
        }
    }
    close(nsd);
}
