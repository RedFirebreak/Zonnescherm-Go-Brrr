import random
import time
from tkinter import *
from functools import partial
from matplotlib import animation
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
from matplotlib.axes import Axes
import serial.tools.list_ports
import serial
import webbrowser
import tkinter.font as tkFont


### WAT ER NOG MOET GEBEUREN:

#   ~ DONE
# - Binnengekomen data moet op een bepaalde manier opgeslagen worden zodat het mogelijk is voor matplotlib
#   om er grafieken van te maken die elke zoveel seconden worden geupdate. Zie deze link:
#   https://makersportal.com/blog/2018/2/25/python-datalogger-reading-the-serial-output-from-arduino-to-analyze-data-using-pyserial

#   DONE
# - Data voor de grafiek mag maximaal x eenheden zijn, wanneer het x + 1e datapunt binnenkomt zal het
#   oudste worden verwijderd.

# - Zorgen dat het gehele programma draait en blijft zoeken naar updates in data om deze vervolgens uit te voeren.
#   Misschien 1 functie die alles verzamelt en dan in één klap de GUI update. Dit kan bij de timer countdown inzitten

# - In plaats van het volledig verwijderen van een besturingseenheid via Edit het mogelijk maken om hem alleen te
#   verbergen zodat het ook weer terug te halen is.

# - Een label die de status van een eenheid weergeeft. Wanneer je op de knop drukt zal de status automatisch veranderen
#   (eventueel kunnen we de tijd hoe lang het duurt om een scherm in of uit te rollen aanpassen aan de maximale
#   uitrol lengte?)

# - De eenheid moet ook data kunnen ontvangen zodat wanneer je op rol in / rol uit drukt de arduino dit binnenkrijgt en
#   ervoor zorgt dat de lampjes gaan branden zoals het hoort om zo te simuleren dat het zonnescherm bezig gaat.

#   ~ DONE
# - Het type eenheid herkennen aan de hand van de aansluiting. Dit is echter afhankelijk van Stefan en Stefan hoe
#   zij dit meegeven


class Eenheid(serial.Serial):

    def __init__(self, port, baudrate):
        super().__init__(port, baudrate)
        self.data = 20 * [0]
        self.i = 0
        self.kleur = random_kleur(kleuren)
        self.line, = ax.plot(self.data, color=self.kleur, label=self.portstr)


    def update_graph(self, i):
        #self.subplot.set_ylim(min(self.data) - 5, max(self.data) + 5)
        self.line.set_data([x for x in range(len(self.data))], self.data)

class NamedFrame(Frame):

    def __init__(self, name, master, background):
        super().__init__(master=master, bg=background)
        self.name = name



class Window(Frame):

    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.master = master
        self.init_window()
        self.minafstand = 5
        self.maxafstand = 160

    def init_window(self):
        self.master.title("Snek Central")


        menu = Menu(self.master)
        self.master.config(menu=menu, bg='gray')

        file = Menu(menu, tearoff=0)
        file.add_command(label='About Us', command=self.create_menu)
        file.add_command(label='Exit', command=root.destroy)
        menu.add_cascade(label='File', menu=file)

        spacer_begin = Frame(self.master, height=50, width=50, bg='gray')
        spacer_begin.grid(row=0, column=0)

        i = 0
        graph_frame = NamedFrame("graph", master=self.master, background='gray')
        graph_frame.grid(row=1, column=i+1)

        canvas = FigureCanvasTkAgg(figure, master=graph_frame)
        canvas.draw()
        canvas.get_tk_widget().grid(row=1, column=(i * 5 + 1), columnspan=3, rowspan=8)

        i = 1
        for eenheid in eenheidlijst:
            self.initialize_units(eenheid, i)
            i += 1

        spacer_einde = Frame(self.master, height=50, width=50, bg='gray')
        spacer_einde.grid(row=0, column=100)

        spacer_onder = Frame(self.master, height=50, width=50, bg='gray')
        spacer_onder.grid(row=100, column=1)

    def initialize_units(self, eenheid, i):
        eenheid.i = i
        eenheid_f = NamedFrame(eenheid.portstr, self.master, background='gray')
        eenheid_f.configure(highlightbackground='black', highlightthickness=2)
        eenheid_f.grid(row=1, column=i + 1, padx=10)
        spacer_boven = Frame(eenheid_f, height=25, width=50, background='gray')
        spacer_boven.grid(row=0, column=i * 5, columnspan=3)
        spacer_tussen = Frame(eenheid_f, height=50, width=50, background='gray')
        spacer_tussen.grid(row=1, column=i * 5)

        l1 = Label(eenheid_f, text=eenheid.port, padx=30, pady=3.5, relief="solid", borderwidth='0.5', fg="black", bg="#849db5")
        l1.grid(row=1, column=i * 5 + 1)

        min_rol = Label(eenheid_f, text='Minimale rolafstand:', bg='gray')
        min_rol.grid(row=10, column= i * 5 + 1, padx=10)

        l2 = Entry(eenheid_f)
        l2.insert(END, '5-160')
        l2.grid(row=20, column= i * 5 + 1, padx=10)

        b1 = Button(eenheid_f, text="Save", command=partial(self.set_min_uitrolafstand, l2, eenheid), relief="solid", borderwidth='0.5', fg="black", bg="#849db5")
        b1.grid(row=20, column= i * 5 + 2)

        max_rol = Label(eenheid_f, text='Maximale rolafstand:', bg='gray')
        max_rol.grid(row=29, column=i * 5 + 1, padx=10)

        l3 = Entry(eenheid_f)
        l3.insert(END, '5-160')
        l3.grid(row=30, column=i * 5 + 1)

        b2 = Button(eenheid_f, text="Save", command=partial(self.set_max_uitrolafstand, l3, eenheid), relief="solid", borderwidth='0.5', fg="black", bg="#849db5")
        b2.grid(row=30, column=i * 5 + 2)

        b3 = Button(eenheid_f, text="Rol in / Rol uit", padx=10, relief="solid", borderwidth='0.5', fg="black", bg="#849db5")
        b3.grid(row=40, column=i * 5 + 1, pady=10)

        b4 = Button(eenheid_f, text="Destroy", padx=28, command=partial(self.clear_frame, eenheid), relief="solid", borderwidth='0.5', fg="black", bg="#849db5")
        b4.grid(row=50, column=i * 5 + 1, pady=10)

        spacer_einde = Frame(eenheid_f, height=50, width=25, bg='gray')
        spacer_einde.grid(row=25, column=100)


    def set_max_uitrolafstand(self, l3, eenheid):
        afstand = l3.get()
        if afstand.isnumeric():
            afstand = int(afstand)
            if 160 > afstand > self.minafstand:
                self.maxafstand = afstand
                #eenheid.write(self.maxafstand)
                print(self.maxafstand)
            else:
                print(  "noooooob")
        else:
            print("nooooooooooooooooooooooooooooooob")


    def set_min_uitrolafstand(self, l2, eenheid):
        afstand = l2.get()
        if afstand.isnumeric():
            afstand = int(afstand)
            if self.maxafstand > afstand > 5:
                self.minafstand = afstand
                #eenheid.write(self.minafstand)
                print(self.minafstand)
            else:
                print("noooob")

        else:
            print("noooooooooooooooooooob")



    def clear_frame(self, eenheid):
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
        menu_f = Toplevel(self.master, height=400)

        spacer_links = Frame(menu_f, width=50)
        spacer_links.grid(row=0, column=0)

        who_are_we = Label(master=menu_f,text="\
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

        spacer_einde = Frame(menu_f, height=20, width=50)
        spacer_einde.grid(row=0, column=100)





def random_kleur(kleuren):
    i = random.randint(0, len(kleuren) -1)
    return kleuren.pop(i)

def check_for_arduinos():
    #print("check for arduinos")
    try:
        for p in serial.tools.list_ports.comports():
            #print(p)
            if "USB Serial Device (COM" in p.description and p.serial_number is not None:
                temp_eenheid = Eenheid(p.device, 19200)
                if temp_eenheid not in eenheidlijst:
                    eenheidlijst.append(temp_eenheid)
                    app.initialize_units(temp_eenheid, len(eenheidlijst) + 1)
                    anim.append(animation.FuncAnimation(fig=figure, func=temp_eenheid.update_graph, interval=1000))
                    ax.legend(title="Legend")

    except:
        pass


if __name__ == '__main__':
    eenheidlijst = []
    anim = []
    kleuren = ['red', 'blue', 'black', 'green', 'brown']
    figure = Figure(figsize=(6, 4), dpi=100, edgecolor="red")
    figure.set_facecolor('gray')
    ax = figure.add_subplot(1, 1, 1)
    ax.set_ylim(0, 150)

    for p in serial.tools.list_ports.comports():
        #print(p)
        if "USB Serial Device (COM" in p.description and p.serial_number is not None:
            temp_eenheid = Eenheid(p.device, 19200)
            if temp_eenheid not in eenheidlijst:
                eenheidlijst.append(temp_eenheid)

    ax.legend(title="Legend")

    root = Tk()
    root.iconbitmap('snek.ico')
    app = Window(root)
    app.configure(bg='gray')

    for eenheid in eenheidlijst:
        anim.append(animation.FuncAnimation(fig=figure, func=eenheid.update_graph, interval=1000))

    i=0

    while True:
        check_for_arduinos()

        for eenheid in eenheidlijst:
            try:
                s = eenheid.read()
                if len(eenheid.data) > 20:
                    eenheid.data.pop(0)
                eenheid.data.append(int(s.hex(), 16))

            except:
                pass

        root.update_idletasks()
        root.update()