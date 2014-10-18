#I am using osx-pl2303-0.3.1-10.4-universal.dmg which is the OSX usb-serial
#driver, I believe this is the only reason why the devices get these "PL" names.
ls /dev/ | grep PL | grep tty
