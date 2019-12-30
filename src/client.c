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


#define BOARD_SIZE 5

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
int chon;
int back;
int board[BOARD_SIZE][BOARD_SIZE];
int array[BOARD_SIZE*BOARD_SIZE];
char nickname[LENGTH_NAME] = {};
char buff[1024];

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void board_with_0_number( int number)// ham chuyen so duoc chon sang 0
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

void print_matrix(int number)// ham in ra bang voi cac so duoc chon duoc danh dau "X"
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

int check_winner(int board[][BOARD_SIZE])// ham kiem tra dieu kien thang
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

char *string_split(char buff[]){// ham cat chuoi
    static char p[2];
    int index = 0;
    for (int i = 0; i < strlen(buff); ++i)
    {
        if (buff[i]>=48 && buff[i]<=57){
            p[index] = buff[i];
            index ++;
        }
    }
    return p;
}

void recv_msg_handler() {
    char receiveMessage[LENGTH_SEND] = {};
    char *winnerMessage = "BINGO!";
    char *receiveNumber;
    while (1) {
        int receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
        if (receive > 0) {
            printf("\r%s\n", receiveMessage);
            if(strcmp(receiveMessage, "Game Over!")==0){
            	back =1;
            	break;
            }
            str_overwrite_stdout();
            receiveNumber = string_split(receiveMessage);
            int num = atoi(receiveNumber);
            receiveNumber[0] = "";
            receiveNumber[1] = "";
            board_with_0_number(num);
            int count = check_winner(board);
            print_matrix(num);
            if(count == 3){
                printf(" BINGO! You're Winer\n");
                send(sockfd, winnerMessage, strlen(winnerMessage),0);
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
            //check message
            str_trim_lf(message, LENGTH_MSG);

            if (strlen(message) == 0) {
                str_overwrite_stdout();
            } else {
                break;
            }
        }
        send(sockfd, message, LENGTH_MSG, 0);
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }
    catch_ctrl_c_and_exit(2);
}

void menu(){
    // system("clear"); 
    printf("%c[1;33m",27); 
    printf("\n\n||'''''\\  || ||\\     ||  /'''''\\    /'''''\\ \n");
    printf("||     || || || \\    || ||         ||     || \n");
    printf("||,,,,,/  || ||  \\   || ||         ||     || \n");
    printf("||     \\  || ||   \\  || ||    =''= ||     || \n");
    printf("||     || || ||    \\ || ||     ||  ||     || \n");
    printf("||,,,,,/  || ||     \\||  \\,,,,,/    \\ ,,,,/  \n\n");
    printf("Choose 1-3:\n");
    printf("1. Start game\n");
    printf("2. Show user's achievements\n");
    printf("3. Exit\n");
}


int main(int argc, char const *argv[])
{
    signal(SIGINT, catch_ctrl_c_and_exit);
    if (argc != 2){
        fprintf(stderr,"Use command line parameters for the port number of server: %s <PortNumber>\n", argv[0]);
        return 1;
    }

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
    server_info.sin_port = htons(atoi(argv[1]));
    char *achieveMessage = "Show achievements";
  

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

    //Menu
	
    do {
    	back = 0;
        menu();
        scanf("%d",&chon);
        if (chon == 1){
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
	            if(back == 1)
	            	break;
	        }

                
        }

        if (chon == 2){
            send(sockfd, achieveMessage, strlen(achieveMessage),0);
            // recv(sockfd, buff, 1024, 0);
            recv(sockfd, buff, 1024, 0);
            printf("%s\n",buff );
            back = 1;

        }

        if (chon ==3){
            catch_ctrl_c_and_exit(2);
        }
        if(flag) {
	        printf("\nBye\n");
	        break;
	    }
//*********


    } while(chon > 3 || back == 1);

    close(sockfd);
    return 0;

}
