MSR605
======

A C/CPP Library for interacting with the the MSR605 (and other MSR206 compatible) Magstripe Reader/Writers.

Added:
* Raw Write support (for the moment only for BPC 8 8 8 mode)
* Erase functionality
* Set High/Low Coercivity

You are free to fork this repo, add the functionalities you need, and do a push request.

Modified a little program for testing... (don't forget to install the library or set the loader path to the current dir)

Examples
=======
`./msr605 8 8 8 1 r` Read Raw, BPC 8 8 8

`./msr605 8 8 8 2 r` Read ISO, BPC 8 8 8 

`./msr605 8 8 8 1 w` Write Raw, BPC 8 8 8

`./msr605 8 8 8 1 e` Erase

TODO
=======
Add functionalities remaining, like write in ISO mode, or change the BPI
