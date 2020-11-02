void extendSunscreen() {
    /* Deze functie laat het zonnescherm naar beneden.
     * Er moet een motortje runnen tot de gekozen lengte.
     * Het scherm mag niet verder uit staan dan de maximale lengte.
     * Omdat wij geen motor hebben moeten we dit indiceren met een LEDje.
     */
}

void withdrawSunscreen() {
    /* Deze functie trekt het zonnescherm terug.
     * Er moet een motortje runnen tot de gekozen lengte.
     * Het scherm mag niet verder terug getrokken worden dan 0.
     * Omdat wij geen motor hebben moeten we dit indiceren met een LEDje.
     */
}

void withdrawWithRain() {
    /* Deze functie moet het zonnescherm maximaal laten intrekken als het regent.
     * Hiervoor moet hij withdrawSunscreen() aanroepen.
     */
}

void senseLight() {
    /* Deze functie moet zonlicht detecteren. Als er zonlicht is, klapt het zonnescherm uit.
     * Het zonnescherm uitklappen gaat via extendSunscreen().
     */ 
}

void withdrawAfterTime() {
    /* Deze functie trekt het zonnescherm terug na een bepaalde tijd. */
}

void extendAfterTime() {
    /* Deze functie klapt het zonnescherm uit na een bepaalde tijd. */
}

void measureTemp() {
    /* Deze functie meet de temperatuur iedere 40 seconden. */
}

void changeOutput(int type, int amount) {
	/* 3 types of output */
	switch(type) {
		case 1:
			/* output change 1 */
			break;
		case 2:
			/* output change 2 */
			break;
		case 3:
			/* output change 3*/
			break;
	}
}

void getInput() {
	/* Deze functie kijkt of er input klaar staat. En zet het om naar een extra return code. Deze code wordt meegenomen in de main-loop als "extra" actie. */
	uint8_t input = 0b00000000; /* TODO: READ INFO FROM PORT ON MOBO */
	switch(input)
	{
		case '0b00000000': /* input 1: WITHDRAW SCREEN */
			return 1; /* Return code for main */
			break;
		case '0b00000010': /* input 2: EXTEND SCREEN*/
			return 2; /* Return code for main */
			break;
		case '0b00000100': /* input 3: FORCE UPDATE? */
			return 3; /* Return code for main */
			break;
	}
	/* no input? Return 0 and let program continue */
	return 0;
	
}

void main(void) {
	/* zet een aantal variabelen klaar om mee te werken */
	int CurrentExtend = 0;
	int MinExtend = 0;
	int MaxExtend = 255;
	
	int LightLevel = 0;
	int Temperature = 0;
	
	/* Start de infinite loop. Hier wordt elke loop gekeken voor input en de output aangepast */
	while(1) { /* Infinite loop */
			
		/* 1. Check for input */
		int INPUT = getInput();
		
		if (INPUT == 1) {
			withdrawSunscreen();
			/* TODO: We need some sort of override, otherwise the light level will extend / withdraw again because of code logic */
		}
		else if (INPUT == 2) {
			extendSunscreen();
			/* TODO: We need some sort of override, otherwise the light level will extend / withdraw again because of code logic */
		}
		else if INPUT == 3() {
			/* Force an extra update */
			changeOutput(1, Temperature);
			changeOutput(2, LightLevel);
			changeOutput(3, CurrentExtend);
		}
		
		/* 2. Check Temperature 
		IF TIME > 40 SEC
		*/
		if (1) {
			Temperature = measureTemp();
			changeOutput(1, Temperature);
		}


		/* 3. Check LightLevel*/
		 LightLevel = senseLight()
		 changeOutput(2, LightLevel);
		 
		 if (LightLevel >= 200) {
			 extendSunscreen()
			 changeOutput(3, CurrentExtend);
		 }
		 else {
			 withdrawSunscreen()
			 changeOutput(3, CurrentExtend);
		 }
		
	}
}