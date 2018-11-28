/*
 * aregex.c - Gawk extension to access the TRE approximate regex.
 * Copyright (C) 2018 Cam Webb, <cw@camwebb.info>
 * Distributed under the GNU Pulbic Licence v3
 */

/*
 * Program notes: 1. While the amatch() function is roughly equivalent
 * to the gawk match() function, I chose not to return [i,"start"]
 * position and [i,"length"] in the returned substring array, but to
 * return just the literal substring for each parenthetical
 * match. Gawk is multibyte aware, and match() works in terms of
 * characters, not bytes, but TRE is not character-based. Using the
 * 'wchar_t' versions of tre_regcomp and tre_regaexec does not help if
 * the input is a mix of single and multi-byte characters. A simple
 * routine must be used on the output array, if positions and lengths
 * of the substrings are needed.
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
#define DEBUG 0
#define RTNSUBARRMETHOD 1

// Gawkextlib boilerplate:
static const gawk_api_t *api;
static awk_ext_id_t ext_id;
int plugin_is_GPL_compatible;

// Main amatch() function definition
static awk_value_t * do_amatch(int nargs, awk_value_t *result, \
                            struct awk_ext_func *unused)
{
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
  awk_value_t costs;
  awk_value_t costindex;
  awk_value_t costval;
  awk_bool_t hascostarr = 0;
  
  if (nargs > 2) {
    if (!get_argument(2, AWK_ARRAY, &costs))
      fatal(ext_id, "amatch: 3rd argument present, but could not be read.");
    hascostarr = 1;
    
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

  // ( for wchar_t:
  //   wchar_t rew[] = L"";
  //   swprintf(rew, strlen(re.str_value.str), L"%ls", re.str_value.str); )
  
  // 4. Compile regex
  regex_t preg;
  tre_regcomp(&preg, re.str_value.str, REG_EXTENDED);

  // ( for wchar_t:
  //   tre_regwcomp(&preg, rew, REG_EXTENDED); )

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

  // ( for wchar_t:
  //   treret = tre_regawexec(&pregw, rew, &match, params, 0); )
  
  // 5d. Set the amatch() return value depending on tre_regaexec() return:
  // Return 1 if success, 0 if no match,
  int rval;
  if (treret == REG_NOMATCH) rval = 0;
  else rval = 1;

  // Catch a "mem. not. allocated" return from tre_regaexec()
  if (treret == REG_ESPACE) {
    warning(ext_id,                                                     \
            "amatch: TRE err., mem. insufficient to complete the match.");
    return make_null_string(result);
  }

  // 6. If there is a cost array, set some return values (if matched)
  if ((hascostarr) && (rval > -1)) {
    char matchcost[20]; // Single integers, max width ~= 10
    // cost
    del_array_element(costs.array_cookie,                           \
          make_const_string("cost", strlen("cost"), &costindex));
    sprintf(matchcost, "%d", match.cost);
    set_array_element(costs.array_cookie, \
          make_const_string("cost", strlen("cost"), &costindex), \
          make_const_string(matchcost, strlen(matchcost), &costval));
    // num_ins;
    del_array_element(costs.array_cookie,                               \
          make_const_string("num_ins", strlen("num_ins"), &costindex));
    sprintf(matchcost, "%d", match.num_ins);
    set_array_element(costs.array_cookie, \
          make_const_string("num_ins", strlen("num_ins"), &costindex), \
          make_const_string(matchcost, strlen(matchcost), &costval));
    // num_del;
    del_array_element(costs.array_cookie,                               \
          make_const_string("num_del", strlen("num_del"), &costindex));
    sprintf(matchcost, "%d", match.num_del);
    set_array_element(costs.array_cookie, \
          make_const_string("num_del", strlen("num_del"), &costindex), \
          make_const_string(matchcost, strlen(matchcost), &costval));
    // num_subst;
    del_array_element(costs.array_cookie,                               \
          make_const_string("num_subst", strlen("num_subst"), &costindex));
    sprintf(matchcost, "%d", match.num_subst);
    set_array_element(costs.array_cookie, \
          make_const_string("num_subst", strlen("num_subst"), &costindex), \
          make_const_string(matchcost, strlen(matchcost), &costval));
  }
  
  // 7. Set 4th argument array, for matched substrings, if present
  //    and if a match found
  if ((nargs == 4) && (rval > -1)) {
    awk_value_t substr; 
    // Read 4th argument
    if (!get_argument(3, AWK_ARRAY, &substr)) {
      warning(ext_id, "amatch: Could not read 4th argument.");
    }
    else clear_array(substr.array_cookie);

    // Hand the TRE substrings over to the gawk substring array
    char outindexc[20];  
    char outvalc[20]; // TODO! Dimension based on max length
    awk_value_t outindexp;
    awk_value_t outvalp;

    if (RTNSUBARRMETHOD == 1) {
      for (i = 0 ; i < match.nmatch; i++) {
        if (match.pmatch[i].rm_so != -1) {
          sprintf(outindexc, "%d", i);
          // "%d %.*s", match.pmatch[i].rm_so+1, ... gives position
          //   by bytes, not by chars
          sprintf(outvalc, "%.*s", \
                  match.pmatch[i].rm_eo - match.pmatch[i].rm_so, \
                  str.str_value.str + match.pmatch[i].rm_so);
          set_array_element(substr.array_cookie,                        
                  make_const_string(outindexc, strlen(outindexc), &outindexp),\
                  make_const_string(outvalc, strlen(outvalc), &outvalp));
        }
      }
    }
    else if (RTNSUBARRMETHOD == 2) {
      // Read the SUBSEP symbol
      awk_value_t subsep;
      if (!sym_lookup("SUBSEP", AWK_STRING, &subsep))
        warning(ext_id, "amatch: Could not get SUBSEP from gawk.");

      for (i = 0 ; i < match.nmatch; i++) {
        if (match.pmatch[i].rm_so != -1) {
          // Note back in gawk, determine arr dimansions with 'length(arr)/2'
          // start position
          sprintf(outindexc, "%d%sstart", i, subsep.str_value.str);
          sprintf(outvalc, "%d", match.pmatch[i].rm_so+1);
          set_array_element(substr.array_cookie,                        \
                make_const_string(outindexc, strlen(outindexc), &outindexp), \
                make_const_string(outvalc, strlen(outvalc), &outvalp));
          // length
          sprintf(outindexc, "%d%slength", i, subsep.str_value.str);
          sprintf(outvalc, "%d", match.pmatch[i].rm_eo - match.pmatch[i].rm_so);
          set_array_element(substr.array_cookie,                        \
                make_const_string(outindexc, strlen(outindexc), &outindexp),\
                make_const_string(outvalc, strlen(outvalc), &outvalp));
        }
      }
    }
  }
  return make_number(rval, result);
}


// Gawkextlib boilerplate:

static awk_ext_func_t func_table[] = \
    {
     { "amatch", do_amatch, 4, 2, awk_false, NULL  },
    };

static awk_bool_t (*init_func)(void) = NULL;

static const char *ext_version = "0.1";

dl_load_func(func_table, amatch, "")

// TODO! Free malloc
