/*
 * lref_glob.c
 *
 *  Created on: Aug 27, 2014
 *      Author: reboot
 */

#include <glutil.h>
#include "lref_glob.h"

#include <l_error.h>
#include <mc_glob.h>
#include <str.h>
#include <lref.h>

#include <errno.h>

int64_t glob_curtime = 0;

static void *
g_proc_gstor (__g_handle hdl, char *field, int *output)
{
  if (is_ascii_numeric (field[7]))
    {
      return NULL;
    }

  errno = 0;

  int idx = (int) strtol (&field[7], NULL, 10);

  if (errno == ERANGE || errno == EINVAL)
    {
      return NULL;
    }

  if (!strncmp (field, _MC_GLOB_U64G, 7))
    {
      *output = 8;
      memset ((void*) &glob_ui64_stor[idx], 0x0, sizeof(uint64_t));
      return (void*) &glob_ui64_stor[idx];
    }
  else if (!strncmp (field, _MC_GLOB_S64G, 7))
    {
      *output = -8;
      memset ((void*) &glob_si64_stor[idx], 0x0, sizeof(int64_t));
      return (void*) &glob_si64_stor[idx];
    }
  else if (!strncmp (field, _MC_GLOB_F32G, 7))
    {
      *output = -32;
      memset ((void*) &glob_float_stor[idx], 0x0, sizeof(float));
      return (void*) &glob_float_stor[idx];
    }

  return NULL;
}

void *
g_get_glob_ptr_e (__g_handle hdl, char *field, int *output)
{
  g_setjmp (0, "g_get_glob_ptr", NULL, NULL);

  if (!strncmp (field, _MC_GLOB_CURTIME, 7))
    {
      *output = -33;
      memset ((void*) &glob_curtime, 0x0, sizeof(glob_curtime));
      return (void*) &glob_curtime;
    }
  else
    {
      return g_proc_gstor (hdl, field, output);
    }
}

void *
g_get_glob_ptr (void *arg, char *match, int *output)
{
  REF_TO_VAL_RESOLVE(arg, match, output, g_get_glob_ptr_e)
}

