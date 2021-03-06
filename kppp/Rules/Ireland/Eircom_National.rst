################################################################
#
# This is a kppp ruleset for Eircom (formerly Telecom Eireann)
# for standard national calls (NOT special-rate Internet 1891 calls).
#
# Unbelievably, Eircom has now dropped the former (ludicrously
# irrelevant) distance-based charge-bands on direct-dialled calls.
# They still remain for operator-connected calls but these are
# (a) a rarity and (b) unusable for modems anyway. Calls in Ireland
# are therefore in one of the following categories:
#
# 1. Local calls
# 2. Special-rate Internet calls (ISPs with 1891 numbers)
# 3. National calls (ie all other trunk or long-distance calls)
#
# Note that some Telcos offer special deals of a fixed-rate per-month
# charge which gives you unlimited, uncharged local calls in off-peak
# times. At other times, your standard Telco rates apply.
#
# "Local" is as hard to define as in any other Telco administration,
# as it can cross area codes, even when they are in different regions,
# in order to allow people to call their neighbours 100 yards away
# even though they may technically be in an area code which would
# normally qualify as "long-distance", because such calls don't go
# onto the trunk, just the local exchange.
#
# Note all values here include Value-Added Tax at 21% current
# at 31-Dec-1999
#
# Peter Flynn <peter@silmaril.ie>
################################################################

name=Ireland_Eircom_National

# Define IEP (Irish Pounds) to be used as currency symbol
# ??? There is no way to define the currency code AND the symbol !!!
# WARNING this will have to be changed to EUR from 2002-01-01
currency_symbol=�

# Define the position of the currency symbol.
# (not absolutely needed, default is "right")
# ??? Curious default, why not left, which is _way_ more common? !!!
currency_position=left 

# Define the number of significant digits.
# (not absolutely needed, default is "2"
currency_digits=2

# NOTE: rules are applied from top to bottom - the
#       LAST matching rule is the one used for the
#       cost computations.

# It costs 11.5p the moment a call connects. This covers the first
# 69.01 secs (peak hours, 8am-6pm M-F) or first 103.64 secs (evenings)
# or 10 mins (weekends). Yes, they calculate to the 1/100th sec...
per_connection=0.115

# Therefore the minimum cost is the same as the per-connection cost
minimum_costs=0.115

# Therefore the first 69 secs costs this much no matter what.
flat_init_costs=(0.115,69)
# A pity there's no peak/offpeak differential for this one.

# All subsequent charging is done per-second, based on the unit
# charge of 11.5p for 69.01 sec (peak hours) or 11.5p for 103.64 sec
# (evenings) or 11.5p for 600 sec (weekends), which works out at 
# �0.0016664/sec, �0.00110961p/sec, and �0.0001916667p/sec
# respectively...that's what they claim, anyway.

# Rather than expect kppp to check the rate every second and add
# tiny fractions, I've expressed these rates in terms of the amount
# needed to clock up half a penny (or the closest amount exceeding
# that value obtainable by multiplying the per-second rate by an
# integer). Not a whole penny, because you may be damn certain the
# bean-counters will round up half-penny amounts to the nearest
# whole penny anyway (anal-retentive, are we? :-)

# Thus the base rate for peak-time calls is �0.004999 for 3 secs
# (0.115 / 69.01 = 0.0016664251 / 0.005 = 0.3332850 inv = 3.0004)
# evenings is �0.0055481 for 5 secs
# (0.115 / 103.64 = 0.00110961 / 0.005 = 0.2219220 inv = 4.5060869565)
# and weekends is �0.005175 for 27 secs
# (0.115 / 600 = 0.0001916667 / 0.005 = 0.0383333 inv = 26.086956522)
# so accounting should happen in approx 1/2p increments...

# OK, here we go...

# Because of the need to detect time-of-day as well as initial-period, 
# this default should never actually get applied, but we assume that
# connections are made in the peak rate period...
default=(0.004999,3)

# PEAK-TIME CALLS are 8am to 6pm Mon-Fri, so after flat_init_costs
# this rule should apply:
on (monday..friday) between (08:00..18:00) use (0.004999,3,69)

# EVENING CALLS are 6pm to 8am Mon-Fri 
# This needs to supersede the flat_init_costs on time, because that
# only applies to the first 69.01 secs of PEAK-TIME calls
on (monday..friday) between (00:00..08:00) use (0.115,104)
on (monday..friday) between (18:00..23:59) use (0.115,104)
# Thereafter the per-second rate applies after the first 104 secs
on (monday..friday) between (00:00..08:00) use (0.0055481,5,104)
on (monday..friday) between (18:00..23:59) use (0.0055481,5,104)

# WEEKEND CALLS are midnight Friday to midnight Sunday
# This needs to supersede the flat_init_costs on time, because that
# only applies to the first 69.01 secs of PEAK-TIME calls
on (saturday..sunday) between (00:00..23:59) use (0.115,600)
# Thereafter the per-second rate applies after the first 104 secs
on (saturday..sunday) between (00:00..23:59) use (0.005175,27,600)

# KNOWN HOLIDAYS are all at weekend rates

# New Year's Day
on (01/01) between (00:00..23:59) use (0.115,600)
on (01/01) between (00:00..23:59) use (0.005175,27,600)

# St Patrick's Day
on (03/17) between (00:00..23:59) use (0.115,600)
on (03/17) between (00:00..23:59) use (0.005175,27,600)

# Easter Monday
on (easter+1) between (00:00..23:59) use (0.115,600)
on (easter+1) between (00:00..23:59) use (0.005175,27,600)

# May Day (Bealtaine)
on (05/01) between (00:00..23:59) use (0.115,600)
on (05/01) between (00:00..23:59) use (0.005175,27,600)

# Christmas Day and St Stephen's Day
on (12/25) between (00:00..23:59) use (0.115,600)
on (12/25) between (00:00..23:59) use (0.005175,27,600)
on (12/26) between (00:00..23:59) use (0.115,600)
on (12/26) between (00:00..23:59) use (0.005175,27,600)

# This file should be refreshed every year to take account of the
# moveable public holidays we inherited from the British practice,
# known as "Bank Holidays" (originally the quarter-days when banks
# had to close for accounting purposes, but now almost unpredictable).
# These happen several times a year, always on a Monday. Dates 
# for 2000 are June 5th, August 7th, and October 30th.
# The exact dates are known several years in advance and are fixed
# by the Taoiseach's Office and the Dept of Local Government.
# They are NOT the same days as British Bank Holidays, which are
# fixed on a different basis.

# June Bank Holiday 2000 (in lieu of Oimelc/Imbolc, which was in Feb)
on (06/05) between (00:00..23:59) use (0.115,600)
on (06/05) between (00:00..23:59) use (0.005175,27,600)

# August Bank Holiday 2000 (Lughnasa)
on (08/07) between (00:00..23:59) use (0.115,600)
on (08/07) between (00:00..23:59) use (0.005175,27,600)

# October Bank Holiday 2000 (Samhain)
on (10/30) between (00:00..23:59) use (0.115,600)
on (10/30) between (00:00..23:59) use (0.005175,27,600)

# No automatic account is taken of Transference, when a fixed public 
# holiday occurs on a weekend, which means the following Monday becomes
# a holiday in compensation. (1/1/2000 is a good example!!)

# Transfer New Year's Day holiday 2000 to first working day afterwards
on (01/03) between (00:00..23:59) use (0.115,600)
on (01/03) between (00:00..23:59) use (0.005175,27,600)

# None of the other fixed holidays in 2000 needs this doing. 

# When Christmas occurs on a Saturday (and St Stephen's Day therefore
# on a Sunday), ONLY the following Monday is a holiday, not the Tuesday
# as well (sorry, guys :-)
