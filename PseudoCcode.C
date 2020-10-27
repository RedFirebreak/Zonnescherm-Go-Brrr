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