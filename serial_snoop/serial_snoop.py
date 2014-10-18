#!/usr/bin/env python
__doc2__ = """


"""

try:
    from Tkinter import *
    from tkMessageBox import *
    from tkFileDialog import askopenfile
except ImportError:
    print "Note: You dont have Tkinter or any of the Tk stuff...\n\n"
from binascii import hexlify
from cmd import Cmd
import thread 
import sys, os, fcntl, time
import serial
import select
import socket
import pprint
import code

stdin=sys.stdin.fileno()
stdout=sys.stdout.fileno()
stderr=sys.stderr.fileno()

class ttyProxy:
    def __init__(self, dev1, dev2):
        self.FILTER=''.join([(len(repr(chr(x)))==3) and chr(x) or '.' for x in range(256)])
        self.dev1 = dev1
        self.dev2 = dev2
        self.tty1 = None
        self.tty2 = None
        self.tty_fd1 = None
        self.tty_fd2 = None
        self.log_fname = "serial_Data_"+time.asctime().replace(" ","_")+".log"
        self.log_h = open(self.log_fname, 'wb')

    def dump(self, src, length=16):
        N=0; result=''
        while src:
            s,src = src[:length],src[length:]
            hexa = ' '.join(["%02X"%ord(x) for x in s])
            s = s.translate(self.FILTER)
            result += "%08X:  %-*s  |%s|\n" % (N, length*3, hexa, s)
            N+=length
        return result

    def init_device1(self):
        """
            Initialize the first serial device and return everthing useful t
        parts of 'self'. 
        """
        self.tty1 = serial.Serial(
            port=self.dev1,
            parity=serial.PARITY_NONE,
            bytesize=serial.EIGHTBITS,
            stopbits=serial.STOPBITS_ONE,
            timeout=1,
            xonxoff=0,
            rtscts=0,
            dsrdtr=0,
            baudrate=115200)

        ## reset them
        self.tty1.setRTS(1)
        time.sleep(0.01)

        ## send "Data Terminal Ready"
        self.tty1.setDTR(1) 
        self.tty1.flushInput()

        # Do non-blocking I/O
        self.tty_fd1 = self.tty1.fileno()
        fcntl.fcntl(self.tty_fd1, fcntl.F_SETFL, fcntl.LOCK_NB)
    
    def init_device2(self):
        """
            Initialize the second serial device and retrn everthing useful to
            parts of 'self'
        """
        self.tty2 = serial.Serial(
            port=self.dev2,
            parity=serial.PARITY_NONE,
            bytesize=serial.EIGHTBITS,
            stopbits=serial.STOPBITS_ONE,
            timeout=1,
            xonxoff=1, #I turned this back on for a *real* analog modem
            rtscts=0,
            dsrdtr=0,
            baudrate=115200)
        self.tty2.setRTS(1)
#        self.tty2.setDTR(1)#comment this line for *real* modems.
        self.tty2.flushInput()
        self.tty_fd2 = self.tty2.fileno()
        fcntl.fcntl(self.tty_fd2, fcntl.F_SETFL, fcntl.LOCK_NB)

    def go(self):
        if self.dev1.lower() not in ("log", "term"):
            try:
                self._go_snoop()
            except KeyboardInterrupt:
                print "Closing serial ports.."
                if self.tty1 != None:
                    self.tty1.close()
                if self.tty2 != None:
                    self.tty2.close()
                print "Closing Log File."
                self.log_h.close()
                print "Exiting..."
                sys.exit(1)

        elif self.dev1.lower() == "log": 
            try:
                self._go_log()
            except KeyboardInterrupt:
                print "Closing serial ports.."
                if self.tty1 != None:
                    self.tty1.close()
                if self.tty2 != None:
                    self.tty2.close()
                print "Closing Log File."
                self.log_h.close()
                print "Exiting..."
                sys.exit(1)

        elif self.dev1.lower() == "term":
            self._go_term()
            
    def _go_snoop(self):
        """
            This function used to handle the passive sniffing between the
devices...but right now it will be used to write both sides of the serial
conversation to the disk, in its entirety in a single LOG.
        """
    # open both serial ports
        print "Logging from BOTH %s <--> %s" % (self.dev1,self.dev2)
        print "Logging from BOTH devices to %s" % self.log_fname
        self.log_h.write("Logging from BOTH devices toev1: %s\ndev2: %s\n" % (self.dev1, self.dev2))
        print "Press Ctrl-C to exit..."

        self.init_device1()
        self.init_device2()
        self.mydev1thr = thread.start_new_thread(self.__device1_listener_thread, ())
        self.mydev2thr = thread.start_new_thread(self.__device2_listener_thread, ())
        # start select loop
        while 1:
          r,w,e = select.select([self.tty_fd1, self.tty_fd2], [], [], 1000)

          if r == [self.tty_fd1]:
            self.chunk = self.tty1.read(8192) #I guess 8192 bytes is our limit here,
                                    #but maybe later this can be a CAN_READ type
                                    #of thing.
            print self.dump(self.chunk)
            self.intercept_hook()

          elif r == [self.tty_fd2]:
            self.chunk = self.tty2.read(8192)
            print self.dump(self.chunk)
            self.intercept_hook()

    def _go_log(self):
        """
            This is the section that handles "logging", like a half functional
            minicom, the proxying functionality is handed by _go_snoop(). 
        """
        # this def is pretty ghetto, its just a copy of _go_snoop()
        print "Logging/Displaying Data From %s" % (self.dev2)
        print "Logging to file %s" % self.log_fname
        self.log_h.write("Single logging of Dev1: %s\n" % (self.dev2))
        print "Press Ctrl-C to exit..."

        self.init_device2()

        # start select loop
        while 1:
            self.chunk = self.tty2.read(8192) #I guess 8192 bytes is our limit here,
                                    #but maybe later this can be a CAN_READ type
                                    #of thing.
            if self.chunk:#This gets rid of printing spaces or newlines even
            #when there isnt anything worthy of printing.
                print self.dump(self.chunk),
                self.intercept_hook()

    def _go_term(self):
        """
            The ghetto terminal
        """
        print "\nEntering Terminal Mode on Dev1: %s" % (self.dev2)
        print "Logging to file %s" % self.log_fname
        self.log_h.write("Entering Terminal Mode on Dev1: %s\n" % (self.dev2))
        self.init_device2()
#        stdin_h = sys.stdin.fileno()
#        fcntl.fcntl(stdin_h, fcntl.F_SETFL, fcntl.LOCK_NB)
        self.mydev2thr = thread.start_new_thread(self.__device2_listener_thread, ())
#        self.__device2_listener_thread()
        mycmd = code.InteractiveConsole(locals())
        banner = """
    Ok be careful, you have access to the full scope EG: dir(self).
    As data comes in, it will be displayed so it may interrupt what you are
    writing.
    
      EXAMPLE:

      To write to the serial port: 
       >>> self.tty2.write("some data") 
    
      TO IMPORT HELPERS
       
       >>> dir(); import project_specific.init_verizon_gprs_modem
        
    CTRL-D to exit.
        """
        mycmd.interact(banner)
        print "Closing serial ports.."
        self.tty2.close()
        print "Closing Log File."
        self.log_h.close()
        print "Exiting..."
        sys.exit(1)
         
    def __device2_listener_thread(self):
        """
            This is the sub that listens on the serial device for input.
            just a helper function for _go_term()
        """
        try:
            while 1:
                self.chunk = self.tty2.read(8192) #I guess 8192 bytes is our limit here,
                                        #but maybe later this can be a CAN_READ type
                                        #of thing.
                print self.dump(self.chunk),
                self.intercept_hook()
        except select.error:
            #do nothing, the thread should just exit
            pass
    
    def __device1_listener_thread(self):
        """
            This is the stub that listens on the the other serial device for
            input. It is just a helper function.
        """
        try:
            while 1:
                self.chunk = self.tty1.read(8192) #I guess 8192 bytes is our limit here,
                                        #but maybe later this can be a CAN_READ type
                                        #of thing.
                print self.dump(self.chunk),
                self.intercept_hook()
        except select.error:
            #do nothing, the thread should just exit
            pass
    
    def _terminal_interactive(self):
        mycmd = code.InteractiveConsole(locals())
        mycmd.interact(banner)
        
    def log_hook(self):
        """ log serial data to file """
        self.log_h.write(self.chunk)
        self.log_h.flush()
         
    def tamper_hook(self):
        """ change the contents of the data written to the serial port """
#        print """
#        \n\nI know this is ghetto, but edit the 'self.chunk' variable to
#        intercept...or 'cont' to continue without editing it."
#        """
        #import pdb; pdb.set_trace() 
        if self.chunk.find(chr(0x5)):
            self.chunk = "hi"

    def intercept_hook(self):
        """
            Perform some transformations on chunk.
        """
#        self.tamper_hook()
        self.log_hook()        
        
class MainWindow(Tk):
    """
        This is just a small container class for the main Tk() window
        class
    """
    def __init__(self):
        Tk.__init__(self)
        self.title(string="INPUT")
        self.tv_frame = tv_frame = Frame(self)
        self.tv = tv = Text(tv_frame, name='text', padx=5, wrap='none',
                foreground="black",
                background="white",
                highlightcolor="white",
                highlightbackground="purple",
                width = 80,
                height = 25)
#                state = 'disabled')
        self.vbar = vbar = Scrollbar(tv_frame, name='vbar')
        vbar['command'] = tv.yview
        vbar.pack(side=RIGHT, fill=Y)
        tv['yscrollcommand'] = vbar.set
        fontWeight = 'normal'
        clear_button = Button(tv_frame, text="Clear Scrollback",
            state="active", command=self.do_thang)
        clear_button.pack(side=BOTTOM, fill=X)    
        #probably should perform tv.config() here
        tv_frame.pack(side=LEFT, fill=BOTH, expand=1)
        tv.pack(side=TOP, fill=BOTH, expand=1)
        tv.focus_set()
        self.apply_bindings()
        self.stderr = PseudoFile(self)
        self.stdout = PseudoFile(self)
        self.stdin = PseudoFile(self)

    def oprint(self, text_to_print):
        """
            This function will be exposed externally to allow others to
        print to our window.
        """
        self.tv.insert(END, text_to_print)

    def clear_scrollback(self):
        """
            Clear the scrollback of the text window.
        """
        self.tv.delete("1.0", END)
        
    def insert_bytes_from_file(self):
        """
            Insert hex escaped bytes from a files on the filesystem.
        """
        f_h = askopenfile('r')
        disp_buf = ""
        if f_h is not None:
            self.fhandle = f_h
            f_bytes = self.fhandle.read()
            print(("%d escaped bytes inserted from file..." % (len(f_bytes))))
            #change this to use slicing
#            wrap_count = 0 
            for byte in f_bytes:
#                if wrap_count < 10:
#                    wrap_count+=1
#                elif wrap_count == 10:
#                    disp_buf+="\\\n\t"
#                    wrap_count = 0
                disp_buf+='\\x'+hexlify(byte)
            self.text.insert(END, "\""+disp_buf+"\"")
#            print "\""+disp_buf+"\""
        else:
            print "Error Opening that File."

    def apply_bindings(self, keydefs=None):
        """
        """
        self.bind_all("<Key>", self.do_nothing)
        pass

    def do_nothing(self, event):
        print hexlify(event.char),

    def startcmd(self, *args):
        cmd = BaseUI()
        cmd.cmdloop()

    def do_thang(self):
        self.mythr = thread.start_new_thread(self.startcmd, ())

class PseudoFile:
    """
        This is used to overload sys.stderr and sys.stdout.
        the object reference passed in on "window_obj" must
        have an "oprint" method.
    """
    def __init__(self, window_obj, encoding=None):
        self.encoding = encoding
        self.window_obj = window_obj
    
    def write(self, s):
        self.window_obj.oprint(s)
    
    def writelines(self, l):
        map(self.write, l)
    
    def flush(self):
        pass

    def isatty(self):
        return True
    

def start_gui():
    rootWindow = MainWindow()
    #We overload the normal stdout/stderr to go to our 
    #output window
    global saved_stderr, saved_stdout, saved_stdin
    saved_stderr = sys.stderr
    saved_stdout = sys.stdout
    saved_stdin = sys.stdout
    sys.stderr = rootWindow.stderr
    sys.stdout = rootWindow.stdout
    sys.stdin = rootWindow.stdin

    rootWindow.mainloop()
    rootWindow.destroy()

class BaseUI(Cmd):
    path =[] #this is how we fake the "path" of commands.
    name = ""

    def __init__(self):
        Cmd.__init__(self)

    def make_prompt(self, name=""):
        test_str = self.get_prompt()
        if test_str.endswith(name+"."):
            test_str += ">> "
            return(test_str)
        #the above is a little hack to test if the path
        #is already set for us, incase this object instance
        #is actually getting reused under the hood.
        self.path.append(name)
        tmp_name = ""
        tmp_name = self.get_prompt()
        tmp_name += ">> "
        return(tmp_name)

    def get_prompt(self):
        tmp_name = ""
        for x in self.path: #iterate through object heirarchy
            tmp_name += (x + ".")
        return tmp_name

    def do_help(self, args):
        """
           Getting help on "help" is kinda silly dont you think?
        """
        #The only reason to define this method is for the help text in the
        #docstring
        Cmd.do_help(self, args)

    def do_hist(self, args):
        """
            Display command history.
        """
#        n = 0
#        for i in self._hist:
#            print "%d: %s" % (n, i)
#            n+=1
        pp = pprint.PrettyPrinter(indent=4)
        pp.pprint(self._hist)

    def emptyline(self):
        """
            Do nothing on empty input line
        """
        pass

    def preloop(self):
        """
            Initialization before prompting user for commands.
            Despite the claims in the Cmd documentaion, Cmd.preloop() is not a
            stub.
        """
        Cmd.preloop(self)   ## sets up command completion
        self._hist    = []      ## No history yet
        self._locals  = {}      ## Initialize execution namespace for user
        self._globals = {}

    def postloop(self):
        """
            Take care of any unfinished business.
            Despite the claims in the Cmd documentaion, Cmd.postloop() is not a
            stub.
        """
        Cmd.postloop(self)   ## Clean up command completion
        print "Exiting..."

    def precmd(self, line):
        """
            This method is called after the line has been input but before
            it has been interpreted. If you want to modifdy the input line
            before execution (for example, variable substitution) do it here.

        """
        self._hist+=[line.strip()]
        return line

    def postcmd(self, stop, line):
        """
            If you want to stop the console, return something that evaluates to
            true. If you want to do some post command processing, do it here.

        """
        return stop

    def default(self, line):
        """
            Called on an input line when the command prefix is not recognized.
            In that case we execute the line as Python code.

        """
        try:
            exec(line) in self._locals, self._globals
        except Exception, e:
            #print e.__class__, ":", e
            print "\tWhat!? I dont understand: %s'" % (e)

    def do_exit(self, args):
        """
            Exits from this tier in the CLI.
            If you need to HARD exit, use 'diemfqr'.
        """
        return 1

    do_quit = do_exit

    def do_die(self, args):
        """
            Hard exit from Proteus.
        """
        print("\nHard exiting...")
        sys.exit(1)

if __name__ == "__main__":
    argv = sys.argv

    if len(argv) != 3:
        usage = """

Serial Snoop Usage.
    SNOOP mode: 
        %s <full-path-to-serialdevice1> <full-path-to-serialdevice2>
        
        This has been modified to just be a sniffer for both sides of the
        coversation.

    LOG MODE:
        %s LOG <full-path-to-serialdevice1>

        This just logs/displays data coming into the serial port.
        Mostly for use with hardware serial taps.
    
    TERMINAL MODE:
        %s TERM <full-path-to-serialdevice1>
    
        This mode not only logs but has "interactivity",
        via interactive interpreter mode, this gives access so that buffers
        can be modified, send() methods called manually, etc etc.

        """ % (sys.argv[0], sys.argv[0], sys.argv[0])
        print usage
        exit(1)
    if argv[1].lower == "log":
        tp = ttyProxy(argv[1], None)
    elif len(argv) == 3:
        tp = ttyProxy(argv[1], argv[2])
    tp.go()
#    start_gui()
#    start_cli()
