from tkinter import *
from functools import partial
from matplotlib import animation
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
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

# - Het type eenheid herkennen aan de hand van de aansluiting. Dit is echter afhankelijk van Stefan en Stefan hoe
#   zij dit meegeven


class Eenheid(serial.Serial):

    def __init__(self, port, baudrate):
        super().__init__(port, baudrate)
        self.data = 20 * [55]
        self.i = 0
        self.figure = Figure(figsize=(6, 4), dpi=100, edgecolor="red")
        self.subplot = self.figure.add_subplot(1, 1, 1)
        self.line, = self.subplot.plot(self.data, color='black')
        self.subplot.set_ylim(min(self.data) - 5, max(self.data) + 5)

    def update_graph(self, i):
        self.subplot.set_ylim(min(self.data) - 5, max(self.data) + 5)
        self.line.set_data([x for x in range(len(self.data))], self.data)


class Window(Frame):

    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.master = master
        self.init_window()

    def init_window(self):
        # global time_l
        self.master.title("Python Centrale")

        menu = Menu(self.master)
        self.master.config(menu=menu)

        file = Menu(menu, tearoff=0)
        file.add_command(label='Settings')
        file.add_command(label='Graph')
        file.add_command(label='Exit', command=exit)
        menu.add_cascade(label='File', menu=file)

        edit = Menu(menu, tearoff=0)
        for eenheid in eenheidlijst:
            edit.add_command(label='Remove ' + eenheid.port, command=partial(self.remove_from_list, eenheid))
        menu.add_cascade(label='Edit', menu=edit)

        time_l = Label(self.master, text="", fg='pink', font=tkFont.Font(family="Comic Sans MS", size=30))
        time_l.grid(row=0, column=0)
        i = 0
        # spacer_begin = Frame(self.master, height=50, width=50)
        # spacer_begin.grid(row=0, column=0)

        for eenheid in eenheidlijst:
            eenheid.i = i
            eenheid_f = Frame(self.master, height=400, width=400)
            eenheid_f.grid(row=1, column=i + 1)
            spacer_tussen = Frame(eenheid_f, height=50, width=50)
            spacer_tussen.grid(row=1, column=i * 5)

            # eenheid.line = eenheid.subplot.plot(eenheid.data)

            canvas = FigureCanvasTkAgg(eenheid.figure, master=eenheid_f)
            canvas.draw()
            canvas.get_tk_widget().grid(row=1, column=(i * 5 + 1), columnspan=3)

            b1 = Button(eenheid_f, text="In-/Uitrollen", padx=10)
            b1.grid(row=2, column=i * 5 + 1, pady=10)

            b2 = Button(eenheid_f, text="Grafiek/Instellingen", padx=10)
            b2.grid(row=2, column=i * 5 + 2, pady=10)

            b3 = Label(eenheid_f, text=eenheid.port, padx=30, pady=3.5, relief="solid", fg="white", bg="pink",
                       font=tkFont.Font(family="Comic Sans MS", size=30))
            b3.grid(row=2, column=i * 5 + 3, pady=10)

            i += 1

        spacer_einde = Frame(self.master, height=50, width=50)
        spacer_einde.grid(row=0, column=100)

    def remove_from_list(self, eenheid):
        global eenheidlijst
        temp = []
        for x in eenheidlijst:
            if x.port != eenheid.port:
                temp.append(x)

        eenheidlijst = temp

        self.clear_frame()
        self.init_window()

        if len(self.master.grid_slaves()) == 2:
            self.create_menu()

    def clear_frame(self):
        for widget in root.grid_slaves():
            widget.destroy()

    def create_menu(self):
        menu_f = Frame(self.master, height=400)
        menu_f.grid(row=0, column=0)

        spacer_links = Frame(menu_f, width=50)
        spacer_links.grid(row=0, column=0)
        menu_button = Button(menu_f, text="Waddup Youtuuub, Youtuuuuuuuub",
                             command=partial(webbrowser.open, "https://www.youtube.com/watch?v=CXkG8TOJ2BY"))
        menu_button.grid(row=1, column=1)


# def clock():
#     time = datetime.datetime.now().strftime("%S")
#     time_l.config(text=time)
#     root.after(1000, clock)

if __name__ == '__main__':
    eenheidlijst = []
    for p in serial.tools.list_ports.comports():
        if "USB Serial Device (COM" in p.description and p.serial_number is not None:
            eenheidlijst.append(Eenheid(p.device, 19200))

    # eenheidlijst.append(Eenheid('COM1', 19200))

    root = Tk()
    app = Window(root)

    while True:
        for eenheid in eenheidlijst:
            try:
                # print(eenheid.read_all())
                s = eenheid.read()
                if len(eenheid.data) > 20:
                    eenheid.data.pop(0)

                eenheid.data.append(int(s.hex(), 16))

            except:
                print("Kan niet lezen niet, ouwe")

            # eenheid.close()
            ani = animation.FuncAnimation(fig=eenheid.figure, func=eenheid.update_graph, interval=500)

        root.update_idletasks()
        root.update()
