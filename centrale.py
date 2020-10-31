from tkinter import *

def show_menu():
    #maak het frame leeg
    for widget in frame.winfo_children():
        widget.destroy()

    #plemp tekst in het menu
    label = Label(frame, text="MENU")
    label.pack()

    #maak knoppen aan
    knop1 = Button(frame, text="Instellingen", command=open_settings)
    knop1.pack(pady=5)

    knop2 = Button(frame, text="Grafiek", command=open_graph)
    knop2.pack(pady=5)

    knop3 = Button(frame, text="About Us", command=open_about)
    knop3.pack(pady=5)


def open_settings():
    for widget in frame.winfo_children():
        widget.destroy()

    #plemp tekst in het menu
    label = Label(frame, text="SETTINGS", fg="blue")
    label.pack()

    knop1 = Button(frame, text="Menu", command=show_menu)
    knop1.pack(pady=4)

    frame.pack()

def open_graph():
    #maak het frame leeg
    for widget in frame.winfo_children():
        widget.destroy()

    #plemp tekst in het menu
    label = Label(frame, text="GRAFZIIIIEEEEK", fg="red")
    label.pack()

    knop1 = Button(frame, text="Menu", command=show_menu)
    knop1.pack(pady=4)

    frame.pack()


def open_about():
    about_window = Toplevel(master)
    about_window.title("About Us")
    about_window.geometry("400x200")
    Label(about_window, text="Gemaakt door: Teun en Jens").pack(pady=25)



master = Tk()
master.geometry("300x200")
master.title("Zonnescherm Centrale")

# maak nieuw frame aan
frame = Frame(master)
frame.pack()

show_menu()

#canvas = Canvas(frame1, width=1000, height=500)
#canvas.pack()


master.mainloop()