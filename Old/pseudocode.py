# maakSchermAan.py
import serial


def create_window():
    # Deze functie maakt het basisscherm aan waarin vervolgens alle
    # onderdelen worden geplaatst.
    # Deze zal gebruik maken van tkinter
    pass

def fill_window():
    # Deze functie neemt alle onderdelen die in het scherm geplaatst moeten worden
    # en plaatst ze erin.
    # De dingen die in het scherm moeten komen zijn:
    # - de mogelijkheid om tussen de x aantal eenheden te switchen
    # - een grafiek die de binnengekomen data weergeeft
    # - een scherm met de mogelijkheid instellingen te veranderen
    #       - de parameters (max en min) van lichtintensiteit veranderen
    #       - de parameters (max en min) uitrolafstand van het zonnescherm
    # - knop om de zonneschermen afhankelijk van de huidige staat in of uit te rollen
    # - knop om te wisselen tussen het grafiek- en instellingenscherm
    # - een indicator van waar het zonnescherm mee bezig is (misschien in de vorm van een stoplicht)
    # - een timer op een 60 seconden cyclus die aangeeft wanneer de volgende data binnenkomt
    pass

def save_data():
    pass
    # een functie die de binnengekomen data opslaat zodat deze later kan worden gebruikt
    # voor het genereren van een grafiek
    # return: data in een bepaalde vorm

def generate_graph(data):
    pass
    # een functie die de opgeslagen data uit saveData() visueel maakt door middel van een grafiek

def cycle_timer():
    pass
    # functie die een 60 seconden cyclus timer maakt

def look_for_control_units():
    pass
    # functie die altijd draait en op zoek is naar control units om weer te geven




