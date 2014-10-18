#!/usr/bin/env ruby
require 'optparse'
require 'rdoc/usage'
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
#opts.on("-h", "--help", "You're looking at it."){puts opts.to_s;Kernel.exit(0)}
opts.on("-h", "--host hostname", "The hostname of the server."){|val| options[:host] = val}
opts.on("-s", "--ssl ", "Check SSL also."){|val| options[:ssl] = True}
opts.parse(ARGV) rescue puts opts.to_s
#need to learn how OptionParser does this the "right" way.
if options[:host].nil? then puts opts.to_s;Kernel.exit(1) end

if options[:file1] then
    check_webdav(options[:host])
end

def check_webdav(hostname)
    xmlreq <<EOF
<?xml version="1.0" encoding="utf-8"?>
<propfind xmlns="DAV:">
<prop>
<getcontentlength xmlns="DAV:"/>
<getlastmodified xmlns="DAV:"/>
<executable xmlns="http://apache.org/dav/props/"/>
<resourcetype xmlns="DAV:"/>
<checked-in xmlns="DAV:"/>
<checked-out xmlns="DAV:"/>
</prop>
</propfind>
EOF
    Net::HTTP::Propfind.start(hostname) {|http|
    }
end

def get_headers(hostname)
    Net::HTTP.start(hostname) {|http|
        res = http.head("/index.html")
        res.each_header {|key,val| pp "#{key} ==> #{val}"}
    }   
    return #return nothing so irb doesnt print anything
end 

def server_type(hostname)
    IRB.start_session(Kernel.binding) #Invoke, passing in our binding.
    Net::HTTP.start(hostname) {|http|
        res = http.head("/index.html")
        puts "The webserver on: #{hostname} is: #{res["server"]}."
    }   
    return #return nothing so irb doesnt print anything
end

 

