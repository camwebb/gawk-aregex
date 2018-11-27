@load "./aregex.so"

BEGIN {
  str    = "abcdefghi"
  cost   = 2

  re     = "^bcdefghij$"
  print "RE1: '" re "', string: '" str "', cost: " cost ", match dist: " \
    aregex(re, str, cost)
  # RE1: '^bcdefghij$', string: 'abcdefghi', cost: 2, match dist: 2
  
  re     = "^xcdefghij$"
  print "RE2: '" re "', string: '" str "', cost: " cost ", match dist: " \
    aregex(re, str, cost)
  # RE2: '^xcdefghij$', string: 'abcdefghi', cost: 2, match dist: -1

  # generate warning with gawk --lint :
  print "RE3: '" re "', string: '" str "', cost: " cost ", match dist: " \
    aregex(re, str, cost, "extra1", "extra2")
}
