# Autogenerated Makefile -*- makefile -*-
# Copyright (C) 2009-2010 Ross Biro.
# Do not edit. Do not Distribute.
#
-include make_parts/begin/*.inc

-include make/textus/logging/Logging.inc

-include make_parts/middle/*.inc


-include make_parts/end/*.inc

runtests: runtests_textus_logging_Logging

valgrind: valgrind_textus_logging_Logging

