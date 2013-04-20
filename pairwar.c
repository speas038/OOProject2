#include<stdlib.h>
#include<pthread.h>
#include<stdio.h>


struct Deque{
	int val;
	struct Deque* next;
};

pthread_mutex_t mutex;
pthread_cond_t player1_sem, player2_sem, player3_sem, dealer_sem;
struct Deque* head = NULL;
struct Deque* tail = NULL;
int winner = 0;
int starting_player = 1;
int current_round = 0;
int startup = 0;
int player1_hand, player2_hand, player3_hand;
int draw;
int decksize = 52;



void push_back(int val){
	
	struct Deque* newNode = (struct Deque*)malloc(sizeof(struct Deque));
	newNode->val = val;
	newNode->next = NULL;
	
	if(tail != NULL){
		tail->next = newNode;
		tail = tail->next;
	}else{
		tail = newNode;
		head = newNode;
	}
	
}


int pop(){
	
	int val;
	struct Deque*  nodePtr = head;
	if(head != NULL)
		val = head->val;
	head = head->next;
	free(nodePtr);
	return val;
	
}

int dequeSize(){
	
	int size = 0;
	struct Deque* nodePtr;
	nodePtr = head;
	
	while(nodePtr != NULL){
		nodePtr=nodePtr->next;
		size++;
	}
	return size;

}

void displayDeque(){
	
	struct Deque* nodePtr;
	nodePtr = head;

	printf("Displaying Deque\n");	
	while(nodePtr != NULL){
		printf("%d ", nodePtr->val);
		nodePtr = nodePtr->next;
	}
	printf("\n");
}



void generateDeque(){
	int i, j;
	for(i=0; i<13; i++)
		for(j=0; j<4; j++)
			push_back(i+1);
}



void destroyDeque(){

	
	struct Deque* nodePtr = head;
	struct Deque* prevNode = head;

	while(nodePtr != NULL){
		prevNode = nodePtr;
		nodePtr = nodePtr->next;
		free(prevNode);
	}

	head = NULL;
	tail = NULL;
}


void shuffleDeque(void* val){
	
	struct Deque* nodePtr;
	struct Deque* shuffleNode;
	struct Deque* prevNode;
	int i, j, s, r;
	long seed = (long)val;

	//start nodePtr at head
	nodePtr = head;
	//dequesize - 2 along with the inner for loop ensure that
	//we will not go out of bounds with our randomizing;
	//praise Jesus, this took me forever to figure out
	s = dequeSize() - 2;
	srand(seed);
	
	//iterate enought times to touch at least each element of the deque and place
	//it in a random spot
	for(i = 0; i < 52; i++){

		shuffleNode = malloc(sizeof(struct Deque));
		shuffleNode->val = head->val;
		shuffleNode->next = NULL;
		nodePtr = head->next;
		free(head);
		head = nodePtr;
		r = (rand() % s);

		for(j = 0; j <= r % s; j++){
			prevNode = nodePtr;
			nodePtr = nodePtr->next;
		}
		
		if(nodePtr == NULL){
			prevNode->next = shuffleNode;
		}else{
			shuffleNode->next = nodePtr;
			prevNode->next = shuffleNode;
		}
	}
	
	//restore tail pointer
	nodePtr = head;
	while(nodePtr->next != NULL)
		nodePtr = nodePtr->next;
	tail = nodePtr;
}

void* dealer(void* seed){

	while(current_round < 3){
		pthread_mutex_lock(&mutex);
	
		if( current_round != 0 ){
			printf("dealer waiting for signal\n");
			pthread_cond_wait(&dealer_sem, &mutex);
		}

		if( current_round == 0 )
			current_round++;

		printf("\n\n  ROUND %d\n", current_round);

		//shuffle deque
		displayDeque();
		printf("Dealer Shuffling\n"); fflush(stdout);
		shuffleDeque(seed);
		displayDeque();

		//Dealer deals
		printf("Dealer dealing:\n");
		player1_hand = pop();
		player2_hand = pop();
		player3_hand = pop();

		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&player1_sem);
	}

	printf("DEALER EXITING\n");
	pthread_exit(NULL);
}


void* player1(){

	while(current_round < 3){

		pthread_mutex_lock(&mutex);
		startup++;
		pthread_cond_wait(&player1_sem, &mutex);
		
		draw = pop();
		printf("Player1 hand: %d\n", player1_hand);
		printf("Player1 draw: %d\n", draw);

		if(draw == player1_hand){
			printf("WINNER\n");
			printf("Ending Round %d\n", current_round);
			push_back(player1_hand);
			push_back(draw);
			current_round++;
		}else{
			printf("RETURNING CARD\n");
			if(rand() % 2 == 1)
				push_back(player1_hand);
			else
				push_back(draw);
		}
	
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&player2_sem);
	}
	printf("PLAYER1 EXITING\n");
	pthread_exit(NULL);
}

void* player2(){

	while(current_round < 3){

		pthread_mutex_lock(&mutex);
		startup++;
		pthread_cond_wait(&player2_sem, &mutex);

		draw = pop();
		printf("Player2 hand: %d\n", player2_hand);
		printf("Player2 draw: %d\n", draw);

		if(draw == player2_hand){
			printf("WINNER\n");
			printf("Ending Round %d\n", current_round);
			push_back(player2_hand);
			push_back(draw);
			current_round++;
		}else{
			printf("RETURNING CARD\n");
			if( rand() % 2 == 1 )
				push_back(player2_hand);
			else
				push_back(draw);

		}
	
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&player3_sem);
	}

	printf("PLAYER2 EXITING\n");
	pthread_exit(NULL);
}

void* player3(){

	while(current_round <= 3){

		pthread_mutex_lock(&mutex);
		startup++;
		pthread_cond_wait(&player3_sem, &mutex);

		draw = pop();
		printf("Playerr3 hand: %d\n", player1_hand);
		printf("Player3 draw: %d\n", draw);

		if(draw == player3_hand){
			printf("WINNER\n");
			printf("Ending Round %d\n", current_round);
			push_back(player3_hand);
			push_back(draw);
			current_round++;
		}else{
			printf("RETURNING CARD\n");
			if( rand() % 2 == 1 )
				push_back(player3_hand);
			else
				push_back(draw);
		}
	
		current_round++;

		pthread_mutex_unlock(&mutex);
		sleep(1);
		pthread_cond_signal(&dealer_sem);
	}

	printf("PLAYER3 EXITING\n");
	pthread_exit(NULL);
}

int main(int argc, char* argv[]){
	
	int i;
	long seed = 0;
	long thread;

	if(argc == 2){
		seed = atoi( argv[1] );
		printf("using seed: %d\n", seed);
	}else{
		seed = 56789;
		printf("using default seed: %d\n", seed);
	}
	
	if( pthread_mutex_init(&mutex, NULL) != 0){
		printf("Mutex Init Failed\n");
		return 1;
	}

	if(seed == 1){

		generateDeque();
		displayDeque();
		printf("deque size: %d\n", dequeSize());
		printf("SHUFFLING DEQUE\n");
		fflush(stdout);
		shuffleDeque((void*)seed);
		displayDeque();
		printf("deque size: %d\n", dequeSize());
		return 0;
	
	}
	
	//generate deck
	generateDeque();

	
	//I need a thread handle for the dealer and 3 players
	pthread_t* thread_handles = malloc( 4 * sizeof(pthread_t) );
	
	pthread_cond_init(&dealer_sem, NULL);
	pthread_cond_init(&player1_sem, NULL);
	pthread_cond_init(&player2_sem, NULL);
	pthread_cond_init(&player3_sem, NULL);

	//create dealer and players
//	pthread_cond_wait(&player1_start);
	pthread_create( &thread_handles[1], NULL, player1, NULL);
	pthread_create( &thread_handles[2], NULL, player2, NULL);
	pthread_create( &thread_handles[3], NULL, player3, NULL);
	sleep(1);
	while(startup < 3) sleep(.001);
	pthread_create( &thread_handles[0], NULL, dealer, (void*)seed);


	//join players
	for(thread = 1; thread < 4; thread++)
		pthread_join( thread_handles[thread], NULL); 

	//join dealer
	pthread_join( thread_handles[0], NULL );

	pthread_cond_destroy(&dealer_sem);
	pthread_cond_destroy(&player1_sem);
	pthread_cond_destroy(&player2_sem);
	pthread_cond_destroy(&player3_sem);
	pthread_mutex_destroy(&mutex);	
	destroyDeque();		
	
	return 0;
}



