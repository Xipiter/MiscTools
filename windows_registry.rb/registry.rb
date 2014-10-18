#!/usr/bin/env ruby -W0
# == Windows Registry
#
#   This thing is a basic interface to the windows registry. 
#   It implements a module called RegistryInterface which contains 
#   all the helper methods. 
#   These methods are then wrapped in a cli. If you want you can even drop into an IRB
#   session from this CLI to use these methods directly.
#
# == Author
#   Stephen A. Ridley stephen@matasano.com 2009
#

require 'optparse'
require 'rdoc/usage'
require 'Win32API' #prob'ly not gonna use this
require 'win32/registry'
#require 'ruby-debug'
require 'cmd' #http://files.rubyforge.vm.bytemark.co.uk/cmd/cmd-0.7.2.gem
require 'pp'
require 'irb'
require 'regenum'

module IRB
# http://errtheblog.com/posts/9-drop-to-irb 
    def self.start_session(binding) 
        IRB.setup(nil) 
        workspace = WorkSpace.new(binding) 
        if @CONF[:SCRIPT] 
            irb = Irb.new(workspace, @CONF[:SCRIPT]) 
        else 
            irb = Irb.new(workspace) 
        end 
        @CONF[:IRB_RC].call(irb.context) if @CONF[:IRB_RC] 
        @CONF[:MAIN_CONTEXT] = irb.context 
        trap("SIGINT") do irb.signal_handle end 
        catch(:IRB_EXIT) do irb.eval_input end 
    end 
end 

module RegistryInterface 
    # Search the registry for specific values.
    def self.search(regpath, search_val)
        r_h = Win32::Registry.open_full_path(regpath)
        r_h.each_value do |i| 
            result = i.select{|m| m=~/#{search_val}/}
            if result.length() > 0 then
                puts "-------"
                puts "Match found at: ", r_h.name
                pp result 
                puts "-------"
            else puts "No matches." end
        end
    end

    # Show which root hkey values are supported, by the API, not system
    def self.hkeys
        puts "The supported HKEYs are:"
        roots = Win32::Registry::Constants.constants.select{|i| i =~ /HKEY/}
        pp roots 
    end

    def self.list(regpath)
        r_h = Win32::Registry.open_full_path(regpath)
        r_h.each_value{|i| pp i}
    end
end

class RegUI < Cmd
    @root = nil
    @prompt = "RegUI> "
    doc(:find, "Search the registry for something.")
    doc(:irb, "Drop to an IRB session.")
    doc(:ls, "List current 'branch' of registry, or specify a path to list.")
    doc(:sfs, "Find all Safe-For-Scripting ActiveX modules.")
    doc(:activex, "Find all ActiveX modules registered.")
    doc(:c2p, "Get the ProgID of a CLSID.")
    doc(:p2c, "Get the CLSID of a ProgiD.")
    doc(:checksfs, "Check if an CLSID is marked Safe-For-Scripting.")
    doc(:hkeys, "Display the Predefined HKEY values supported.")
    doc(:cd, "Set the path to work in.\n\t eg: HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes")
    doc(:pwd, "Display the current 'branch' of the registry you are in. ")

    # ----- Cleanup all the broken stuff inherited from Cmd
    undef_method "do_shell" # undef_method deletes the method from the Object!
                            # "shell" was a broken method inherited from Cmd
    @@docs.delete("shell")
    # ------

    def rootset?
        if @root == nil then return false else return true end
    end

    def do_cd(*arg)
#        if Win32::Registry::Constants.constants.include?(arg[0]) then
#            @hkey = arg[0]
#            prompt_with("Regtool:#{@root}> ")
#        else
#            puts "They hkey is not supported, use 'hkeys' command to get a list."
#        end
        if arg.length() < 1 then return end
        @root = arg[0]
        if arg[0].length > 30 then truncroot = arg[0][0..30]+"..."
        else truncroot = arg[0] end
        prompt_with("RegUI:#{truncroot}> ")     
    end

    def do_irb
        puts "\n\nDropping to irb.\nBe sure you use irb_exit or ctrl-d to return to RegUI NOT 'exit'.\n\n"
        IRB.start_session(Kernel.binding)
    end

    def do_hkeys
        RegistryInterface.hkeys() 
    end

    def do_pwd
        if rootset? == false then puts "There is currently no root set." 
        else puts "You are currently operating in:\n #{@root}" end
    end

    def do_find(*args)
        if rootset? == false then puts "Please set a default root with 'cd'.";return
#        else puts "Searching in:\n#{@root}\nFor: '#{args[0]}' ." end
        else puts "Searching in current registry branch for: '#{args[0]}' ." end
        RegistryInterface.search(@root, args[0])
    end 

    def do_ls(*args)
        if rootset? == false then puts "Please set a default root with 'cd'."; return end
        if args[0] == nil then RegistryInterface.list(@root)
        else RegistryInterface.list(args[0]) end
    end

    protected 
    def command_missing(command, args)
    end
end
a = RegUI.new()
#a.run()
a.cmdloop("\n..ooOO Ruby Windows Registry Tool OOOooo...\n")

