/*
 * aregex.c - Gawk extension to access the TRE approximate regex.
 * Copyright (C) 2018 Cam Webb
 * Distributed under the Gnu Pulbic Licence v3
 */

#include "common.h"
#include <tre/tre.h>

static awk_value_t * do_aregex(int nargs, awk_value_t *result API_FINFO_ARG)
{
  awk_value_t re;
  awk_value_t str;
  awk_value_t incost;

  // Check args if gawk called with lint 
  if (do_lint) {
    if (nargs > 3)
      lintwarn(ext_id, _("aregex() called with >3 arguments"));
  }

  // set the cost
  int defcost = 2;
  if (get_argument(2, AWK_NUMBER, &incost)) defcost = incost.num_value ; 

  // If the string arguments are read
  if ((get_argument(0, AWK_STRING, &re)) &&     \
      (get_argument(1, AWK_STRING, &str))) {
    
    // Compile RE
    regex_t preg;
    tre_regcomp(&preg, re.str_value.str, REG_EXTENDED);
    
    regaparams_t params = { 0 };
    params.cost_ins   = 1;
    params.cost_del   = 1;
    params.cost_subst = 1;
    params.max_cost   = defcost;
    params.max_del    = defcost;
    params.max_ins    = defcost;
    params.max_subst  = defcost;
    params.max_err    = defcost;
  
    regamatch_t match;
    match.nmatch = 0;
    match.pmatch = 0;
    
    int treret = 0;
    int rval = 0;
    
    treret = tre_regaexec(&preg, str.str_value.str, &match, params, 0);

    if (treret == REG_NOMATCH) rval = -1;
    else if (treret == REG_ESPACE) {
      rval = -999;
      warning(ext_id, _("aregex: TRE error: temporary memory could not be allocated\n      to complete the matching operation."));
    }
    else rval = match.cost;

    return make_number(rval, result);
  }
  
  // On string argument failure:
  return make_null_string(result);
}

// Boilerplate:

static awk_ext_func_t func_table[] = \
    {
     { "aregex", do_aregex, 3, 2, awk_false, NULL  },
    };

static awk_bool_t init_my_module(void)
{
  GAWKEXTLIB_COMMON_INIT
  return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, fmatch, "")
