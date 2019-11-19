/*
  Name: Jeremiah Krick
  Instructor: Dr. Carroll
  Class: CS570
  Due: 11/01/19


   This is the ONLY file you are allowed to change. (In fact, the other
   files should be symbolic links to
     ~cs570/Three/p3main.c
     ~cs570/Three/p3robot.c
     ~cs570/Three/p3.h
     ~cs570/Three/makefile
     ~cs570/Three/CHK.h    )
*/
#include "p3.h"
#include <math.h> // Used to calulate the Peak of the pyramid (maxWidth).
/* You may put declarations/definitions here.
   In particular, you will probably want access to information
   about the job (for details see the assignment and the documentation
   in p3robot.c):
*/
extern int nrRobots;
extern int quota;
extern int seed;
sem_t *pmutx; /* semaphore guarding access to shared data */
int count; /* local copy of counter */
int fd;
char semaphoreMutx[SEMNAMESIZE];
int row; // The current row of the pyramid
int maxWidth;
int botCount; // The count for the bottom half of the pyramid

/* General documentation for the following functions is in p3.h
   Here you supply the code, and internal documentation:
*/
void initStudentStuff(void) {
    /* Try to make a "name" for the semaphore that will not clash. */
    sprintf(semaphoreMutx,"/%s%ldmutx",COURSEID,(long)getuid());
    /* Create and initialize the semaphore. */
    if( SEM_FAILED != (pmutx = sem_open(semaphoreMutx,O_RDWR|O_CREAT|O_EXCL,S_IRUSR|S_IWUSR,0))) {
        CHK(fd = open("/home/cs/carroll/cssc0056/Three/countfile",O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR));
        count = 0;
        botCount = 0;
        row = 1;
        maxWidth = ((int)ceil(sqrt(nrRobots * quota))); // maxWidth is the Peak of the pyramid.
        CHK(lseek(fd,0,SEEK_SET));
        assert(sizeof(count) == write(fd,&count,sizeof(count))); // Initialize count
  	    assert(sizeof(row) == write(fd,&row,sizeof(row))); // Initialize row
  	    assert(sizeof(maxWidth) == write(fd,&maxWidth,sizeof(maxWidth))); // Initialize maxWidth
  	    assert(sizeof(botCount) == write(fd,&botCount,sizeof(botCount))); // Initialize botCount
  	    CHK(sem_post(pmutx));
    }
    /* semaphore has already been created.*/
    else CHK(SEM_FAILED != (pmutx = sem_open(semaphoreMutx, O_RDWR)));
}
/* In this braindamaged version of placeWidget, the widget builders don't
   coordinate at all, and merely print a random pattern. You should replace
   this code with something that fully follows the p3 specification. 
*/
void placeWidget(int n) {
    CHK(sem_wait(pmutx)); /* request access to critical section */
    /* begin critical section -- read count, increment count, write count */
    CHK(fd = open("/home/cs/carroll/cssc0056/Three/countfile", O_RDWR));
    CHK(lseek(fd,0,SEEK_SET));
    assert(sizeof(count) == read(fd,&count,sizeof(count)));
  	assert(sizeof(row) == read(fd,&row,sizeof(row)));
  	assert(sizeof(maxWidth) == read(fd,&maxWidth,sizeof(maxWidth)));
  	assert(sizeof(botCount) == read(fd,&botCount,sizeof(botCount)));
    count++;
    int temp = count; // Save the total widget count in temp.
    if(count == ((nrRobots * quota))) { // If last integer, print 'F', and close everything.
  	    printeger(n);
	    printf("F\n");
	    fflush(stdout);
        CHK(close(fd));
        CHK(unlink("/home/cs/carroll/cssc0056/Three/countfile"));
        CHK(sem_close(pmutx));
        CHK(sem_unlink(semaphoreMutx));
        return;
    }
    else if (row <= maxWidth) { // Top half of pyramid.
        int i;
        for(i=1; i<row; i++) count = count - i; // Count is now the widget count in the current row.
  	    if (count == row) { // If last widget in the row print 'N'.
  	        if(count == maxWidth) maxWidth--; // If pyramid's Peak has been reached then decrement & go to bottom half of pyramid.
    	    printeger(n);
  		    printf("N\n");
  		    fflush(stdout);
  		    row++;
  	    }
	    else { // The widget is not the last in the row, so just print it.
		    printeger(n);
		    fflush(stdout);
	    }
	}
	else { // Bottom half of pyramid.
	    botCount++; // Total count of widget in the bottom half only.
        if (botCount > maxWidth) botCount = botCount - (maxWidth + 1);	   // botCount is now the count of the row. 
	    if (!(botCount % maxWidth)) { // If last widget in the row print 'N'. 
	        printeger(n);
  		    printf("N\n");
  		    fflush(stdout);
  		    maxWidth--;
  		    row++; // Current row of the pyramid continues to increment so that we never test the previous 'else if' 
  		           // condition as we are decrementing maxWidth.
	    }
	    else { // The widget is not the last in the row, so just print it.
	        printeger(n);
	        fflush(stdout);
	    }
	}
	count = temp; // Count returns to the total widget count after being changed in the top half of the pyramid.
	CHK(lseek(fd,0,SEEK_SET));
  	assert(sizeof(count) == write(fd,&count,sizeof(count)));
  	assert(sizeof(row) == write(fd,&row,sizeof(row)));
  	assert(sizeof(maxWidth) == write(fd,&maxWidth,sizeof(maxWidth)));
  	assert(sizeof(botCount) == write(fd,&botCount,sizeof(botCount)));
  	/* end critical section */
  	CHK(sem_post(pmutx)); /* release critical section */
}
