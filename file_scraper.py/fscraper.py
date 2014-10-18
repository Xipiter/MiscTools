#!/usr/bin/env python
"""
filescraper...to go get a buncha files.
"""
import os
import google
import urllib
from urllib import FancyURLopener
from time import localtime
from urlparse import *
from sys import argv 

global APIKEY 
APIKEY = "nYiW30JQFHL4tmaXW+GxuAH8zwfFwq4j"

def pt(t2print):
    """ print tuple for debugging """
    for x in range(0,len(t2print)):
        print x, t2print[x]

class uriobj:
    url = None
    scheme = None
    netloc = None
    path = None
    file = None
    params = None
    query = None
    fragment = None
    dl_total = 0
    dl_count = 0
    def __init__(self, url):
        uri_t = urlparse(url)
        self.url = url
        self.scheme = uri_t[0]
        self.netloc = uri_t[1]
        self.path = uri_t[2]
        if uri_t[2] == "/":
            self.file = "smeh"
        else:
            tmp_path=uri_t[2].split("/")
            self.file = tmp_path[len(tmp_path)-1] 
        self.params = uri_t[3]
        self.query = uri_t[4]
        self.fragment = uri_t[5]
        self.dl_total = 0
        self.dl_count = 0

    def dl_progress(self, blocknum, bs, size):
        self.dl_total = (size / bs)
        #print self.dl_total
        self.dl_count += 1
        #print self.dl_count
        print ".",

def progresshook(blocknum, bs, size):
    #print blocknum, bs, size
    print ".",
                
def append_date(string):
    """ Append a few numbers from the current time to <string>
    """
    timet = localtime()
    timestr = ""
    for x in timet[:6]:
        timestr += str(x)
    return (string+"."+timestr)

def create_dir(filetype):
    home = os.getenv("HOME")
    scrapedir = home+"/fscrape/"
    dl_dir = scrapedir+append_date(filetype)
    try: 
        os.chdir(scrapedir)
    except OSError:
        os.mkdir(scrapedir)
        os.chdir(scrapedir)
    os.mkdir(dl_dir)
    os.chdir(dl_dir)
    print "Putting downloads in: %s ..." % (dl_dir)
  
def go_get_googles(filetype, results_desired=10):
    search_string = "e filetype:%s" % (filetype)
    if results_desired <= 10:
        batches = 1 #the google api only supports retrieving
                    #10 results per search so we have to batch 
                    #the requests
    if results_desired > 10:
        if ((results_desired % 10) != 0): #if there is a remainder
            batches = (results_desired / 10)+1 #then round up
        else:
            batches = (results_desired / 10)
    urls = []
    for inc in range(0, batches):
        googles = google.doGoogleSearch(search_string, (inc*10), 10)
        rl = len(googles.results)
        for x in range(0,len(googles.results)):
           urls.append(uriobj(googles.results[x].URL))
    #pt(urls)
    print "Doing:", batches, "batches for", len(urls), "files found."
    return urls #returns a list of uriobj's

def write_urls_txt(urls):
    """create urls.txt """
    of = open("urls.log", "w")
    of.write("\n=== URI's TO FILES FOUND ===\n")    
    for x in range(len(urls)):
        #print urls[x].url
        of.write(urls[x].url+"\n")
    of.close()

def go_get_that_shit(filetype):
    create_dir(filetype)
    urls = go_get_googles(filetype)
    write_urls_txt(urls)
    openinst = urllib.URLopener()
    openinst.version = "filescrape.py"
    x = 0
    while (x < len(urls)):
        print "\nDownloading", urls[x].file, 
        #urllib.URLopener.retrieve(openinst, urls[x].url, urls[x].file, urls[x].dl_progress)
        x+=1

def main():
    print "\n\n\n\n\n\n\n\n"
    google.LICENSE_KEY = APIKEY
    go_get_that_shit(argv[1])

if __name__ == "__main__":
    main()
