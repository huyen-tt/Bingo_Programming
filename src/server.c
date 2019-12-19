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
#include "server.h"
#define FILEHISTORY "history.txt"
typedef struct 
{
    char player[20];
    int point_win;
    int point_lose;
}elementtype;
#include "linkedlist.h"


// Global variables
int server_sockfd = 0, client_sockfd = 0;
int lastest_player = 0;
ClientList *root, *now;
void getDatatoList(singleList *list,elementtype element){
    // make null the list
    deleteSingleList(list);
    FILE *fp;
    fp = fopen(FILEHISTORY,"r");
    // counting the length of this file
    char c;
    int count =0;
    for (c = getc(fp); c != EOF; c = getc(fp)) 
    {
        if (c == '\n') // Increment count if this character is newline 
            count = count + 1; 
    }
    fclose(fp);
    // read the data to list 
    fp = fopen(FILEHISTORY,"r");

    for(int i=0;i< count + 1;i++){
        fscanf(fp, "%s %d %d", element.player, &element.point_win, &element.point_lose);
        insertEnd(list,element);
    }  
    fclose(fp);
}
int searchData(singleList list, char username[30])  {  
    list.cur = list.root; 
    while (list.cur != NULL)  
    {  
        if (strcmp(list.cur->element.player, username) == 0)  
            return 1;  
        list.cur = list.cur->next;
    }  
    return 0;  
} 
void add_new_player(char ten[30]){
    FILE *fp;
    fp = fopen(FILEHISTORY,"a");
    fprintf(fp,"\n%s 0 0",ten);
    fclose(fp);
} 
void add_win_game(singleList list,char ten[30]){
    // Change the status of username in list to 0
    list.cur = list.root;
    while(list.cur != NULL){
        if (strcmp(list.cur->element.player, ten) == 0){
            list.cur->element.point_win ++;
            break;
        }
        list.cur = list.cur->next;
    }
    list.cur = list.root;
    // rewrite all list to file
    FILE *fp;
    fp = fopen(FILEHISTORY,"w");
    int i =0;
    while(list.cur!=NULL){
        if(i == 0){
            fprintf(fp,"%s %d %d",list.cur->element.player, list.cur->element.point_win, list.cur->element.point_lose);
        }
        else{
            fprintf(fp,"\n%s %d %d",list.cur->element.player, list.cur->element.point_win, list.cur->element.point_lose);           
        }
        i++;
        list.cur = list.cur->next;
    }
    fclose(fp);
}
void add_lose_game(singleList list,char ten[30]){
    // Change the status of username in list to 0
    list.cur = list.root;
    while(list.cur != NULL){
        if (strcmp(list.cur->element.player, ten) == 0){
            list.cur->element.point_lose ++;
            break;
        }
        list.cur = list.cur->next;
    }
    list.cur = list.root;
    // rewrite all list to file
    FILE *fp;
    fp = fopen(FILEHISTORY,"w");
    int i =0;
    while(list.cur!=NULL){
        if(i == 0){
            fprintf(fp,"%s %d %d",list.cur->element.player, list.cur->element.point_win, list.cur->element.point_lose);
        }
        else{
            fprintf(fp,"\n%s %d %d",list.cur->element.player, list.cur->element.point_win, list.cur->element.point_lose);           
        }
        i++;
        list.cur = list.cur->next;
    }
    fclose(fp);
}
void catch_ctrl_c_and_exit(int sig) {
    ClientList *tmp;
    while (root != NULL) {
        printf("\nClose socketfd: %d\n", root->data);
        close(root->data); // close all socket include server_sockfd
        tmp = root;
        root = root->link;
        free(tmp);
    }
    printf("Bye\n");
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(ClientList *np, char tmp_buffer[]) {
    ClientList *tmp = root->link;
    while (tmp != NULL) {
        // if (np->data != tmp->data) { // all clients except itself.
            printf("Send to sockfd %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
       // }
        tmp = tmp->link;
    }
}

void client_handler(void *p_client) {
    int leave_flag = 0;
    char nickname[LENGTH_NAME] = {};
    char recv_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = {};
    ClientList *np = (ClientList *)p_client;
    singleList list;
    createSingleList(&list);
    elementtype element;
    getDatatoList(&list,element);
 
    // Naming
    if (recv(np->data, nickname, LENGTH_NAME, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("%s didn't input name.\n", np->ip);
        leave_flag = 1;
    } else {
        strncpy(np->name, nickname, LENGTH_NAME);
        printf("%s(%s)(%d) join the BINGO game.\n", np->name, np->ip, np->data);
        sprintf(send_buffer, "%s(%s) join the BINGO game.", np->name, np->ip);
        if (searchData(list,np->name)==0){
            add_new_player(np->name);
            getDatatoList(&list,element);
        }
        else{
            // add_win_game(list,np->name);
            // add_win_game(list,np->name);
            // add_win_game(list,np->name);
            // add_lose_game(list,np->name);
        }
        send_to_all_clients(np, send_buffer);
    }

    // Conversation
    while (1) {

        if (leave_flag) {
            break;
        }
        int receive = recv(np->data, recv_buffer, LENGTH_MSG, 0);
        if (receive > 0) {
            if (strlen(recv_buffer) == 0) {
                continue;
            }
            sprintf(send_buffer, "%s",recv_buffer);
            if (lastest_player != np->data){
                sprintf(send_buffer, "%s", recv_buffer);
            }
            else{
                printf("Not turn!\n");
                continue;
            }
        } else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
            printf("%s(%s)(%d) leave the game.\n", np->name, np->ip, np->data);
            sprintf(send_buffer, "%s(%s) leave the game.", np->name, np->ip);
            leave_flag = 1;
        } else {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        send_to_all_clients(np, send_buffer);
        lastest_player = np->data;
    }

    // Remove Node
    close(np->data);
    if (np == now) { // remove an edge node
        now = np->prev;
        now->link = NULL;
    } else { // remove a middle node
        np->prev->link = np->link;
        np->link->prev = np->prev;
    }
    free(np);
}
int main()
{

    signal(SIGINT, catch_ctrl_c_and_exit);

    // Create socket
    server_sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (server_sockfd == -1) {
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
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(8888);

    // Bind and Listen
    bind(server_sockfd, (struct sockaddr *)&server_info, s_addrlen);
    listen(server_sockfd, 5);

    // Print Server IP
    getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Start Server on: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));

    // Initial linked list for clients
    root = newNode(server_sockfd, inet_ntoa(server_info.sin_addr));
    now = root;

    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);

        // Print Client IP
        getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        printf("Client %s:%d come in.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

        // Append linked list for clients
        ClientList *c = newNode(client_sockfd, inet_ntoa(client_info.sin_addr));
        c->prev = now;
        now->link = c;
        now = c;
        pthread_t id;
        if (pthread_create(&id, NULL, (void *)client_handler, (void *)c) != 0) {
            perror("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }

    }

    return 0;
}
