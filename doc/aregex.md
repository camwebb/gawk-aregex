% AREGEX(3am) Free Software Foundation | GNU Awk Extension Modules
%
% Nov 24 2018

# NAME

aregex - approximate (fuzzy) string matching with regular expressions

# SYNOPSIS

@load `"aregex"`

success = amatch(str, regex [, cost|costs [, submatches]])

# DESCRIPTION

The TRE library (ref. below) provides approximate matching regex
capabilities. A match between two strings that differ in some number
of characters will be found when the cost of character insertions,
deletions and substitutions does not exceed some specified maximum
cost. For example,

```
"abcdef"
"abcxdef"  # one insertion
"abdef"    # one deletion
"abxdef"   # one substitution
```

The cost of the match (the Levenshtein distance between strings)
can be reported. This Gawk extension provides an interface with the
_tre\_regaexec()_ function in the TRE library, permitting the setting
of all possible parameters for that function, and returning all
possible information about a match.

## Function summary

A single function, **amatch()** is
provided, modeled on the Gawk _match()_ function:

**amatch(** _str_ **,** _regex_ **\[,** _cost_|_costs_ **\[,** _submatches_ **\]** **\]** **)**

This function takes two mandatory string arguments, and two optional
arguments. _regex_ is an **extended** regular expression (or plain
string) to be matched against string _str_. Note that the regular
expression _regex_ is bounded by double-quotes, not by the usual Gawk
slashes.

## Setting approximate match costs

With only two arguments, the default maximum cost for the approximate
match is set to 5 (and other costs are set as below). The maximum cost
may also be set by the user via an optional **third argument**: either an
integer (_cost_), or a member of a one-dimensional array (_costs_)
indexed by `"max_cost"`. Setting maximum cost to 0 forces an exact
regular expression match, as with Gawk’s _match()_.  Other members of
the _costs_ array with appropriate index values will set the parameters
of the _regaparams\_t_ structure used by _tre\_regaexec()_:

```
Array index   Parameter    Def val  Meaning
============  ===========  =======  =====================
"cost_ins"    .cost_ins       1     Cost of one insertion
"cost_del"    .cost_del       1     Cost of one deletion
"cost_subst"  .cost_subst     1     Cost of one substitution
"max_cost"    .max_cost       5     Max. cost
"max_del"     .max_del        5     Max. number of deletions
"max_ins"     .max_ins        5     Max. number of insertions
"max_subst"   .max_subst      5     Max. number of substitutions
"max_err"     .max_err        5     Max. number of ins+del+subst
```

If the array _costs_ is provided but contains none of the above
indexes, the default values are used.

## Return value

The **amatch()** function returns 1 on a successful
match, 0 on a failure to match and -1 if _regex_ is invalid
(with TRE's error message in _ERRNO_) .

## Obtaining match summary data

If a **third array argument** is provided to **amatch()**, and a match was
successful, information about the match is returned via (clearing and)
filling members of the _costs_ array with these indexes:

```
Array index   Meaning
============  ==============================================
"cost"        Total cost of the match (Levenshtein distance)
"num_ins"     Total number of insertions
"num_del"     Total number of deletions
"num_subst"   Total number of substitutions
```

## Obtaining parenthetical submatches

If an array (or empty Gawk variable symbol) is provided as the
**fourth argument** , and a match is successful, the array will be
cleared and filled with submatches corresponding to the parenthetical
sub-expression in _regex_, with indexes _1...n_, up to a maximum of
20. The array member indexed by _0_ will be the portion of _str_
matched by the whole of _regex_.

**A note on bytes and characters**: While the **amatch()** function is
roughly equivalent to the Gawk _match()_ function, submatches are not
returned as in _match()_, e.g. via _[i,"start"]_ position and _[i,"length"]_
(see Gawk `man` page). Instead only the literal substring for each
parenthetical match is given. Gawk is multibyte aware, and _match()_
works in terms of characters, not bytes, but TRE is byte-based, not
character-based. Using the _wchar\_t_ versions of TRE functions cannot
help if the input is a mix of single and multi-byte characters.  A
simple Gawk routine must be used on the output array (_submatches_), if
positions and lengths of the substrings are needed. E.g.:

```
print "i", "substring", "posn", "length"
p = 1
for (i = 1; i < length(submatches); i++) {
  idx = index(substr(str, p), submatches[i])
  len = length(out[i])
  print i, submatches[i], idx+p-1, len
  p = p + idx + len
}
```

# EXAMPLE

```
@load "aregex"
BEGIN {
  str = "abcdễfbc"
  regex = "^a(bc)d(ễ)(f)$"
  costs["max_cost"] = 6
  costs["cost_ins"] = 2
  if (amatch(str, regex, costs, submatches)>0)
    print costs["cost"], submatches[1]
}
```

# SEE ALSO

The Gawk extension lib: https://sourceforge.net/projects/gawkextlib/
and TRE library: https://laurikari.net/tre/

# AUTHORS

Cam Webb <cw@camwebb.info>, @laurikari for the TRE library, the
_gawkextlib_ authors

# COPYING PERMISSIONS

Copyright © 2018, the Free Software Foundation, Inc.

Copyright © 2018, Campbell O. Webb

Permission is granted to make and distribute verbatim  copies  of  this
manual  page  provided  the copyright notice and this permission notice
are preserved on all copies.

Permission is granted to copy and distribute modified versions of  this
manual  page  under  the conditions for verbatim copying, provided that
the entire resulting derived work is distributed under the terms  of  a
permission notice identical to this one.

Permission  is granted to copy and distribute translations of this manual page into another language, under the above conditions for modified
versions,  except that this permission notice may be stated in a trans‐
lation approved by the Foundation.

