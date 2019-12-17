#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "proto.h"
#include "string.h"

#define BOARD_SIZE 5

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
int board[BOARD_SIZE][BOARD_SIZE];
int array[BOARD_SIZE*BOARD_SIZE];
char nickname[LENGTH_NAME] = {};

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void recv_msg_handler() {
    char receiveMessage[LENGTH_SEND] = {};
    while (1) {
        int receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
        if (receive > 0) {
            printf("\r%s\n", receiveMessage);
            str_overwrite_stdout();
            int num = atoi(receiveMessage);
            board_with_0_number(receiveMessage);
            int count = check_winner(board);
            print_matrix(num);
            if(count == 3){
                printf("Winer\n");
            }
        } else if (receive == 0) {
            break;
        } else { 
            // -1 
        }
    }
}

void send_msg_handler() {
    char message[LENGTH_MSG] = {};
    while (1) {
        str_overwrite_stdout();
        while (fgets(message, LENGTH_MSG, stdin) != NULL) {
            str_trim_lf(message, LENGTH_MSG);
            if (strlen(message) == 0) {
                str_overwrite_stdout();
            } else {
                break;
            }
        }
        send(sockfd, message, LENGTH_MSG, 0);
        // int num = atoi(message);
        // print_matrix(num);
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }
    catch_ctrl_c_and_exit(2);
}

void board_with_0_number( int number)
{
    int i, j;
    
    for(i=0; i < BOARD_SIZE; i++)
    {
        for(j=0; j < BOARD_SIZE; j++)
        {
            if(board[i][j]==number)
                board[i][j]=0;
        }
    }
}

void print_matrix(int number)
{
    int i, j;

    //system("clear"); 
    printf("%c[1;33m",27); 
    printf("Your matrix:\n");
    printf("+----+----+----+----+----+\n"); 
    board_with_0_number(number);
    for(i=0; i < BOARD_SIZE; i++)
    {
        for(j=0; j < BOARD_SIZE; j++)
        {
            if(board[i][j]==0)
            {
                printf("| ");
                printf("%c[1;31m",27);
                printf("%2c ", 88); 
                printf("%c[1;33m",27);
            }
            else
                printf("| %2d ", board[i][j]); 
        }
        printf("|\n");
        printf("+----+----+----+----+----+\n"); 
    }      
    printf("%c[0m", 27);
}

int check_winner(int board[][BOARD_SIZE])
{
    int i;
    int count=0;

    for(i=0; i < BOARD_SIZE; i++)
    {
        if(board[i][0]==0&&board[i][1]==0&&board[i][2]==0&&board[i][3]==0&&board[i][4]==0) 
        {
            count++;
        }
        if(board[0][i]==0&&board[1][i]==0&&board[2][i]==0&&board[3][i]==0&&board[4][i]==0)
            count++;
    }
    if(board[0][0]==0&&board[1][1]==0&&board[2][2]==0&&board[3][3]==0&&board[4][4]==0)
        count++;
    if(board[0][4]==0&&board[1][3]==0&&board[2][2]==0&&board[3][1]==0&&board[4][0]==0)
        count++;
    return count;
}

int main()
{
    signal(SIGINT, catch_ctrl_c_and_exit);

    // Naming
    printf("Please enter your name: ");
    if (fgets(nickname, LENGTH_NAME, stdin) != NULL) {
        str_trim_lf(nickname, LENGTH_NAME);
    }
    if (strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("\nName must be more than one and less than thirty characters.\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }
    

    // Socket information
    struct sockaddr_in server_info, client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.sin_port = htons(8888);

    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *)&server_info, s_addrlen);
    if (err == -1) {
        printf("Connection to Server error!\n");
        exit(EXIT_FAILURE);
    }
    
    // Names
    getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
    getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

    send(sockfd, nickname, LENGTH_NAME, 0);

    // Nhap matric
    printf("Enter 25 numbers from 1 to 25\n");
    int m=0;
    for(int i = 0;i < BOARD_SIZE; ++i){
        for (int j = 0; j < BOARD_SIZE; ++j)
        {
            int check;
            do{
                check =0;
                printf("[%d, %d]= ", i,j);
                scanf("%d",&board[i][j]);
                array[m] = board[i][j];
                m++;
                for(int k = 0; k < m; ++k){
                    for (int h = 0; h < k; ++h)
                    {
                        if ((array[h] == array[k]) || (array[k] < 1) || (array[k] > 25)){
                            check = 1;
                        }
                    }
                }
                if (check ==1 ){
                    printf("Nhap lai\n");
                    --m;
                }
                    
            }while(check == 1);
        }
    }

    print_matrix(0);

    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if(flag) {
            printf("\nBye\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
