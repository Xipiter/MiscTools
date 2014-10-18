#!/usr/bin/env ruby
# == Author 
#   Eric Monti eric@matasano.com Oct 2009
#
require 'Win32API'
require 'win32/registry'
require 'enumerator'

class Win32::Registry

  # This module gives RegValues and Registry a common set of methods
  # for reverse registry branch traversal via their parent objects.
  module WalksBackwards
    # Returns an Enumerator for recursing upwards through the parents of a 
    # registry key (self)
    def recurse_back
      self.to_enum(:each_recurse_back)
    end

    # Recurses backwards in the key path via parents. 
    # yields self followed by each key in the parent trail to a block
    def each_recurse_back
      yield(key=self)
      while key=key.parent
        yield(key)
      end
    end

    # Returns the full path string for the registry key/value
    def full_path
      recurse_back.map{|k| k.keyname }.reverse.join('\\')
    end

  end

  include WalksBackwards

  class RegValue < Struct.new(:parent, :keyname, :disposition, :value)
    include WalksBackwards
    def reg_type
      ::Win32::Registry::DISPOSITION_TYPES[disposition]
    end
  end

  ROOT_KEYS = {
    :HKLM   => HKEY_LOCAL_MACHINE,
    :HKEY_LOCAL_MACHINE   => HKEY_LOCAL_MACHINE,
    :LOCAL_MACHINE   => HKEY_LOCAL_MACHINE,

    :HKCU => HKEY_CURRENT_USER,
    :HKEY_CURRENT_USER => HKEY_CURRENT_USER,
    :CURRENT_USER => HKEY_CURRENT_USER,

    :HKCR => HKEY_CLASSES_ROOT,
    :HKEY_CLASSES_ROOT => HKEY_CLASSES_ROOT,
    :CLASSES_ROOT => HKEY_CLASSES_ROOT,

    :HKU => HKEY_USERS,
    :HKEY_USERS => HKEY_USERS,
    :USERS => HKEY_USERS,

    :HKCC => HKEY_CURRENT_CONFIG,
    :HKEY_CURRENT_CONFIG => HKEY_CURRENT_CONFIG,
    :CURRENT_CONFIG => HKEY_CURRENT_CONFIG,

    :HKPD => HKEY_PERFORMANCE_DATA,
    :HKEY_PERFORMANCE_DATA => HKEY_PERFORMANCE_DATA,
    :PERFORMANCE_DATA => HKEY_PERFORMANCE_DATA,

    :HKPT => HKEY_PERFORMANCE_TEXT,
    :HKEY_PERFORMANCE_TEXT => HKEY_PERFORMANCE_TEXT,
    :PERFORMANCE_TEXT => HKEY_PERFORMANCE_TEXT,
    
    :HKPNT => HKEY_PERFORMANCE_NLSTEXT,
    :HKEY_PERFORMANCE_NLSTEXT => HKEY_PERFORMANCE_NLSTEXT,
    :PERFORMANCE_NLSTEXT => HKEY_PERFORMANCE_NLSTEXT,

    :HKDD => HKEY_DYN_DATA,
    :HKEY_DYN_DATA => HKEY_DYN_DATA,
    :DYN_DATA => HKEY_DYN_DATA,
  }

  DISPOSITION_TYPES = [
    :REG_NONE,
    :REG_SZ,
    :REG_EXPAND_SZ,
    :REG_BINARY,
    :REG_DWORD,
    :REG_DWORD_BIG_ENDIAN,
    :REG_LINK,
    :REG_MULTI_SZ,
    :REG_RESOURCE_LIST,
    :REG_FULL_RESOURCE_DESCRIPTOR,
    :REG_RESOURCE_REQUIREMENTS_LIST,
    :REG_QWORD
  ]

  # convenience method - just like 'open' but hkey can be a symbol or string
  def self.alt_open hkey, subkey, desired=KEY_READ, o=REG_OPTION_RESERVED, &blk
    hk = case 
         when hkey.respond_to?(:to_sym)
           raise(Win32::Registry::Error, 
                 "bad root key: #{hkey}") unless rk=ROOT_KEYS[hkey.to_sym]
           rk
         when hkey.kind_of?(::Win32::Registry)
           shkey
         else
           raise(Win32::Registry::Error, "unknown hkey class: #{hkey.class}")
         end
    open(hk, subkey, desired, o, &blk)
  end


  # convenience method - lets you open a key by it's full path string
  def self.open_full_path(path, desired=KEY_READ, opt=REG_OPTION_RESERVED, &blk)
    hk, subk = path.split('\\', 2)
    alt_open(hk, subk, desired, opt, &blk)
  end


  attr_reader :recursing_keys


  # Iterates the subkeys directly under this registry key
  def each_subkey(&block)
    each_key do |subkey, wtime|
      child = ::Win32::Registry.open(self, subkey)
      block.call(child, wtime)
    end
  end

  # Returns an Enumerator for the subkeys directly under this key
  def subkeys(&block)
    self.to_enum(:each_subkey)
  end

  # Recursively iterates all RegValue and subkey under this registry key
  def each_recurse_all(&block)
    @recursing_keys = true
    each_reg_value {|reg| block.call(reg) }
    each_subkey do |child, wtime| 
      block.call(child)
      if child.respond_to? :each_recurse_all and not child.recursing_keys
        child.each_recurse_all &block
      end
    end
    @recursing_keys = false
  end

  # Returns an Enumerator for all RegValue and subkeys under this registry key
  def all_recursed
    self.to_enum(:each_recurse_all)
  end

  # iterates over all child values in this key encapsulated as RegValue 
  # instances
  def each_reg_value
    each_value {|*v| yield(RegValue.new(self, *v)) }
  end

  # returns an enumerator for all child values on this key encapsulated as 
  # RegValue instances.
  def reg_values
    self.to_enum(:each_reg_value)
  end

  # returns an enumerator for all child values on this key
  def values
    self.to_enum(:each_value)
  end

  # returns the registry key's default value or nil if there isn't one
  def value
    (df = reg_values.to_a.first and df.keyname == "" )? df : nil
  end


end

