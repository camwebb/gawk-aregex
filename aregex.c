/*
 * aregex.c - Gawk extension to access the TRE approximate regex.
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
#define DEFMAXCOST 3
#define DEBUG 1

// Gawkextlib boilerplate:
static const gawk_api_t *api;
static awk_ext_id_t ext_id;
int plugin_is_GPL_compatible;

// Main amatch() function definition
static awk_value_t * do_amatch(int nargs, awk_value_t *result, \
                            struct awk_ext_func *unused)
{
  // Variables for reading awk function's arguments
  awk_value_t substr;
  // Counter:
  int i;
  
  // 1. Set default costs
  const char *parami[8];
  int paramv[8];
  parami[0] = "cost_ins";   paramv[0] = 1;
  parami[1] = "cost_del";   paramv[1] = 1;
  parami[2] = "cost_subst"; paramv[2] = 1;
  parami[3] = "max_cost";   paramv[3] = DEFMAXCOST;
  parami[4] = "max_del";    paramv[4] = DEFMAXCOST;
  parami[5] = "max_ins";    paramv[5] = DEFMAXCOST;
  parami[6] = "max_subst";  paramv[6] = DEFMAXCOST;
  parami[7] = "max_err";    paramv[7] = DEFMAXCOST;
  
  // 2. Read 3rd, 'costs' argument, if present
  if (nargs > 2) {
    awk_value_t costs;
    if (!get_argument(2, AWK_ARRAY, &costs))
      fatal(ext_id, "amatch: 3rd argument present, but could not be read.");
    
    awk_value_t costval;
    awk_value_t costindex;
    char c[30];
    for (i = 0; i < 8; i++) {
      // Create an index for reading array
      make_const_string(parami[i], strlen(parami[i]), &costindex);
      // if there is an array element with that index
      if (get_array_element(costs.array_cookie, &costindex, \
                            AWK_STRING, &costval)) {
        // update the cost value
        paramv[i] = atoi(costval.str_value.str);
        if (DEBUG) {
          strcpy(c,"") ;
          sprintf(c, "cost %s = %d", parami[i], atoi(costval.str_value.str));
          warning(ext_id, c);
        }
      }
    }
  }

  // 3. Read the string and regex arguments (1st and 2nd)
  awk_value_t re;
  awk_value_t str;
  if (!get_argument(0, AWK_STRING, &str))
    fatal(ext_id, "amatch: 1st param., the string, not found");
  if (!get_argument(1, AWK_STRING, &re))
    fatal(ext_id, "amatch: 2nd param., the regex, not found");
  
  // 4. Compile regex
  regex_t preg;
  tre_regcomp(&preg, re.str_value.str, REG_EXTENDED);

  // 5. Do the match
  // 5a. Set approx amatch params
  regaparams_t params = { 0 };
  params.cost_ins   = paramv[0]; 
  params.cost_del   = paramv[1];
  params.cost_subst = paramv[2];
  params.max_cost   = paramv[3];
  params.max_del    = paramv[4];
  params.max_ins    = paramv[5];
  params.max_subst  = paramv[6];
  params.max_err    = paramv[7];

  // 5b. Create necessary tre_ structure for details of match
  regamatch_t match ;
  match.nmatch = NSUBMATCH; 
  match.pmatch = (regmatch_t *) malloc(NSUBMATCH * sizeof(regmatch_t));
  
  // 5c. Do the approx regexp
  int treret;
  treret = tre_regaexec(&preg, str.str_value.str, &match, params, 0);
  
  // 5d. Set the amatch() return value depending on tre_regaexec() return:
  // Return cost (Levenshtein distance) if success, -1 if no match,
  int rval;
  if (treret == REG_NOMATCH) rval = -1;
  else rval = match.cost;

  // Catch a "mem. not. allocated" return from tre_regaexec()
  if (treret == REG_ESPACE) {
    warning(ext_id,                                                     \
            "amatch: TRE err., mem. insufficient to complete the match.");
    return make_null_string(result);
  }

  // 6. Set 4th argument array, for match details, if present
  if (nargs == 4) {
    
    // Read the SUBSEP symbol
    awk_value_t subsep;
    if (!sym_lookup("SUBSEP", AWK_STRING, &subsep))
      warning(ext_id, "amatch: Could not get SUBSEP from gawk.");

    // Read 4th argument
    if (!get_argument(3, AWK_ARRAY, &substr)) {
      warning(ext_id, "amatch: Could not read 4th argument.");
    }
    else clear_array(substr.array_cookie);

    // Hand the substrings over to the substring array
    char outindexc[20];
    char outvalc[20];
    awk_value_t outindexp;
    awk_value_t outvalp;
    for (i = 0 ; i < match.nmatch; i++) {
      if (match.pmatch[i].rm_so != -1) {
        // Method 1:
        // start
        sprintf(outindexc, "%d%sstart", i, subsep.str_value.str);
        sprintf(outvalc, "%d", match.pmatch[i].rm_so+1);
        set_array_element(substr.array_cookie,                          \
                make_const_string(outindexc, strlen(outindexc), &outindexp),\
                make_const_string(outvalc, strlen(outvalc), &outvalp));
        // length
        sprintf(outindexc, "%d%slength", i, subsep.str_value.str);
        sprintf(outvalc, "%d", match.pmatch[i].rm_eo - match.pmatch[i].rm_so);
        set_array_element(substr.array_cookie,                        \
                make_const_string(outindexc, strlen(outindexc), &outindexp),\
                make_const_string(outvalc, strlen(outvalc), &outvalp));
        
        // Method 2:
        // sprintf(outindexc, "%d", i);
        // sprintf(outvalc, "%d %.*s", match.pmatch[i].rm_so+1, 
        //        match.pmatch[i].rm_eo - match.pmatch[i].rm_so, 
        //        str.str_value.str + match.pmatch[i].rm_so);
        // set_array_element(substr.array_cookie,                        
        //         make_const_string(outindexc, strlen(outindexc), &outindexp),
        //         make_const_string(outvalc, strlen(outvalc), &outvalp));
      }
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

