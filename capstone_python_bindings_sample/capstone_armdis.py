#!/usr/bin/env python
from capstone import *
import binascii

def clean_pasted_code(code):
    space_count = code.count(" ")
    ccode = code.strip(" ")
    for byte in ccode:
        if len(byte) != 2:
            print("'%s' didn't look like a byte, deleting and continuing.") % (byte)
             
            
def disasm_thumb(code):
	try:
	    md = Cs(CS_ARCH_ARM, CS_MODE_THUMB)
            md.syntax = 0
	    for insn in md.disasm(code, 0):
		    #bytes = binascii.hexlify(insn.bytes)
		    #print("0x%x:\t%s\t%s\t// hex-code: %s" %(insn.address, insn.mnemonic, insn.op_str, bytes))
		    #import pdb;pdb.set_trace()
		    print("0x%x:\t%s\t%s" %(insn.address, insn.mnemonic, insn.op_str))
	    print("0x%x:" % (insn.address + insn.size))
	    print
	except CsError as e:
	    print("ERROR: %s" %e)

def disasm_thumb2(code):
	try:
	    md = Cs(CS_ARCH_ARM, CS_MODE_THUMB) 
            md.syntax = 0
	    for insn in md.disasm(code, 0x1000):
		    #bytes = binascii.hexlify(insn.bytes)
		    #print("0x%x:\t%s\t%s\t// hex-code: %s" %(insn.address, insn.mnemonic, insn.op_str, bytes))
		    #import pdb;pdb.set_trace()
		    print("0x%x:\t%s\t%s" %(insn.address, insn.mnemonic, insn.op_str))
	    print("0x%x:" % (insn.address + insn.size))
	    print
	except CsError as e:
	    print("ERROR: %s" %e)

def disasm_arm(code):
	try:
	    md = Cs(CS_ARCH_ARM, CS_MODE_ARM)
            md.syntax = 0
	    for insn in md.disasm(code, 0x1000):
		    #bytes = binascii.hexlify(insn.bytes)
		    #print("0x%x:\t%s\t%s\t// hex-code: %s" %(insn.address, insn.mnemonic, insn.op_str, bytes))
		    #import pdb;pdb.set_trace()
		    print("0x%x:\t%s\t%s" %(insn.address, insn.mnemonic, insn.op_str))
	    print("0x%x:" % (insn.address + insn.size))
	    print
	except CsError as e:
	    print("ERROR: %s" %e)
