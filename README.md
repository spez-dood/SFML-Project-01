Run the makefile (linux only atm), or compile the project including SFML graphics libraries.
It should build default-settings.ini and settings.ini on the initial run/if settings.ini is missing.

Currently, it's just a black window with a single button with some text on it.
The button is clickable, and changes color when mouse hovers over it.
So far it just disappears when it's clicked.

Press escape as an extra method of closing the window and exiting the program.

config.h header comes from [SimpleIni](https://github.com/brofield/simpleini#) (renamed from SimpleIni.h):
