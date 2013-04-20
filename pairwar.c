#include<stdlib.h>
#include<pthread.h>
#include<stdio.h>


struct Deque{
	int val;
	struct Deque* next;
};

pthread_mutex_t mutex;
pthread_cond_t player1_sem, player2_sem, player3_sem, dealer_sem;
int *deck;
struct Deque* head = NULL;
struct Deque* tail = NULL;
int winner = 0;
int starting_player = 1;
int current_round = 0;
int startup = 0;
int player1_hand, player2_hand, player3_hand;
int draw;
int decksize = 52;


void push(int val){

	struct Deque* newNode = (struct Deque*)malloc(sizeof(struct Deque));
	newNode->val = val;
	newNode->next = NULL;

	if( head == NULL ){
		head = newNode;
		tail = newNode;
	}else{

		tail->next = newNode;
		tail = newNode;	
	}

	
}

void push_back(int val){
	
	struct Deque* newNode = (struct Deque*)malloc(sizeof(struct Deque));
	newNode->val = val;
	newNode->next = NULL;
	
	if(tail != NULL){
		tail->next = newNode;
		tail = tail->next;
	}else{
		tail = newNode;
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


void shuffleArray(void* seed){
	long s = (long)seed;
	int temp, r, i;
	srand(s);

	for(i = 0; i<52; i++){
		r = rand()%52;
		temp = deck[i];
		deck[i] = deck[r];
		deck[r] = temp;
	}
}


void generateArray(){
	int i;
	int j;
	for(i = 0; i < 13; i++){
		for(j = 0; j < 4; j++){
			deck[i*4+j] = i+1;
		}
	}
}


void arraytoDeque(){
	
	int i;

	for( i = 0; i < decksize; i++)
		push(deck[i]);
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

void dequetoArray(){
	int i;
	for( i = 0; i < 52; i++ )
		deck[i] = pop();
}


void shuffleDeque(void* val){
	
	struct Deque* nodePtr;
	struct Deque* shuffleNode;
	struct Deque* prevNode;
	int i, j, s, r;
	long seed = (long)val;

	shuffleNode = malloc(sizeof(struct Deque));
	nodePtr = head;
	s = dequeSize() - 1;
	srand(seed);
	
	for(i = 0; i < 300; i++){

		shuffleNode->val = head->val;
		shuffleNode->next = NULL;
		nodePtr = head->next;
		free(head);
		head = nodePtr;
		r = rand();

		for(j = 0; j < 7; j++){
			prevNode = nodePtr;
			nodePtr = nodePtr->next;
		}

		prevNode->next = shuffleNode;
		shuffleNode->next = nodePtr;	
	}
	
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
		printf("Dequesize(): %d\n", dequeSize());
		printf("Dealer Shuffling\n"); fflush(stdout);
		shuffleDeque(seed);
		displayDeque();
//		dequetoArray();
//		shuffleArray(seed);
//		destroyDeque();
//		arraytoDeque();

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
			printf("RETURNING CARDS\n");
			push_back(player1_hand);
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
			printf("RETURNING CARDS\n");
			push_back(player2_hand);
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
			printf("RETURNING CARDS\n");
			push_back(player3_hand);
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
	
		printf("Generating Deque\n");
		fflush(stdout);
		for (i = 0; i < 15; i++)
			push(i);
		printf("Displaying Deque\n");
		fflush(stdout);

		displayDeque();
		return 0;
	}
	
	deck = (int*)malloc(52 * sizeof(int));
	//generate starting array(deck)
	generateArray();
	arraytoDeque();

	
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



