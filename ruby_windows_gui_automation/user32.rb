#!/usr/bin/env ruby
# == User32.rb
#       PoC for how to interface with the User32 subsystem on windows
#       aka: Control windows and stuff, simulate keyboard/mouse.
#       This is particularly useful for app pentesting because you can use this
#       to "enable" disabled menus/buttons etc.
#       This will demonstrate rudimentary window messaging via the user32 API
#       to control Calc.exe
#       
# == Author
#   Stephen A. Ridley
#       stephen@sa7ori.org
#
# == The Details
# 
#       Use Winspector Pro (a free download)
#       to observe messages being sent to your windowobject
#           http://www.windows-spy.com/       
#       Also use MSDN and the #defines winuser.h to get the numeric
#       values of some of the SendMessage message opcodes. EG:
#            http://msdn.microsoft.com/en-us/library/ms646280(VS.85).aspx
#       I have included a part of some sample C code from an old (now defunct, cuz someone
#       burned me) private remote Yahoo messenger exploit I've been sitting on. 
#       It is in the DATA block.
#--
#   ATTENTION ALL YE HIGH-AND-MIGHTY RUBY STYLE CRITICS:
#   Let me put this in the language you love so much:
#       if the_code.does_it_work?: You::stfu() end
#++
#   NOTE TO MYSELF
#       You cant EnableWindow/SendMessage/SetWindowLong(style) on a button
#       object that has a windowhook/proc defined. This is why when you
#       "re-enable" a disabled button it still remains unclickable. To make this
#       work you need to OpenProcess() and physically modify memory. There might
#       also be a way to do this with "SetWindowsHookEx()". I will have to look
#       into how does this.

require 'Win32API'
require 'irb'
require 'optparse'
require 'rdoc/usage'
require 'zlib'
 
# From winuser.h
#http://doc.ddart.net/msdn/header/include/winuser.h.html
WM_ENABLE = 0x000A
WM_PAINT = 0x000F
WM_KEYDOWN = 0x0100
WM_KEYUP = 0x0101
WM_CHAR = 0x0102
WM_USER = 0x0400
WM_LBUTTONDOWN = 0x0201
WM_LBUTTONUP = 0x0202
WM_MOUSEACTIVATE = 0x0021
WM_NCPAINT=0x0085
RDW_INVALIDATE = 0x0001
RDW_INTERNALPAINT = 0x0002
GWL_STYLE = -16
WS_DISABLED = 0x08000000
 
EnableWindow = Win32API.new("user32", "EnableWindow", 'LL', 'L')
SendMessage = Win32API.new("user32","SendMessage", ['L'] * 4, 'L')
RedrawWindow = Win32API.new("user32","RedrawWindow", ['L'] * 4, 'L')
FindWindowEx = Win32API.new("user32","FindWindowEx", 'LLPP','L')
IsWindowEnabled = Win32API.new("user32","IsWindowEnabled",'L','L')
UpdateWindow = Win32API.new("user32","UpdateWindow",'L','L')
GetWindowLong = Win32API.new("user32", "GetWindowLong", 'LL','L')
SetWindowLong = Win32API.new("user32", "SetWindowLong", 'LLL','L')

def click_button(sm, button)
  sm.call(button, 0x0021, 1, 513)
  sm.call(button,0x0201,0,0x000A0021)
  sm.call(button,0x202,0,0x000A0021)
end

# Open our backpack...
#   Made with: puts [File::open("winuser_test.cpp.gz").read()].pack('u')
def displayC()
    stuff = DATA.read()
    thingz = stuff.unpack('u')
    gz = Zlib::GzipReader.new(StringIO.new(thingz.join())) #trick to convert string
                                                           #to input stream
    thing = gz.read()
    puts thing
end

options={}
options[:e] = false
opts = OptionParser.new()
opts.on("-h", "--help", "You're looking at it."){puts opts.to_s;Kernel.exit(0)}
opts.on("-e", "--extract", "Display a C example of user32 interaction."){|blah| options[:e] = true}
opts.parse(ARGV) rescue puts opts.to_s
if options[:e] then
    displayC()
    Kernel.exit(0)
end
puts "Finding the Start Menu..."
sleep(1)
startbar = FindWindowEx.call(0,0,"Shell_TrayWnd",0)
puts "Finding Start Button..."
start_button = FindWindowEx.call(startbar,0,0,"start")
click_button(SendMessage, start_button)
puts("I cant find what I am looking for there, so I am launching the Calculator.")
sleep(1)
click_button(SendMessage, start_button)
crap = IO.popen('calc.exe')
sleep(2)
puts("Finding Calculator window...")
calculator = FindWindowEx.call(0,0,"SciCalc",0)
#edit_area = FindWindowEx.call(notepad,0,0,"Edit")
#calc_win = FindWindowEx.call(0,0,0,"Calculator")
#= FindWindowEx.call(anchor,0,0,"&Cancel")
#ok = FindWindowEx.call(anchor,0,0,"&OK")
#password = FindWindowEx.call(0,0,0,"&Password:")
#= GetWindowLong.call(GWL_STYLE)
#cancel_style |= WS_DISABLED #If the window style does not have WS_DISABLED, this sets it.
#SetWindowLong.call(cancel, GWL_STYLE, cancel_style) 
 
#EnableWindow(cancel, 0) #This makes the button unclickable but not greyed out.
#EnableWindow(cancel,1) #This makes the button clickable again, but doesnt change color.
 
#ok_style = GetWindowLong.call(ok,  GWL_STYLE)
#ok_style &= ~WS_DISABLED #If the window style has WM_DISABLED this unsets this style.
#SetWindowLong.call(ok, GWL_STYLE, ok_style)
one = FindWindowEx.call(calculator, 0,0,"1")
five = FindWindowEx.call(calculator, 0,0,"5")
six = FindWindowEx.call(calculator, 0,0,"6")
eight = FindWindowEx.call(calculator, 0,0,"8")
two = FindWindowEx.call(calculator, 0,0,"2")
times = FindWindowEx.call(calculator, 0,0,"*")
plus = FindWindowEx.call(calculator, 0,0,"+")
equal = FindWindowEx.call(calculator, 0,0,"=")
c = FindWindowEx.call(calculator, 0,0,"C")
ce = FindWindowEx.call(calculator, 0,0,"CE")

click_button(SendMessage, one)
sleep(1)
click_button(SendMessage, five)
sleep(1)
click_button(SendMessage, six)
sleep(1)
click_button(SendMessage, six)
sleep(1)
click_button(SendMessage, eight)
sleep(1)
click_button(SendMessage, times)
sleep(1)
click_button(SendMessage, two)
sleep(1)
click_button(SendMessage, plus)
sleep(1)
click_button(SendMessage, one)
sleep(1)
click_button(SendMessage, equal)
EnableWindow.call(c, 0)
EnableWindow.call(ce, 0)
puts "Try to click the 'C' or 'CE' buttons..."
sleep(5)
puts "Ok hit <ENTER> to continue.";gets()
EnableWindow.call(c, 1)
EnableWindow.call(ce, 1)
puts "ok you get the idea...quitting."
Kernel.exit(0)
__END__
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
*OP%-YHH+BQ0`````

