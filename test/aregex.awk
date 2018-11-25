@load "aregex"

BEGIN {
    re     = "^abcdefghij$"
    str    = "abcdefghi"
    cost   = 3
	print "RE: '" re "', string: '" str "', cost: " cost ", match dist: " aregex(re, str, 1)
}
