import random
import time
from tkinter import *
from functools import partial
from matplotlib import animation
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import serial.tools.list_ports
import serial

# Het uitlezen van USB poorten geeft mogelijk problemen op afhankelijk
# van of Windows op Nederlands of Engels staat.
# Zet de juiste variabele aan afhankelijk van de taal.

#Engels
portstring = "USB Serial Device (COM" 
#Nederlands
#portstring = "Serieel USB-apparaat (COM"

class Eenheid(serial.Serial):
    """
    Dit is een klasse op basis van de serial.Serial klasse met als extra toevoegingen
    variabelen waarin waarden specifiek aan de eenheid objecten (de arduino's) opgeslagen
    kunnen worden.

    Bij het aanmaken van een Eenheid wordt de weer te geven data in de grafiek geinitieerd op
    20 * 0 welke daarna met een .pop(0) en .append(data) langzaam vervangen wordt door de
    binnenkomende data.

    De i teller wordt gebruikt voor een dynamische plaatsing van widgets binnen de Frames.

    De kleur wordt willekeurig gekozen uit een lijst met kleuren.

    De line wordt op de geinitieerde assen op de grafiek gebouwd met de gekozen kleur.
    """

    def __init__(self, port, baudrate):
        super().__init__(port, baudrate)
        self.data = 20 * [0]
        self.arm_data = 0
        self.i = 0
        self.minafstand = 5
        self.maxafstand = 160
        self.kleur = random_kleur(kleuren)
        self.line, = ax.plot(self.data, color=self.kleur, label=self.name)

    def update_graph(self, i):
        """
        De functie die de nieuwe waarden uit self.data doorgeeft aan de line en daarmee
        ze in de grafiek zet.

        :param i: volgende de matplotlib.animation documentatie is de mee te geven i
        die je vervolgens niet gebruikt nodig.

        :return: None
        """
        # self.subplot.set_ylim(min(self.data) - 5, max(self.data) + 5)
        self.line.set_data([x for x in range(len(self.data))], self.data)


class NamedFrame(Frame):
    """
    Een simpele klasse op basis van een Frame die het mogelijk maakt binnen de
    winfo_children() lijst te zoeken op een frame met de juiste naam om zo binnen
    het object aanpassingen te doen.
    """
    def __init__(self, name, master, background):
        """"
        Simpele constructor voor de NamedFrame klasse die als extra parameters de
        naam voor het NamedFrame en de gewenste achtergrondkleur meekrijgt.
        """
        super().__init__(master=master, bg=background)
        self.name = name


class Window(Frame):
    """
    Een adaptatie van de Frame klasse die uitgewerkt is met alle gewenste functionaliteiten
    voor de uiteindelijke GUI. Het idee van het maken van een aparte klasse hiervoor komt van
    pythonprogramming.net, de uiteindelijke uitwerking is op basis van eigen kunnen en documentatie
    van de geimporteerde libraries.
    """

    def __init__(self, master=None):
        """"
        De constructor voor de main Window. Deze maakt het window aan en geeft vervolgens door
        middel van de init_window() functie invulling aan het scherm.
        """
        Frame.__init__(self, master)
        self.master = master
        self.init_window()


    def init_window(self):
        """
        De init_window() functie geeft invulling aan het main scherm. Hier wordt het hart
        van de applicatie opgebouwd.
        :return: None
        """

        # Zet de titel van het main Window
        self.master.title("Snek Central: Origins")

        # Maakt de menubalk boven in het Window aan
        menu = Menu(self.master)
        self.master.config(menu=menu, bg='gray')

        # Geeft de menubalk invulling
        file = Menu(menu, tearoff=0)
        file.add_command(label='About Us', command=self.create_menu)
        file.add_command(label='Exit', command=root.destroy)
        menu.add_cascade(label='File', menu=file)

        # Maakt het NamedFrame aan waar de grafiek vervolgens in wordt aangemaakt
        graph_frame = NamedFrame("graph", master=self.master, background='gray')
        graph_frame.grid(row=1, column=1, padx=50, pady=50)

        # Maakt de grafiek aan
        canvas = FigureCanvasTkAgg(figure, master=graph_frame)
        canvas.draw()
        canvas.get_tk_widget().grid(row=1, column=1, columnspan=3, rowspan=8)

        # Zet de teller vervolgens op 1 voor dynamische plaatsing van de Eenheid Frames
        # Dit is 1 omdat de grafiek op plaats 0 staat
        i = 1

        # Gaat de eenheidlijst door om voor elke eenheid het Frame en de invulling aan te maken
        # op basis van de aangemaakte Eenheid objecten

        for eenheid in eenheidlijst:
            self.initialize_units(eenheid, i)
            # Teller vervolgens met 1 ophogen om de plaats in de centrale met één op te schuiven
            i += 1

        # Een spacer aan de meest rechterkant van het scherm zodat er altijd een margin is

        spacer_einde = Frame(self.master, height=50, width=50, bg='gray')
        spacer_einde.grid(row=0, column=100)

    def initialize_units(self, eenheid, i):
        """
        De initialize_units() functie maakt voor een Eenheid een Frame en geeft daar invulling aan.
        """

        eenheid.i = i
        eenheid_f = NamedFrame(eenheid.name, self.master, background='gray')
        eenheid_f.configure(highlightbackground='black', highlightthickness=2)
        eenheid_f.grid(row=1, column=i + 1, padx=10)
        spacer_boven = Frame(eenheid_f, height=25, width=50, background='gray')
        spacer_boven.grid(row=0, column=i * 5, columnspan=3)
        spacer_tussen = Frame(eenheid_f, height=50, width=50, background='gray')
        spacer_tussen.grid(row=1, column=i * 5)

        l1 = Label(eenheid_f, text=eenheid.port, padx=30, pady=3.5, relief="solid", borderwidth='0.5', fg="black",
                   bg="#849db5")
        l1.grid(row=1, column=i * 5 + 1)

        min_rol = Label(eenheid_f, text='Minimale rolafstand:', bg='gray')
        min_rol.grid(row=27, column=i * 5 + 1, padx=10)

        l2 = Entry(eenheid_f)
        l2.insert(END, '5-160')
        l2.grid(row=28, column=i * 5 + 1, padx=10)

        b1 = Button(eenheid_f, text="Save", command=partial(self.set_min_uitrolafstand, l2, eenheid), relief="solid",
                    borderwidth='0.5', fg="black", bg="#849db5")
        b1.grid(row=28, column=i * 5 + 2)

        max_rol = Label(eenheid_f, text='Maximale rolafstand:', bg='gray')
        max_rol.grid(row=29, column=i * 5 + 1, padx=10)

        l3 = Entry(eenheid_f)
        l3.insert(END, '5-160')
        l3.grid(row=30, column=i * 5 + 1)

        l4 = Label(eenheid_f, text='Current arm extend: ' + str(eenheid.arm_data), bg='gray')
        l4.grid(row=25, column=i*5+1)

        b2 = Button(eenheid_f, text="Save", command=partial(self.set_max_uitrolafstand, l3, eenheid), relief="solid",
                    borderwidth='0.5', fg="black", bg="#849db5")
        b2.grid(row=30, column=i * 5 + 2)

        b3 = Button(eenheid_f, text="Rol in / Rol uit", padx=10, relief="solid", borderwidth='0.5', fg="black",
                    bg="#849db5")
        b3.grid(row=40, column=i * 5 + 1, pady=10)

        b4 = Button(eenheid_f, text="Destroy", padx=28, command=partial(self.clear_frame, eenheid), relief="solid",
                    borderwidth='0.5', fg="black", bg="#849db5")
        b4.grid(row=50, column=i * 5 + 1, pady=10)

        spacer_einde = Frame(eenheid_f, height=50, width=25, bg='gray')
        spacer_einde.grid(row=25, column=100)

    def set_max_uitrolafstand(self, l3, eenheid):
        """
        Functie voor het aanpassen van de maximale uitrolafstand. Deze waarde wordt opgeslagen in het aangemaakte
        Eenheid object
        :param l3: Naam van het object waar de waarde wordt ingevoerd
        :param eenheid: Het Eenheid object waar de waarde moet worden opgeslagen
        :return: None
        """
        afstand = l3.get()
        if afstand.isnumeric():
            afstand = int(afstand)
            if 160 >= afstand > eenheid.minafstand:
                eenheid.maxafstand = afstand
                print("Maximale uitrol afstand ingesteld op: ", eenheid.maxafstand)
            else:
                print("Error: Maximale uitrolafstand moet groter zijn dan de minimale uitrolafstand en kleiner of gelijk aan 160")
        else:
            print("Error: Vul een numerieke waarde in.")

    def set_min_uitrolafstand(self, l2, eenheid):
        """
        Functie voor het aanpassen van de minimale uitrolafstand. Deze waarde wordt opgeslagen in het aangemaakte
        Eenheid object
        :param l2: Naam van het object waar de waarde wordt ingevoerd
        :param eenheid: Het Eenheid object waar de waarde moet worden opgeslagen
        :return: None
        """
        afstand = l2.get()
        if afstand.isnumeric():
            afstand = int(afstand)
            if eenheid.maxafstand > afstand >= 5:
                eenheid.minafstand = afstand
                print("Minimale uitrol afstand ingesteld op: ", eenheid.minafstand)
            else:
                print("Error: Minimale uitrolafstand moet kleiner zijn dan de maximale uitrolafstand en groter of gelijk aan 5")

        else:
            print("Error: Vul een numerieke waarde in.")

    def clear_frame(self, eenheid):
        """
        De clear_frame() functie verwijdert het gewenste frame. De NamedFrame klasse maakt het mogelijk om de
        juiste te verwijderen.
        :param eenheid: de meegegeven te verwijderen eenheid
        :return: None
        """
        for widget in root.grid_slaves():
            if isinstance(widget, NamedFrame):
                if widget.name == eenheid.portstr:
                    widget.destroy()
                    for eenheid in eenheidlijst:
                        if eenheid.portstr == widget.name:
                            anim.pop(eenheidlijst.index(eenheid))
                            eenheidlijst.remove(eenheid)
                            eenheid.line.remove()
                            eenheid.__del__()
                            ax.legend(title="Legend")

    def create_menu(self):
        """
        De create_menu() functie maakt een nieuw scherm waarin de About Us informatie te zien is.
        :return: None
        """
        menu_f = Toplevel(self.master, height=400)
        menu_f.iconbitmap('snek.ico')
        menu_f.title('Snek Central: About Us')

        spacer_links = Frame(menu_f, width=50)
        spacer_links.grid(row=0, column=0)

        who_are_we = Label(master=menu_f, text="\
        Who are we?\n We are four students currently studying IT at the Hanzehogeschool in Groningen.\n")
        who_are_we.grid(row=1, column=1)

        what_did_we_make = Label(master=menu_f, text="\
        What did we make?\n We made a GUI using Python. This GUI controls two Arduino Uno's. \n \
        The Arduino Uno's both control a different sensor. The sensor data is displayed in the GUI.\n \
        With this data we can control a sunscreen.\n")
        what_did_we_make.grid(row=2, column=1)

        individuals = Label(master=menu_f, text=
        "Individuals:\n \
        Stefan Jilderda, 406347\n \
        Teun de Jong, 308158\n \
        Stefan Kuppen, 405611\n \
        Jens Maas, 439557\n\n\n")
        individuals.grid(row=3, column=1)

        spacer_onder = Frame(menu_f, height=25, width=25)
        spacer_onder.grid(row=10, column=1)

        spacer_einde = Frame(menu_f, height=20, width=50)
        spacer_einde.grid(row=0, column=100)


def random_kleur(kleuren):
    """
    Kiest uit een lijst met kleuren een willekeurige kleur
    :param kleuren: Een lijst met kleuren waaruit gekozen kan worden
    :return: De gekozen kleur
    """
    i = random.randint(0, len(kleuren) - 1)
    return kleuren.pop(i)


def check_for_arduinos():
    """
    De check_for_arduinos() functie checkt of er arduino's zijn aangesloten via USB en wanneer deze nog niet
    zijn toegevoegd aan de eenheidlijst voegt het ze toe.
    :return: None
    """
    try:
        for p in serial.tools.list_ports.comports():
            # print(p)
            if portstring in p.description and p.serial_number is not None:
                temp_eenheid = Eenheid(p.device, 19200)
                if temp_eenheid not in eenheidlijst:
                    eenheidlijst.append(temp_eenheid)
                    app.initialize_units(temp_eenheid, len(eenheidlijst) + 1)
                    anim.append(animation.FuncAnimation(fig=figure, func=temp_eenheid.update_graph, interval=1000))
                    ax.legend(title="Legend")

    except:
        pass


def configure_units(eenheid, data_binnen):
    """
    De configure_units() functie gebruikt de binnengekomen data en bepaalt wat er met de data gedaan wordt. De data
    wordt naar de juiste bestemming gebracht door te zoeken naar een specifieke NamedFrame binnen de widget kinderen
    van de root.

    De arduino's sturen data in 'pakketjes' van 3. De functie kijkt naar het eerste deel van het pakketje en bepaalt
    van wat voor arduino het afkomstig is (een temperatuur- of lichtsensor).

    In het volgende segment van het pakketje staat om wat voor soort data het gaat dus of het data is van
    de ultrasonoorsensor of van de eerder bepaalde temperatuur- of lichtsensor.

    In het laatste segment van het pakketje staat de daadwerkelijke data.

    :return: None
    """

    if data_binnen[0] == 1:
        for frame in root.winfo_children()[3:]:
            if isinstance(frame, NamedFrame):
                if frame.name == eenheid.name:
                    frame.winfo_children()[2].configure(text=eenheid.name + ': Light')

    if data_binnen[0] == 2:
        for frame in root.winfo_children()[3:]:
            if isinstance(frame, NamedFrame):
                if frame.name == eenheid.name:
                    frame.winfo_children()[2].configure(text=eenheid.name + ': Temperature')

    if data_binnen[1] == 1:
        for frame in root.winfo_children()[3:]:
            if isinstance(frame, NamedFrame):
                if frame.name == eenheid.name:
                    eenheid.arm_data = data_binnen[2]
                    frame.winfo_children()[8].configure(text='Current arm extend: ' + str(eenheid.arm_data))

    if data_binnen[1] > 1:
        if len(eenheid.data) > 20:
            eenheid.data.pop(0)
        eenheid.data.append(data_binnen[2])

if __name__ == '__main__':
    # maakt de benodigde 'globale' lijsten aan waarin data of objecten worden opgeslagen
    eenheidlijst = []
    anim = []
    labellijst = []
    #vorigewaarden = [1,2,50]

    kleuren = ['red', 'blue', 'black', 'green']

    # maakt de globale figuur aan voor de grafiek waar vervolgens de eenheden hun lijn op kunnen plotten
    figure = Figure(figsize=(6, 4), dpi=100, edgecolor="red")
    figure.set_facecolor('gray')
    ax = figure.add_subplot(1, 1, 1, ylabel='Value', xlabel='Time in data points', xticks=[x for x in range(20)])
    ax.set_ylim(0, 260)


    # zoekt in eerste instantie naar de aangesloten arduino's

    for p in serial.tools.list_ports.comports():
        # print(p)
        if portstring in p.description and p.serial_number is not None:
            temp_eenheid = Eenheid(p.device, 19200)
            if temp_eenheid not in eenheidlijst:
                eenheidlijst.append(temp_eenheid)

    # initieert de legenda na het vinden van de arduinos
    ax.legend(title="Legend")

    # initieert het scherm
    root = Tk()
    root.iconbitmap('snek.ico')
    app = Window(root)
    app.configure(bg='gray')

    # initieert de live feed van de grafiek
    for eenheid in eenheidlijst:
        anim.append(animation.FuncAnimation(fig=figure, func=eenheid.update_graph, interval=1000))


    # de oneindige while loop waarin het programma belandt om de gui te blijven verversen
    while True:
        # eerst kijkt het of er arduino's zijn aangesloten die nog niet in de eenheidlijst staan
        check_for_arduinos()

        # daarna wordt er per arduino naar de binnengekomen data gekeken in pakketjes met grootte 3
        for eenheid in eenheidlijst:
            data_binnen = []
            if eenheid.in_waiting > 0:
                try:
                    for i in range(3):
                        s = eenheid.read(1)
                        data_binnen.append(int(s.hex(), 16))


                    print(data_binnen)
                    time.sleep(2)

                except:
                    print("Er ging iets mis.")


                # vervolgens wordt de data gebruikt
                configure_units(eenheid, data_binnen)


        # dit doet hetzelfde als de root.mainloop() en zorgt ervoor dat het in een eindeloze loop belandt.
        root.update_idletasks()
        root.update()
