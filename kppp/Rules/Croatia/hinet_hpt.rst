################################################################
#
# NAME OF THE RULESET. This is NEEDED for accounting purposes.
#
################################################################
name=hinet_hpt.rst

################################################################
# currency settings
################################################################

currency_symbol=KN
currency_position=right
currency_digits=2

################################################################
# connection settings
################################################################

per_connection=0.0
minimum_costs=0.0

# You pay .74 for the first 180 secons ( 3minutes) no matter
# whether you are connected for 1 second or 180 seconds.
# This rule will take priority during the first 180 seconds
# over any other rule, in particular the 'default' rule.
# have a look at costgraphs.gif in the docs directory
# of the kppp distribution for a graphic illustration.

flat_init_costs=(0.2684,180)

# This is the default rule which is used when no other rule
# applies. The first component "0.1" is the price of one
# "unit", while "72" is the duration in seconds.
# Therefore the following rule means: "Every 72 seconds 0.1
# ATS are added to the bill"
default=(0.2684, 180)

## more complicated rules:
# "on monday until sunday from 12:00 am until 11:59 pm the costs
# are 0.2 each 72 seconds"
#on () between () use (0.2, 2)

# same as above. You must use 24 hour notation, or the accounting
# will not work correctly. (Example: write 15:00 for 3 pm)
#on (monday..sunday) between (0:00..23:59) use (0.2684, 2)

#############################################
# 7-16 1 impuls svakih 180 sekundi
# 16-22 1 impuls svakih 240 sekundi
# 22-7 1 impuls svakih 360 sekundi
#############################################

on (monday..saturday) between (0:00..6:59) use (0.2684, 360)
on (monday..saturday) between (7:00..15:59) use (0.2684, 180)
on (monday..saturday) between (16:00..21:59) use (0.2684, 240)
on (monday..saturday) between (22:00..23:59) use (0.2684, 360)

# praznicima i nedjeljom 360 sek impuls
on (sunday) between () use (0.2684, 360)
on (01/01, easter, 05/30, 08/04, 08/15, 12/25) between () use (0.2684, 360)
