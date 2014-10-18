#!/usr/bin/env ruby
# == BinCompare
#
#     This compares two files starting from both the beginning and 
#     reports at which file offsets the files begin to differ. You
#     may specify a "tolerance" which is the number of differences to
#     ignore before reporting back to you.
#
# == Author
#   Stephen A. Ridley
#       stephen@sa7ori.org
# --
# WTF is this? http://optionparser.rubyforge.org/
#   why are there like 20 differnt option parsers all called OptionParser!?
#   and seemingly no attempt at disambiguation. its like an oldskool CPAN clusterfuck
#   all over again. This is what you get when a buncha non english speakers
#   design a programming language in english, and a buncha SanFran 
#   webdesigners, in all their 
#   VWbug-pastelcolored-sushiloving-DeathCabforCutie glory
#   flocked like hipster lemmings because Ruby was "written by a Japanese guy"
#   This language is seriously pissing me off. Why are smart people using
#   this?
#   
#   Clearly I am still in the "hate" phase of learning this language. Its like algebra and 6th
#   grade all over again.

require 'optparse'
require 'rdoc/usage'

# Retrieve the contents of the file _fname_ and it in a big buffer.
# :call-seq:
#       get_file_contents(fname) -> string
#           Return the contents of the file pointed to by "fname" as a string
#           buffer.
def get_file_contents(fname)
    f_h = File.open(fname,'r')
    readbuf = f_h.read()
#    puts "Read #{readbuf.length()} bytes from #{fname}."
    f_h.close()
    return readbuf
end

# Check if the contents of the file _fname_ is is a binary or text file (ASCII
# UTF 8) This is done by checking to see if the file has more than 5% of binary
# data. If it does then it is considered not pure text.
# 
# :call-seq:
#       isBinary?(fname) -> True/False
#           
def isBinary?(fname)
    readbuf = get_file_contents(fname)
    count = 0
#    puts "The size is: #{readbuf.length()}."
    threshold = Integer(readbuf.length() * 0.25)
#    puts "Threshold: #{threshold}."
    readbuf.each_byte do |byte|  #I have to use each_byte because Ruby 1.8.6 (Darwin) is
                                 #fucked and doesnt have String::each_char
        if !(0x20..0x7f).include?(byte) then
            #puts "Non-ascii byte found: #{byte}"
            count+=1 
        end
    end
#    puts "#{count} nonascii bytes found in file."
    if count >= threshold then
        return true
    else 
        return false
    end
end

# Begin comparing the files. Print when the files begin to differ.
# :call-seq:
#       bindiff(file1, file2,tolerance) -> nothing
#           
def bindiff(file1, file2, tol)
    if tol.nil? then tol = 0 end #Ruby seriously needs to fix this nil/0 shit
    i = 0; small = 0; tol_count = 0; 
    f1 = get_file_contents(file1)
    f2 = get_file_contents(file2)
    if f1.length() != f2.length() then
        puts "The two files are of differing lengths."
        if f1.length() < f2.length() then small = f1.length() 
        else small = f2.length() end
    end
    while i <= small
        if f1[i] != f2[i] then 
            tol_count+=1
            if tol_count > tol then puts "Files begin to differ at offset 0x#{i.to_s(16)}" end
        end
        i+=1
    end
    if tol_count == 0 then puts "Files are exactly the same." end
end

# --
#I see how to specify optional and mandatory  arguments, I dont see how to specify
#optional or mandatory switches. 
#if options[:file1] and options[:file2] and options[:tol] then
#   for (option, value) in options
#        puts value 
#    end
#else
#    puts opts.to_s
#    Kernel.exit()
#end 

options = {}
opts = OptionParser.new()
#opts.on("-h", "--help", "You're looking at it."){RDoc::usage}
opts.on("-h", "--help", "You're looking at it."){puts opts.to_s;Kernel.exit(0)}
opts.on("-1", "--file1 file1", "First file to compare."){|val| options[:file1] = val}
opts.on("-2", "--file2 file2", "Second file to compare.") {|val| options[:file2] = val}
opts.on("-t", "--tol [tol]", "Number of occurrences to ignore.") do |val| 
    if val.nil? then options[:tol] = 0 else options[:tol] = val.to_i() end
end
opts.parse(ARGV) rescue puts opts.to_s
#need to learn how OptionParser does this the "right" way.
if options[:file1].nil? or options[:file1].nil? then puts opts.to_s;Kernel.exit(1) end

if options[:file1] then
   if File::exists?(options[:file1]) and (File::readable?(options[:file1])) then
        f1_bytes=get_file_contents(options[:file1])
    else 
        puts "The file #{options[:file1]} does not exist or is not readable."
        Kernel.exit(1)
    end 
end

if options[:file2] then
    if File::exists?(options[:file2]) then 
        f2_bytes=get_file_contents(options[:file2])
    else 
        puts "The file #{options[:file2]} does not exist or is not readable."
        Kernel.exit(1)
    end 
end

if isBinary?(options[:file1]) then
    puts "The file #{options[:file1]} is probably binary."
else
    puts "The file #{options[:file1]} is probably text."
end

if isBinary?(options[:file2]) then
    puts "The file #{options[:file2]} is probably binary."
else
    puts "The file #{options[:file2]} is probably text."
end
if isBinary?(options[:file1]) != isBinary?(options[:file2]) then
    puts "One file is text the other is binary."
end
bindiff(options[:file1], options[:file2], options[:tol])
