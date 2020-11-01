from tkinter import *
import serial

class Eenheid():

    def __init__(self, name):
        self.name = name

eenheidlijst = []
for i in range(8):
    try:
        ser = serial.Serial('COM'+str(i), 9600, timeout=0)
        if isinstance(ser, serial.Serial):
            eenheidlijst.append(ser)
            print("Aansluiting gevonden op COM"+str(i))
    except:
        print("Niets aangesloten op port COM"+str(i))


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
            edit.add_command(label='Remove '+ eenheid.name, command=lambda: self.remove_from_list(eenheid))
        menu.add_cascade(label='Edit', menu=edit)

        i = 0
        spacer_begin = Frame(self.master, height=50, width=50)
        spacer_begin.grid(row=0, column=0)

        for eenheid in eenheidlijst:

            spacer_tussen = Frame(self.master, height=50, width=50)
            spacer_tussen.grid(row=1,column=i*5)

            graph = Frame(self.master, bg="#d8eded", height=300, width=400)
            graph.grid(row=1,column=(i*5+1), columnspan=3)

            b1 = Button(self.master, text="In-/Uitrollen", padx=10)
            b1.grid(row=2,column=i*5+1, pady=10)

            b2 = Button(self.master, text="Grafiek/Instellingen", padx=10)
            b2.grid(row=2,column=i*5+2, pady=10)

            b3 = Button(self.master, text=eenheid.name, padx=10)
            b3.grid(row=2,column=i*5+3, pady=10)

            i+=1

        spacer_eind = Frame(self.master, height=50, width=50)
        spacer_eind.grid(row=0, column=100)

    ## Dit werkt nog niet helemaal

    def remove_from_list(self, eenheid):
        eenheidlijst.remove(eenheid)
        self.clear_frame()
        self.init_window()

    ## Deze werkt ook nog niet helemaal
    def clear_frame(self):
        for widget in self.winfo_children():
            print(widget)
            widget.destroy()
        self.grid_forget()






root = Tk()

app = Window(root)

root.mainloop()