################################################################
# NOTES:
# You can check for changes in prices at http://www.ote.gr/
# These rules apply to you if you are accessing an ISP without using an EPAK or PEAK
# number (0965-) in the same city as you, from a digital phone.
# The costs for a digital local phone-call are:
# 1 Unit (10.5 drs) every 1 minute (60 secs).
# Exception: 22:00 - 08:00 :1 Unit every 2 minutes (120secs).
################################################################
# Dimitris Kamenopoulos el97146@mail.ntua.gr
################################################################
# NAME OF THE RULESET. This is NEEDED for accounting purposes.
################################################################
name=Hellas_digital_local_in_drs.rts

################################################################
# currency settings
################################################################

# defines DRS (Hellenic draxmi) to be used as currency
# symbol (not absolutely needed, default = "$")
currency_symbol=drs

# Define the position of the currency symbol.
# (not absolutely needed, default is "right")
currency_position=right

# Define the number of significat digits.
# (not absolutely needed, default is "2"
currency_digits=1

################################################################
# connection settings
################################################################

# NOTE: rules are applied from top to bottom - the
#       LAST matching rule is the one used for the
#       cost computations.


default=(10.5, 60)

on (monday..sunday) between (22:00..08:00) use (10.5, 120)
