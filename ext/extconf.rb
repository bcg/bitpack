#!/usr/bin/env ruby -Ks
 
require "mkmf"

$CFLAGS << ' -W -Wall'

create_makefile("bitpack")

