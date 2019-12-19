#include <stdio.h>
#include<string.h>
#define FILEHISTORY "history.txt"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>
#include <arpa/inet.h>

typedef struct 
{
	char player[20];
	int point_win;
	int point_lose;
}elementtype;
#include "linkedlist.h"
void displaySingleList(singleList list)
{
  if (list.root == NULL) return;
  printf("----------------DS--------------\n");
  list.cur = list.root;
  while(list.cur != NULL)
    {
      printf("%s %d %d \n",list.cur->element.player,list.cur->element.point_win,list.cur->element.point_lose);
      list.cur = list.cur->next;
    }
}
void getDatatoList(singleList *list,elementtype element){
	// make null the list
	//deleteSingleList(list);
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
int main(int argc, char const *argv[])
{

	singleList list;
	createSingleList(&list);
	elementtype element;
	getDatatoList(&list,element);
	displaySingleList(list);
	char name1[31]={0};
	//char name2[31]={0};
	//char number[31]={0};
	scanf("%s",name1);
	if (searchData(list, name1)==1){
		add_win_game(list, name1);
		displaySingleList(list);
	}
	else{
		add_new_player(name1);
		getDatatoList(&list,element);
		add_lose_game(list,name1);
		displaySingleList(list);
	}
	return 0;
}