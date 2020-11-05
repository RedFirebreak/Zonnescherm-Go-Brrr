#include "AVR_TTC_scheduler.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define F_CPU 16000000

// The array of tasks
sTask SCH_tasks_G[SCH_MAX_TASKS];


/*------------------------------------------------------------------*-

  SCH_Dispatch_Tasks()

  This is the 'dispatcher' function.  When a task (function)
  is due to run, SCH_Dispatch_Tasks() will run it.
  This function must be called (repeatedly) from the main loop.

-*------------------------------------------------------------------*/

void SCH_Dispatch_Tasks(void)
{
   unsigned char Index;

   // Dispatches (runs) the next task (if one is ready)
   for(Index = 0; Index < SCH_MAX_TASKS; Index++)
   {
      if((SCH_tasks_G[Index].RunMe > 0) && (SCH_tasks_G[Index].pTask != 0))
      {
         (*SCH_tasks_G[Index].pTask)();  // Run the task
         SCH_tasks_G[Index].RunMe -= 1;   // Reset / reduce RunMe flag

         // Periodic tasks will automatically run again
         // - if this is a 'one shot' task, remove it from the array
         if(SCH_tasks_G[Index].Period == 0)
         {
            SCH_Delete_Task(Index);
         }
      }
   }
}

/*------------------------------------------------------------------*-

  SCH_Add_Task()

  Causes a task (function) to be executed at regular intervals 
  or after a user-defined delay

  pFunction - The name of the function which is to be scheduled.
              NOTE: All scheduled functions must be 'void, void' -
              that is, they must take no parameters, and have 
              a void return type. 
                   
  DELAY     - The interval (TICKS) before the task is first executed

  PERIOD    - If 'PERIOD' is 0, the function is only called once,
              at the time determined by 'DELAY'.  If PERIOD is non-zero,
              then the function is called repeatedly at an interval
              determined by the value of PERIOD (see below for examples
              which should help clarify this).


  RETURN VALUE:  

  Returns the position in the task array at which the task has been 
  added.  If the return value is SCH_MAX_TASKS then the task could 
  not be added to the array (there was insufficient space).  If the
  return value is < SCH_MAX_TASKS, then the task was added 
  successfully.  

  Note: this return value may be required, if a task is
  to be subsequently deleted - see SCH_Delete_Task().

  EXAMPLES:

  Task_ID = SCH_Add_Task(Do_X,1000,0);
  Causes the function Do_X() to be executed once after 1000 sch ticks.            

  Task_ID = SCH_Add_Task(Do_X,0,1000);
  Causes the function Do_X() to be executed regularly, every 1000 sch ticks.            

  Task_ID = SCH_Add_Task(Do_X,300,1000);
  Causes the function Do_X() to be executed regularly, every 1000 ticks.
  Task will be first executed at T = 300 ticks, then 1300, 2300, etc.            
 
-*------------------------------------------------------------------*/

unsigned char SCH_Add_Task(void (*pFunction)(), const unsigned int DELAY, const unsigned int PERIOD)
{
   unsigned char Index = 0;

   // First find a gap in the array (if there is one)
   while((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS))
   {
      Index++;
   }

   // Have we reached the end of the list?   
   if(Index == SCH_MAX_TASKS)
   {
      // Task list is full, return an error code
      return SCH_MAX_TASKS;  
   }

   // If we're here, there is a space in the task array
   SCH_tasks_G[Index].pTask = pFunction;
   SCH_tasks_G[Index].Delay =DELAY;
   SCH_tasks_G[Index].Period = PERIOD;
   SCH_tasks_G[Index].RunMe = 0;

   // return position of task (to allow later deletion)
   return Index;
}

/*------------------------------------------------------------------*-

  SCH_Delete_Task()

  Removes a task from the scheduler.  Note that this does
  *not* delete the associated function from memory: 
  it simply means that it is no longer called by the scheduler. 
 
  TASK_INDEX - The task index.  Provided by SCH_Add_Task(). 

  RETURN VALUE:  RETURN_ERROR or RETURN_NORMAL

-*------------------------------------------------------------------*/

unsigned char SCH_Delete_Task(const unsigned char TASK_INDEX)
{
   // Return_code can be used for error reporting, NOT USED HERE THOUGH!
   unsigned char Return_code = 0;

   SCH_tasks_G[TASK_INDEX].pTask = 0;
   SCH_tasks_G[TASK_INDEX].Delay = 0;
   SCH_tasks_G[TASK_INDEX].Period = 0;
   SCH_tasks_G[TASK_INDEX].RunMe = 0;

   return Return_code;
}

/*------------------------------------------------------------------*-

  SCH_Init_T1()

  Scheduler initialisation function.  Prepares scheduler
  data structures and sets up timer interrupts at required rate.
  You must call this function before using the scheduler.  

-*------------------------------------------------------------------*/

void SCH_Init_T1(void)
{
   unsigned char i;

   for(i = 0; i < SCH_MAX_TASKS; i++)
   {
      SCH_Delete_Task(i);
   }

   // Set up Timer 1
   // Values for 1ms and 10ms ticks are provided for various crystals

   // Hier moet de timer periode worden aangepast ....!
   // uint16_t -  uint8_t
   OCR1A = (uint16_t)625;   		     // 10ms = (256/16.000.000) * 625
   TCCR1B = (1 << CS12) | (1 << WGM12);  // prescale op 64, top counter = value OCR1A (CTC mode)
   TIMSK1 = 1 << OCIE1A;   		     // Timer 1 Output Compare A Match Interrupt Enable
}

/*------------------------------------------------------------------*-

  SCH_Start()

  Starts the scheduler, by enabling interrupts.

  NOTE: Usually called after all regular tasks are added,
  to keep the tasks synchronized.

  NOTE: ONLY THE SCHEDULER INTERRUPT SHOULD BE ENABLED!!! 
 
-*------------------------------------------------------------------*/

void SCH_Start(void)
{
      sei();
}

/*------------------------------------------------------------------*-

  SCH_Update

  This is the scheduler ISR.  It is called at a rate 
  determined by the timer settings in SCH_Init_T1().

-*------------------------------------------------------------------*/

ISR(TIMER1_COMPA_vect)
{
   unsigned char Index;
   for(Index = 0; Index < SCH_MAX_TASKS; Index++)
   {
      // Check if there is a task at this location
      if(SCH_tasks_G[Index].pTask)
      {
         if(SCH_tasks_G[Index].Delay == 0)
         {
            // The task is due to run, Inc. the 'RunMe' flag
            SCH_tasks_G[Index].RunMe += 1;

            if(SCH_tasks_G[Index].Period)
            {
               // Schedule periodic tasks to run again
               SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
               SCH_tasks_G[Index].Delay -= 1;
            }
         }
         else
         {
            // Not yet ready to run: just decrement the delay
            SCH_tasks_G[Index].Delay -= 1;
         }
      }
   }
}

// ------------------------------------------------------------------ 


// Declare global variables
uint8_t i;
uint8_t CurrentExtend;
uint8_t MinExtend;
uint8_t MaxExtend;
	
uint8_t LightLevel;
uint8_t Temperature;
uint8_t input;

void Do_X(void)
{
	i = i + 1;
}

void extendSunscreen(void) {
    /* Deze functie laat het zonnescherm naar beneden.
     * Er moet een motortje runnen tot de gekozen lengte.
     * Het scherm mag niet verder uit staan dan de maximale lengte.
     * Omdat wij geen motor hebben moeten we dit indiceren met een LEDje.
     */

	if (CurrentExtend  < MaxExtend) {
		for(uint8_t i = CurrentExtend; i < MaxExtend; i++) {
			CurrentExtend++;
			PORTD = 0b00001000;
			_delay_ms(500); // "Expanding delay"
			PORTD = 0b00000000;
			_delay_ms(500); // "Expanding delay"

			// TODO if Ultrasone sensor too far, stop extend!
		}
	}
	PORTD = 0b00000100; // Max extended. Green!
}

void withdrawSunscreen(void) {
    /* Deze functie trekt het zonnescherm terug.
     * Er moet een motortje runnen tot de gekozen lengte.
     * Het scherm mag niet verder terug getrokken worden dan 0.
     * Omdat wij geen motor hebben moeten we dit indiceren met een LEDje.
     */
	if (CurrentExtend > MinExtend) {
		for(uint8_t i = CurrentExtend; MinExtend < i; i--) {
			CurrentExtend--;
			PORTD = 0b00001000;
			_delay_ms(500); // "Expanding delay"
			PORTD = 0b00000000;
			_delay_ms(500); // "Expanding delay"

			// TODO if ultrasone sensor too far back, stop extend!
		}
	}
	PORTD = 0b00010000; // Max retracted. Red!
}

void CheckTemp(void)
{
	int Temperature;

	changeOutput(1, 12);
}

void CheckLight(void)
{
	int LightLevel;

	changeOutput(2, 50);
}

void CheckInput(void)
{
/* Deze functie kijkt of er input klaar staat. En zet het om naar een extra return code. Deze code wordt meegenomen in de main-loop als "extra" actie. */
	uint8_t input = 0b00000000; /* TODO: READ INFO FROM PORT ON MOBO */
	switch(input)
	{
		case '0b00000000': /* input 1: WITHDRAW SCREEN */
			input = 1; /* Return code for main */
			break;
		case '0b00000010': /* input 2: EXTEND SCREEN*/
			input = 2; /* Return code for main */
			break;
		case '0b00000100': /* input 3: FORCE UPDATE? */
			input = 3; /* Return code for main */
			break;
		default:
			input = 0; /* No input */
			break;
	}
}


void changeOutput(int type, int amount) {
	/* 3 types of output */
	switch(type) {
		case 1:
		/* output change 1: Lightlevel */
		break;
		case 2:
		/* output change 2: Temperature */
		break;
		case 3:
		/* output change 3*/
		break;
	}
}

int main() {
	i = 0;
	CurrentExtend = 0;
	MinExtend = 0;
	MaxExtend = 20;

	LightLevel = 0;
	Temperature = 0;

	DDRD = 0b11111100; // Port D2 (Pin 4 in the ATmega) made output
	PORTD = 0b00000000; // Turn LED off

	 // Setup data scheduler
	SCH_Init_T1();
	
	// 1 = 100ms
	// 10 = 1s
	// 300 = 30s
	int CheckInput_Task_ID = SCH_Add_Task(CheckInput,0,1); // Every 0.1 sec
	int CheckTemp_Task_ID = SCH_Add_Task(CheckTemp,0,400); // Every 40 sec
	int CheckLight_Task_ID = SCH_Add_Task(CheckLight,0,300); // Every 30 sec

	// Test
	SCH_Add_Task(withdrawSunscreen,0,2000);
	SCH_Add_Task(extendSunscreen,1000,2000);

	SCH_Start(); // Start scheduler

	while(1) { // Infinite loop
		SCH_Dispatch_Tasks(); // Execute scheduled tasks
	} // While loop end
	

	return 0;
}