#!/usr/bin/env ruby
#
# == WorkFocus
#   This thing should be put in your execute path ($PATH or whatever).
#   When you run it it sets an environment variable called "WORK" to your
#   current directory and modifies your .profile to automatically chdir() to
#   this directory when bash starts. This way you can "bookmark" directories
#   and remember where it is you were working last. This is particularly useful
#   when developing deep in file shares and SVN/GIT checkouts with large paths.
#   For example, my login looks like this:
#
#   Last login: Mon Nov  2 17:17:18 on ttys005
#   You have mail.
#   
#   *********WORK FOCUS********
#   /data/CHECKOUTS/github/
#   *****************************
#   
#   navi-two:github s7ephen$ 

#      
# == Author
#   Stephen A. Ridley
#       stephen@sa7ori.org
#
#--
#   ATTENTION ALL YE HIGH-AND-MIGHTY RUBY STYLE CRITICS:
#   Let me put this in the language you love so much:
#       if the_code.does_it_work?: You::stfu() end
#++
require 'time'

def new_profile(fname,data)
    if !File::writable?(fname) then
        puts "File: #{fname} is not writeable!"
        Kernel.exit(1)
    end
    f_h = File.new(fname,'w')
    f_h.write(data)
    f_h.close()
end

bashrc = ENV['HOME']+'/.profile'
pwd = Dir::pwd
if (!File::exists?(bashrc) & File::readable?(bashrc)) then
    puts "[-] Error: #{bashrc} doesn't seem to exist or is not readable."
    Kernel.exit(1)    
end
f_h = File.new(bashrc,'r+')
fbytes = f_h.read()
f_h.close()
block = <<BLOCK
#<WORKFOCUS>
# Added by setwork.rb on: #{Time.new().asctime}
# Please do not edit this by hand!
# 
export WORK=#{pwd}
echo -ne \'\\n*********WORK FOCUS********\\n\'
echo -ne \'#{pwd}\\n\'
echo -ne \'*****************************\\n\\n\\n\'
cd #{pwd}
#<WORKFOCUS>
BLOCK
if fbytes.index("#<WORKFOCUS>") != nil then
    fb = fbytes.split("#<WORKFOCUS>");fb.delete("")
    if fb.length > 3 then
        puts "[-] Found multiple occurances of <WORKFOCUS> tags. Please fix."
        Kernel.exit(1)
    else
        fb.delete_at(1)
        new_profile(bashrc, fb.join().chop()+block)
        puts "\n\n*** $WORK focus set to: ***\n#{pwd}\n\n"
        #puts fb.join()+block
    end
else
    puts "[+] WORKFOCUS block not found in .profile, creating it."
    new_profile(bashrc, fbytes.chop()+block)
    puts "\n\n*** $WORK focus set to: ***\n#{pwd}\n\n"
    #puts fbytes+block
end


