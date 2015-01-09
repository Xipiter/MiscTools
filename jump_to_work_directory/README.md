SetWorkFocus
=====

This is just a simple thing to let you run the command "setworkfocus" from
anywhere. It will then take your current working directory and smash it into
your bash profile, so that next time that you open a shell it drops you into your
"workfocus" directory. If you are like me, you spend most of your time deep in Git/SVN checkouts
or on weird file share paths all of which SUUUUCK to type in to get to in a new terminal ....this fixes that.

# Installation
1. Make the .py or .rb (whichever version you choose to use) executable and copy to your $PATH(i.e. /usr/bin)
2. Copy the ```dot_profile``` file to your ```~/.bash_profile``` or the equivalent for whichever shell you are using
3. Modify the line in ```setworkfocus``` that reads ```bashrc = os.getenv("HOME")+"/.profile"``` to reference your correct "profile" file depending on your shell. If using bash, you won't have to change this line.


# Usage:
```
tachiro:ropgenius s7$ setworkfocus
Found "export" but no "WORKFOCUS" on line #4.
Found "export" but no "WORKFOCUS" on line #8.
Found "export" but no "WORKFOCUS" on line #9.
Found "export" but no "WORKFOCUS" on line #10.
Found "export" but no "WORKFOCUS" on line #12.
Found "export" but no "WORKFOCUS" on line #13.
Found "export WORKFOCUS=" on line #14 of /Users/s7/.bash_profile
Changed WORKFOCUS to /Users/s7/Desktop/WORKDRIVE/CHECKOUTS/XipiterHQ/Xipiter_OLD/projects/ropgenius
tachiro:ropgenius s7$ 
```

Now when opening any new shell:

```
Last login: Fri Jan  9 12:29:15 on ttys004


    ************ WORKFOCUS **************
    /Users/s7/Desktop/WORKDRIVE/CHECKOUTS/XipiterHQ/Xipiter_OLD/projects/ropgenius
    *************************************

tachiro:ropgenius s7$ pwd
/Users/s7/Desktop/WORKDRIVE/CHECKOUTS/XipiterHQ/Xipiter_OLD/projects/ropgenius
tachiro:ropgenius s7$ 
```
