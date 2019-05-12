/*
	gcc -pthread -o play player.c -lmpg123 -lao
	./play

	1. fitur play nama lagu
	baby_shark.mp3

	2. pause
	pause ->pause
	play ->play lagi

	3. next
	next

	4. prev
	prev

	5. list lagu
	list
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <ao/ao.h>
#include <mpg123.h>
#include <sys/wait.h>
#include<sys/types.h>

#define BITS 8
#define MAX 1000

char cmd[MAX];
int status =0;

//-----------------------------linked list---------------------------------

// Structure of a Node
struct Node
{
    char* data;
    struct Node *next;
    struct Node *prev;
};

// Function to insert at the end
void insertEnd(struct Node** start, char* value)
{
    // If the list is empty, create a single node
    // circular and doubly list
    if (*start == NULL)
    {
        struct Node* new_node = malloc(sizeof(struct Node));
	new_node->data = malloc(strlen(value)+1);
	strcpy(new_node->data, value);
        new_node->next = new_node->prev = new_node;
        *start = new_node;
        return;
    }

    // If list is not empty

    /* Find last node */
    struct Node *last = (*start)->prev;

    // Create Node dynamically
    struct Node* new_node = malloc(sizeof(struct Node));
	new_node->data = malloc(strlen(value)+1);
	strcpy(new_node->data, value);

    // Start is going to be next of new_node
    new_node->next = *start;

    // Make new node previous of start
    (*start)->prev = new_node;

    // Make last preivous of new node
    new_node->prev = last;

    // Make new node next of old last
    last->next = new_node;
}

void display(struct Node* start)
{
    struct Node *temp = start;

    while (temp->next != start)
    {
        printf("%s\n", temp->data);
        temp = temp->next;
    }
    printf("%s\n", temp->data);
}



int cari(struct Node* start)
{
    struct Node *temp = start;

    while (temp->next != start)
    {
	if(strcmp(temp->data, cmd)==0)
		return 1;

        temp = temp->next;
    }
	if(strcmp(temp->data, cmd)==0){
		return 1;
	}

	return 0;
}

void next(struct Node* start, char* title)
{

    struct Node *temp = start;

    while (temp->next != start)
    {
	if(strcmp(temp->data, title)==0)
		break;

        temp = temp->next;
    }
	if(strcmp(temp->data, title)==0){
		strcpy(title, temp->next->data);
	}

}


void prev(struct Node* start, char* title)
{

    struct Node *temp = start;

    while (temp->next != start)
    {
	if(strcmp(temp->data, title)==0)
		break;

        temp = temp->next;
    }
	if(strcmp(temp->data, title)==0){
		strcpy(title, temp->prev->data);
	}

}

//-----------------------------music player-----------------------------------

void * play (void * ptr){
	char *title;
	title = (void*)ptr;
	status =1;
    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;
    int tanda=1;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;

    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    mpg123_open(mh, title);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

    /* decode and play */
    while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK){
	if(strcmp(cmd, "pause") == 0){
	    memset(cmd, 0, strlen(cmd));
	    while(strcmp(cmd, "play") != 0){
		    sleep(1);
	    }
	    memset(cmd, 0, strlen(cmd));
	}
	if(strcmp(cmd, "stop") == 0){
	    /* clean up */
	    free(buffer);
	    ao_close(dev);
	    mpg123_close(mh);
	    mpg123_delete(mh);
	    mpg123_exit();
	    ao_shutdown();
	    tanda =0;
	    strcpy(cmd, "lanjut");
	    break;
	}
        ao_play(dev, buffer, done);
    }
	if(tanda == 1){
	 	/* clean up */
	   	 free(buffer);
	 	  ao_close(dev);
		  mpg123_close(mh);
		  mpg123_delete(mh);
		  mpg123_exit();
		  ao_shutdown();
	}
	status =0;
}

//---------------------------------input user---------------------------

void * command (void * ptr){
	while (1){
		scanf("%s", cmd);
	}
}

//-------------------------------main----------------------------------

int main(void)
{
	pthread_t thread1, thread2;
	char judul[MAX]="";

	int flag=2;

    /* Start with the empty list */
    struct Node* start = NULL;

    DIR *directory = opendir("/home/nitama/fp/music");

    if(directory == NULL)
        return -1;

    struct dirent *ent;

     while ((ent = readdir (directory)) != NULL)
     {
	const size_t len = strlen(ent->d_name);
	if(ent->d_name[len - 4] == '.' && ent->d_name[len - 3] == 'm' &&
        	ent->d_name[len - 2] == 'p' && ent->d_name[len - 1] == '3'){
		insertEnd(&start, ent->d_name);}
     }

    	if(closedir(directory) < 0)
        return -1;

	pthread_create (&thread2, NULL, command, NULL);

	while(1){
		if(strstr(cmd, ".mp3") != NULL){
			flag = cari(start);
			if (flag == 1){
				strcpy(judul, cmd);
				if(status == 1){
					strcpy(cmd, "stop");
					while(strcmp(cmd, "stop")==0){
					}
				}
				char title[MAX];
				strcpy(title, "music/");
				strcat(title, judul);
				pthread_create (&thread1, NULL, play, (void *) title);
				flag = 0;
			}
			else{

				printf("Lagu tidak ada dalam playlist\n");
			}
			memset(cmd, 0, strlen(cmd));
		}
		if(strcmp(cmd, "list") == 0){
			printf("\n");
			printf("-----------------\n");
			display(start);
			printf("-----------------");
			printf("\n\n");
			memset(cmd, 0, strlen(cmd));

		}
		if(strcmp(cmd, "next")==0){
			char temp[strlen(judul)];
			strcpy(temp, judul);

			if(strcmp(temp, "") == 0){
				printf("play salah satu lagu dulu lah\n");
				memset(cmd, 0, strlen(cmd));
			}
			else{
				strcpy(cmd, "stop");
				while(strcmp(cmd, "stop")==0){
				}

				next(start, judul);
				char title[MAX];
				strcpy(title, "music/");
				strcat(title, judul);
				pthread_create (&thread1, NULL, play, (void *) title);
				memset(cmd, 0, strlen(cmd));
			}
		}
		if(strcmp(cmd, "prev")==0){
			char temp[strlen(judul)];
			strcpy(temp, judul);
			if(strcmp(temp, "") == 0){
				printf("play salah satu lagu dulu lah\n");
				memset(cmd, 0, strlen(cmd));
			}
			else{
				strcpy(cmd, "stop");
				while(strcmp(cmd, "stop")==0){
				}
				prev(start, judul);
				char title[MAX];
				strcpy(title, "music/");
				strcat(title, judul);
				pthread_create (&thread1, NULL, play, (void *) title);
				memset(cmd, 0, strlen(cmd));
			}
		}

	}

//	pthread_join(thread1, NULL);
//	pthread_join(thread2, NULL);

    return 0;
}
