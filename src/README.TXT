ISO Master
==========
http://littlesvr.ca/isomaster/
An open-source, graphical CD image editor that runs on Linux; and bkisofs, the underlying library used for reading, modifying and writing ISO images.

Compiling
---------
At this early time bkisofs comes in the same package as ISO Master so it doesn't need to be downloaded/built separately.

To compile, simply type

make

at the command prompt in the directory where you extracted the archive.

And to install (optional, you can run from the directory where you compiled it):

su
[enter root password]
make install

Or on something like Ubuntu that won't let you su:
sudo make install
[enter root password]

If you get errors, or even warnings, please do send me the output.

Contact
-------
I would love to hear from you, to know how you made use of either ISO Master or bkisofs, or with bug reports, or feature requests. Please use the form at http://littlesvr.ca/misc/contactandrew.php to send me email. Thanks in advance!

Bug Reports
-----------
Please help me improve ISO Master by sending me bug reports. The more of the following you can do, the easier it will be for me to fix:
- be able to reproduce the bug consistently
- run ISO Master from a terminal window and send me the output (if any) that is printed there
- if it only happens with one image file, tell me what it is and where i can get it (if it's freely available)
- it's better to give me too much detail then too little

Credits
-------
Most of the filename mangling code: the Samba project, http://us2.samba.org/samba/
Everything else: Andrew Smith, http://littlesvr.ca/misc/contactandrew.php

Licence
-------
Any source code for ISO Master and bkisofs is distributed under the GNU General Public Licence version 2.
