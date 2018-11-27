/*
 * amatch.c - Gawk extension to access the TRE approximate regex.
 * Copyright (C) 2018 Cam Webb, <cw@camwebb.info>
 * Distributed under the GNU Pulbic Licence v3
 */

// Minimal headers:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <gawkapi.h>
#include <tre/tre.h>

#define NSUBMATCH 10

// Gawkextlib boilerplate:
static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
int plugin_is_GPL_compatible;

// Main amatch() function definition
static awk_value_t * do_amatch(int nargs, awk_value_t *result, \
                            struct awk_ext_func *unused)
{
  // Variables for reading awk function's arguments
  awk_value_t re;
  awk_value_t str;
  awk_value_t costs;
  awk_value_t substr;
  
  // Read 3rd argument
  if (!get_argument(2, AWK_ARRAY, &costs)) {
    fatal(ext_id, "amatch: 3rd argument must be present, and an array");
  }

  // Read parameters for tre_regaexec()
  awk_value_t val;
  awk_value_t index;
  int i;
  char c[30];
  const char *parami[8];
  int paramv[8];
  parami[0] = "cost_ins";   paramv[0] = 1;
  parami[1] = "cost_del";   paramv[1] = 1;
  parami[2] = "cost_subst"; paramv[2] = 1;
  parami[3] = "max_cost";   paramv[3] = 3;
  parami[4] = "max_del";    paramv[4] = 3;
  parami[5] = "max_ins";    paramv[5] = 3;
  parami[6] = "max_subst";  paramv[6] = 3;
  parami[7] = "max_err";    paramv[7] = 3;
  for (i = 0; i < 8; i++) {
    make_const_string(parami[i], strlen(parami[i]), &index);
    if (get_array_element(costs.array_cookie, &index, AWK_STRING, &val)) {
      paramv[i] = atoi(val.str_value.str);
      strcpy(c,"") ;
      sprintf(c, "%s = %d", parami[i], atoi(val.str_value.str));
      warning(ext_id, c);
    }
  }

  // If the string arguments (1st and 2nd) are read. Q: die, or return NULL?
  if (!get_argument(0, AWK_STRING, &re))
    fatal(ext_id, "amatch: first parameter not found; must be a string");
  if (!get_argument(1, AWK_STRING, &str))
    fatal(ext_id, "amatch: second parameter not found; must be a string");
  
  // Compile regex
  regex_t preg;
  tre_regcomp(&preg, re.str_value.str, REG_EXTENDED);
    
  // Set approx amatch params
  regaparams_t params = { 0 };
  params.cost_ins   = paramv[0]; 
  params.cost_del   = paramv[1];
  params.cost_subst = paramv[2];
  params.max_cost   = paramv[3];
  params.max_del    = paramv[4];
  params.max_ins    = paramv[5];
  params.max_subst  = paramv[6];
  params.max_err    = paramv[7];

  // Create structure for details of match
  regamatch_t match ;
  match.nmatch = NSUBMATCH; // No partial match arrays needed
  match.pmatch = (regmatch_t *) malloc(NSUBMATCH * sizeof(regmatch_t));
  // match.pmatch; //   - ditto -
  //regmatch_t match.pmatch[4];
  
  // Return values
  int treret = 0;
  int rval = 0;

  // Do the approx regexp
  treret = tre_regaexec(&preg, str.str_value.str, &match, params, 0);
  char c2[20];
  sprintf(c2, "offset: %d", match.pmatch[2].rm_so); 
  warning(ext_id, c2);
  
  // Set the do_amatch() return value depending on tre_regaexec() return:
  // Return cost (Levenshtein distance) if success, -1 if no match,
  if (treret == REG_NOMATCH) rval = -1;
  else rval = match.cost;
  // Catch a "mem. not. allocated" return from tre_regaexec()
  if (treret == REG_ESPACE) {
    warning(ext_id,                                                     \
            "amatch: TRE err., mem. insufficient to complete the match.");
    return make_null_string(result);
  }

  
  // Hand the substrings over to the substring array
  // Read the SUBSEP symbol
  awk_value_t subsep;
  if (!sym_lookup("SUBSEP", AWK_STRING, &subsep))
    warning(ext_id, "amatch: Could not get SUBSEP from gawk.");

  // Read 4th argument
  if (!get_argument(3, AWK_ARRAY, &substr)) {
    warning(ext_id, "amatch: 4th argument not present or not an array");
  }
  else clear_array(substr.array_cookie);
  
  char outindex1[20];
  char outval1[20];
  awk_value_t outindex;
  awk_value_t outval;
  for (i = 1 ; i < match.nmatch; i++) {
    if (match.pmatch[i].rm_so > 0) {
      sprintf(outindex1, "%d", i);
      sprintf(outval1, "%d %.*s", match.pmatch[i].rm_so+1, match.pmatch[i].rm_eo - match.pmatch[i].rm_so, str.str_value.str + match.pmatch[i].rm_so);
      set_array_element(substr.array_cookie,                            \
                        make_const_string(outindex1, strlen(outindex1), &outindex), \
                        make_const_string(outval1, strlen(outval1), &outval));
    }
  }
  return make_number(rval, result);
}


// Gawkextlib boilerplate:

static awk_ext_func_t func_table[] = \
    {
     { "amatch", do_amatch, 3, 2, awk_false, NULL  },
    };

static awk_bool_t (*init_func)(void) = NULL;

static const char *ext_version = "0.1";

dl_load_func(func_table, amatch, "")

