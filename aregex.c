/*
 * aregex.c - Gawk extension to access the TRE approximate regex.
 * Copyright (C) 2018 Cam Webb, <cw@camwebb.info>
 * Distributed under the GNU Pulbic Licence v3
 */

// Minimal headers:
#include <stdio.h>
#include <sys/stat.h>
#include <gawkapi.h>
#include <tre/tre.h>

// Gawkextlib boilerplate:
static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
int plugin_is_GPL_compatible;

// Main aregex() function definition
static awk_value_t * do_aregex(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
  // Variables for reading awk function's arguments
  awk_value_t re;
  awk_value_t str;
  awk_value_t incost;

  // Check args if gawk called with lint 
  if (do_lint) {
    if (nargs > 3)
      lintwarn(ext_id, "aregex() called with >3 arguments");
  }

  // Set the default max cost, and test for 3rd argument
  int defcost = 2;
  if (get_argument(2, AWK_NUMBER, &incost)) defcost = incost.num_value ; 

  // If the string arguments (1st and 2nd) are read...
  if ((get_argument(0, AWK_STRING, &re)) &&     \
      (get_argument(1, AWK_STRING, &str))) {
    
    // Compile regex
    regex_t preg;
    tre_regcomp(&preg, re.str_value.str, REG_EXTENDED);

    // Set approx aregex params
    regaparams_t params = { 0 };
    params.cost_ins   = 1;
    params.cost_del   = 1;
    params.cost_subst = 1;
    params.max_cost   = defcost;
    params.max_del    = defcost;
    params.max_ins    = defcost;
    params.max_subst  = defcost;
    params.max_err    = defcost;

    // Create structure for details of match
    regamatch_t match;
    match.nmatch = 0; // No partial match arrays needed
    match.pmatch = 0; //   - ditto -

    // Return values
    int treret = 0;
    int rval = 0;

    // Do the approx regexp
    treret = tre_regaexec(&preg, str.str_value.str, &match, params, 0);

    // Set the do_aregex() return value depending on tre_regaexec() return:
    // Return cost (Levenshtein distance) if success, -1 if no match,
    //   -999 if memory error
    if (treret == REG_NOMATCH) rval = -1;
    else if (treret == REG_ESPACE) {
      rval = -999;
      warning(ext_id, \
        "aregex: TRE err.: mem. insufficient to complete the match.");
      // TO DO?: update_ERRNO_int(). But which ERRNO?
    }
    else rval = match.cost;
    
    return make_number(rval, result);
  }
  
  // On string argument failure:
  return make_null_string(result);
}


// Gawkextlib boilerplate:

static awk_ext_func_t func_table[] = \
    {
     { "aregex", do_aregex, 3, 2, awk_false, NULL  },
    };

static awk_bool_t (*init_func)(void) = NULL;

static const char *ext_version = "0.1";

dl_load_func(func_table, aregex, "")

