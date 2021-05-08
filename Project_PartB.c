#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#define MAX_WORD_SIZE 32
#define BUFFER_SIZE 512


//This struct will be used to store the words
typedef struct word {

	char *str;	//Points at str
	int freq;	//This keeps up with how many times a word wasused               
	struct word *pNext;	//Makes it link to the next struct     

	} Word;		//Calls this word

void swap(struct word *a, struct word *b); //Swap function
void bubbleSort(struct word *pStart);	//This will sort the linked list
int getWord(FILE *fp, char *buf, int bufsize);	//This function will be used to get words from   
void addWord(char *pWord);	//Function will add words to linked list and count how often they are used
void Display(Word *pWordcounter);	//This will traverse the linked list
Word* createWordCounter(char *word);	//This creats a counter for  
void printList(struct word *pStart);

Word *pStart = NULL;	/* start with empty linked list */                  
int wordCount = 0;	//Counts total words                  
int differentwords = 0;	//Counts different words               
char buf[MAX_WORD_SIZE];	//sets character buffer size

pthread_mutex_t mutex;	//mutex lock

FILE *fp;

/******************************************************************************************************/

/* Function to print words in a given linked list */
void printList(struct word *pStart){
	
    struct word *temp = pStart;
    printf("\n");
    while (temp!=NULL){
        printf("\n%s %d ",temp->str, temp->freq);
        temp = temp->pNext;
    }
}

/******************************************************************************************************/

/*function used to show the words*/
void Display(Word *pWordcounter){
	printf("\n%-30s   %5d", pWordcounter->str,pWordcounter->freq);

}//end Display

/******************************************************************************************************/

/*looplist is used to loop the linked list and add the total amount of different words*/
void loopList(){
    
	Word *pCounter = NULL;
	pCounter = pStart;
    
	while(pCounter != NULL){
	
		pthread_mutex_lock(&mutex);
		differentwords++;
		pCounter = pCounter->pNext;
		pthread_mutex_unlock(&mutex);     
		
	}//end while

}//endlooplist

/******************************************************************************************************/

/*Add Word Function*/
void addWord(char *word){

	Word *pCounter = NULL;	//
	Word *pLast = NULL;	//
  

	if(pStart == NULL){

		pStart = createWordCounter(word);
		return;
	}//End if pStart == NULL

  	pCounter = pStart;
  
	while(pCounter != NULL){
    

		/*if the words are the same*/
		if(strcmp(word, pCounter->str) == 0){
      
      		pCounter->freq++;//Add one to the freq in the struct
      
      		return;

		}//Ends if the words are the same

    	pLast = pCounter;            
    	pCounter = pCounter->pNext;  
  	}//Ends while
  
  
	pLast->pNext = createWordCounter(word);

}//Ends add word

/******************************************************************************************************/

/*This is for new words*/
Word* createWordCounter(char *word){

	Word *pCounter = NULL;
	pCounter = (Word*)malloc(sizeof(Word));
	pCounter->str = (char*)malloc(strlen(word)+1);	//Allocates spaces for the word

	
	
		strcpy(pCounter->str, word);	//Copies the new word
	
	//pthread_mutex_lock(&mutex);
		pCounter->freq = 1;		//Sets the frequency of the new word to 1
	//pthread_mutex_unlock(&mutex);
	
	pCounter->pNext = NULL;		//Moves to next link
	

	return pCounter;

}//Ends the counter for new words

/******************************************************************************************************/

/*This gets the next word out of the file*/
int getWord(FILE *fp, char *buf, int bufsize){

	char *p = buf;
	char c;

	do{
		c = fgetc(fp);
		if (c == EOF) 
		return 0;
	} while (!isalpha(c));

	do{

		if (p - buf < bufsize - 1)
		*p++ = tolower(c);
		c = fgetc(fp);
	
	} while (isalpha(c));

	*p = '\0';
	return 1;

}//end getWord

/******************************************************************************************************/

/*generate will add words to the linked list*/
void *generate (void *arg){

	while (getWord(fp, buf, BUFFER_SIZE)){
		
		pthread_mutex_lock(&mutex);	
		addWord(buf); 	
		++wordCount;
		pthread_mutex_unlock(&mutex);		
	}//end while

	return 0;

}//end generate

/******************************************************************************************************/

/* Bubble sort the given linked lsit */
void bubbleSort(struct word *pStart){

	int swapped, i;
    struct word *ptr1;
    struct word *lptr = NULL;
 
    /* Checking for empty list */
    if (ptr1 == pStart)
        return;
 
    do{
        swapped = 0;
        ptr1 = pStart;
 
        while (ptr1->pNext != lptr){
			if (strcmp(ptr1->str, ptr1->pNext->str) > 0) {
            //if (ptr1->freq > ptr1->pNext->freq){ 
                swap(ptr1, ptr1->pNext);
                swapped = 1;
            }
            ptr1 = ptr1->pNext;
        }
        lptr = ptr1;
    } 
	
	while (swapped);
	
}//end bubbleSort

/******************************************************************************************************/ 

/* function to swap data of two nodes a and b*/
void swap(struct word *a, struct word *b){
    
	int temp1 = a->freq;
	a->freq = b->freq;
	b->freq = temp1;
	
	char *temp2 = (char*)malloc(MAX_WORD_SIZE + 1);
	strcpy(temp2, a->str);
	strcpy(a->str, b->str);
	strcpy(b->str, temp2);
	free(temp2);
	
	

}//end swap

/******************************************************************************************************/

/*main*/    
int main (int argc, char* argv[]) {

	Word *pCounter = NULL;
    
	//if the wrong amount of arguments are here
	if(argc != 3){

		fprintf(stderr,"usage: a.out <integer value>\n");
		return -1;

    	}//End if wrong number arguments
	
	int N = atoi(argv[2]);

	if (N < 1) {
		fprintf(stderr,"%d must be > 0\n",atoi(argv[2]));
		return -1;
	}	
	
	fp = (fopen(argv[1],"r"));//read the file that was input as argument
	
	if (fp != NULL){//if the file is there run the program

		pthread_t tid[N];//Thread ID

		/* create the mutex lock */
		pthread_mutex_init(&mutex, NULL);
		
		int i =0;

		/* create the thread */
		for (i=0; i<N; i++){
			pthread_create(&tid[i], NULL, generate, NULL);	//Create new thread
		}//end for

		i=0;	//reset
					
		/* wait for the thread to exit */
		for (i=0; i<N; i++){		

			pthread_join(tid[i], NULL);	//Threads back together
		}//end for
      
		loopList();//Count different words
		
		/*Prints the data*/
		printf("\nText File Data:\n\n");
		printf("   %d words\n", wordCount);	//Total words 
      	printf("   %d unique words\n", differentwords);	//Total different words 

		pCounter = pStart;
		
		bubbleSort(pCounter);	//sorts the linked list
		
		printList(pCounter);	//display's words
		
		printf("\n");	//new line

		pCounter = pStart;	//set to the beginning

		while(pCounter != NULL){

        		free(pCounter->str);        
        		pStart = pCounter;           
        		pCounter = pCounter->pNext;  
        		free(pStart);                  

		}//end while
      
		fclose(fp);// close the file
	
	}//end if
	
	//If file isn't found give an error
	else{

		printf("Input file %s cannot be opened.\n", argv[1]);
	
	}//end if

	return 0;

}//end main