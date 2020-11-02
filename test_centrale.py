from tkinter import *
from functools import partial
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import serial


class Eenheid():

    def __init__(self, name):
        self.name = name

eenheidlijst = []

for i in range(8):
    try:
        ser = serial.Serial('COM' + str(i), 9600, timeout=0)
        if isinstance(ser, serial.Serial):
            eenheidlijst.append(ser)
            print("Aansluiting gevonden op COM" + str(i))
    except:
        print("Niets aangesloten op port COM" + str(i))


# eenheid1 = Eenheid("1")
# eenheid2 = Eenheid("2")
# eenheid3 = Eenheid("3")
#
# for x in [eenheid1, eenheid2, eenheid3]:
#     eenheidlijst.append(x)

class Window(Frame):

    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.master = master
        self.init_window()

    def init_window(self):
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
            edit.add_command(label='Remove ' + eenheid.name, command=partial(self.remove_from_list, eenheid))
        menu.add_cascade(label='Edit', menu=edit)

        i = 0
        # spacer_begin = Frame(self.master, height=50, width=50)
        # spacer_begin.grid(row=0, column=0)

        for eenheid in eenheidlijst:
            eenheid_f = Frame(self.master, height=400, width=400)
            eenheid_f.grid(row=1, column=i+1)
            spacer_tussen = Frame(eenheid_f, height=50, width=50)
            spacer_tussen.grid(row=1, column=i * 5)

            fig = Figure(figsize=(6,4), dpi=100, edgecolor="red")
            fig.add_subplot(111).plot(np.random.normal(8000, 2500, 50))

            canvas = FigureCanvasTkAgg(fig, master=eenheid_f)
            canvas.draw()
            canvas.get_tk_widget().grid(row=1, column=(i*5+1), columnspan=3)

            # graph = Frame(eenheid_f, bg="#d8eded", height=300, width=400)
            # graph.grid(row=1, column=(i * 5 + 1), columnspan=3)

            b1 = Button(eenheid_f, text="In-/Uitrollen", padx=10)
            b1.grid(row=2, column=i * 5 + 1, pady=10)

            b2 = Button(eenheid_f, text="Grafiek/Instellingen", padx=10)
            b2.grid(row=2, column=i * 5 + 2, pady=10)

            b3 = Button(eenheid_f, text=eenheid.name, padx=10)
            b3.grid(row=2, column=i * 5 + 3, pady=10)

            i += 1

        spacer_eind = Frame(self.master, height=50, width=50)
        spacer_eind.grid(row=0, column=100)

    def remove_from_list(self, eenheid):
        global eenheidlijst
        temp = []
        for x in eenheidlijst:
            if x.port != eenheid.port:
                temp.append(x)

        eenheidlijst = temp

        self.clear_frame()
        self.init_window()

        if len(root.grid_slaves()) == 1:
            self.create_menu()
        else:
            pass


    def clear_frame(self):
        for widget in root.grid_slaves():
            widget.destroy()

    def create_menu(self):
        menu_f = Frame(self.master, height=400)
        menu_f.grid(row=0, column=0)

        menu_button = Button(menu_f, text="Waddup Youtuuub")
        menu_button.grid(row=2, column=3)



root = Tk()

app = Window(root)

root.mainloop()
