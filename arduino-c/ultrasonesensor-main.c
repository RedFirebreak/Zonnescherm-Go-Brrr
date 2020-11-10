#include "AVR_TTC_scheduler.h"
#include "distance.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <stdint.h>
#include <stdlib.h>
#define F_CPU 16000000 // CPU amount for correct delay
#include <util/delay.h>
#define UBBRVAL 51 // For Output

// Declare global variables
uint16_t CurrentExtend;
uint16_t SensorDistance;
uint16_t MinExtend;
uint16_t MaxExtend;

uint16_t LightLevelToggle;
uint16_t LightLevel;

uint16_t Temperature;
uint16_t input;

// The array of tasks
sTask SCH_tasks_G[SCH_MAX_TASKS];

volatile uint16_t gv_counter; // 16 bit counter value
volatile uint16_t gv_echo; // a flag

// Init ports
void ports_init(void) {
	// Sets all ports to the required input / output
	DDRD = 0b00000000; // Make port D input
	DDRB = 0b11111111; // Make port B output
	
	PORTB = 0b00000000; // Clear port B
	PORTD = 0b00000000; // Clear port D
}

// For USB input and output
void uart_init() {
	// set the baud rate
	UBRR0H = 0;
	UBRR0L = UBBRVAL;
	// disable U2X mode
	UCSR0A = 0;
	
	// enable transmitter and receiver
	// WERKT NIET || UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
	// WERKT NIET || UCSR0B = 0x18;
	UCSR0B = _BV(TXEN0);
	
	// set frame format : asynchronous, 8 data bits, 1 stop bit, no parity
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

void transmit(uint16_t type,uint16_t data) {
	// Type register
	// 1. Arm length
	// 2. Temperature
	// 3. Light level
	
	// Create the transmit array
	//uint16_t transmitArr[] = { type, data };
	
	//wait for an empty transmit buffer
	//UDRE is set when the transmit buffer is empty
	loop_until_bit_is_set(UCSR0A, UDRE0);
	//send data
	UDR0 = data; 
}

// Lichtsensor
void init_adc() {
	// ref=Vcc, left adjust the result (8-bit resolution),
	// select channel 0 (PC0 = input)
	ADMUX = (1<<REFS0)|(1<<ADLAR);
	// enable the ADC & prescale= 128
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t get_adc_value() {
	ADCSRA |= (1 << ADSC); // start conversion
	loop_until_bit_is_clear(ADCSRA, ADSC);
	// Save a value between 0 and 255 to sensorData.
	return ADCH;
}

// Sonor

int calc_cm(int counter)
{
	// counter 0 ... 65535, f = 16 MHz
	// prescaler = 1024 so multiply with 1024
	// devide by 16 because FCPU = 16MHz, buildin * 1000 * 1000 for uS
	// divide uS by 58 to convert to cm

	return ((int) (counter*1024/16)/58);
}
void init_timer(void)
// prescaling : max time = 2^16/16E6 = 4.1 ms, 4.1 >> 2.3, so no prescaling required
// normal mode, no prescale, stop timer
{
	TCCR1C = 0;
	TCCR1A = 0;
}

void init_ext_int(void)
{
	// any change triggers ext interrupt 1
	EICRA = (1 << ISC10);
	EIMSK = (1 << INT1);
}

uint16_t checkSonor() {
	// Init for the distance
	
	uint16_t cm = 0;
	gv_echo = BEGIN; // set flag for ISR
	// start trigger pulse lo -> hi (D0)
	PORTD |= _BV(0);
	_delay_us(12); // micro sec
	// stop trigger pulse hi -> lo (D0)
	PORTD = 0x00;
	// wait 30 milli sec, gv_counter == timer1 (read in ISR)
	_delay_ms(30);
	cm = calc_cm(gv_counter);
	_delay_ms(500); // wait 0.5 sec
	transmit(1, cm);
	return cm;
}

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
   // uint16_t -  uint16_t
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
   
   if (gv_echo == BEGIN) {
	   // set timer1 value to zero and start counting
	   // C Compiler automatically handles 16-bit I/O read/write operations in the correct order
	   TCNT1 = 0;
	   TCCR1A |= (1<<CS10) | (0<<CS11) | (1<<CS12); // set prescaler to 1024
	   // clear flag
	   gv_echo = END;
	   } else {
	   // stop counting and read value timer1
	   TCCR1A = 0;
	   gv_counter = TCNT1;
   }
   
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

void extendSunscreen(void) {
    /* Deze functie laat het zonnescherm naar beneden.
     * Er moet een motortje runnen tot de gekozen lengte.
     * Het scherm mag niet verder uit staan dan de maximale lengte.
     * Omdat wij geen motor hebben moeten we dit indiceren met een LEDje.
     */

	if (CurrentExtend  < MaxExtend) {
		for(uint16_t i = CurrentExtend; i < MaxExtend; i++) {
			CurrentExtend++;
			PORTB = 0b00000010;
			_delay_ms(400); // "Expanding delay"
			PORTB = 0b00000000;
			_delay_ms(400); // "Expanding delay"

			// TODO if Ultrasone sensor too far, stop extend!
			// SensorDistance = getSensorDistance();
			if (SensorDistance > 150) {
				i = MaxExtend;
			}
			
		}
	}
	transmit(1,CurrentExtend);
	PORTB = 0b00000001; // Max extended. Green!
}

void withdrawSunscreen(void) {
    /* Deze functie trekt het zonnescherm terug.
     * Er moet een motortje runnen tot de gekozen lengte.
     * Het scherm mag niet verder terug getrokken worden dan 0.
     * Omdat wij geen motor hebben moeten we dit indiceren met een LEDje.
     */
	if (CurrentExtend > MinExtend) {
		for(uint16_t i = CurrentExtend; MinExtend < i; i--) {
			CurrentExtend--;
			PORTB = 0b00000010;
			_delay_ms(400); // "Expanding delay"
			PORTB = 0b00000000;
			_delay_ms(400); // "Expanding delay"

			// TODO if ultrasone sensor too far back, stop extend!
			// SensorDistance = getSensorDistance();
			if (SensorDistance < 10) {
				i = MinExtend;
			}
		}
	}
	transmit(1,CurrentExtend);
	PORTB = 0b000000100; // Max retracted. Red!
}

void CheckLight(void) {
	LightLevel = get_adc_value();
	
	if (LightLevel > LightLevelToggle) { // If light level is OVER 125
		extendSunscreen();
	} else {
		withdrawSunscreen();
	}
	
	transmit(1,LightLevel);
}

void CheckInput(void)
{
/* Deze functie kijkt of er input klaar staat. En zet het om naar een extra return code. Deze code wordt meegenomen in de main-loop als "extra" actie. */
	uint16_t input = 0b00000000; /* TODO: READ INFO FROM PORT ON MOBO */
}

int main() {
	MinExtend = 005;
	MaxExtend = 160;
	CurrentExtend = 005;
	SensorDistance = 0;

	LightLevelToggle = 125; // The level at which the screen extracts / withdraws
	LightLevel = 0;
	
	Temperature = 0;
	// Verander de benodigde poorten
	ports_init();
	
	// Init input / output via usb
	uart_init();
	
	// Init sonor
	init_timer();
	init_ext_int();

	// Init van de lichtsensor
	init_adc();

	// Setup data scheduler
	SCH_Init_T1();
	
	// 1 = 10ms || 100 = 1s || 4000 = 30s
	// int CheckTemp_Task_ID = SCH_Add_Task(CheckTemp,0,400); // Every 40 sec
	//int CheckLight_Task_ID = SCH_Add_Task(CheckLight,0,100); // Every 30 sec
	
	// Debug suncreen
		// SCH_Add_Task(extendSunscreen,0,6000); // Every 40 sec
		// SCH_Add_Task(withdrawSunscreen,3000,6000); // Every 30 sec
	 
	// Test
	SCH_Start(); // Start scheduler

	_delay_ms(50);
	while(1) { // Infinite loop
		//transmit(1, checkSonor());
		checkSonor();
		
		CheckInput();
		SCH_Dispatch_Tasks(); // Execute scheduled tasks
		_delay_ms(1000);
	} // While loop end

	return 0;
}