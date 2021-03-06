/*
 * lref_gconf.c
 *
 *  Created on: Dec 10, 2013
 *      Author: reboot
 */

#include <glutil.h>
#include "config.h"
#include "lref_gconf.h"

#include <str.h>
#include <lref.h>
#include <l_sb.h>
#include <omfp.h>

#include <errno.h>
#include "lref_generic.h"

void
dt_set_gconf (__g_handle hdl)
{
  hdl->flags |= F_GH_ISGCONF;
  hdl->block_sz = GC_SZ;
  hdl->d_memb = 1;
  hdl->g_proc0 = gcb_gconf;
  hdl->g_proc1_lookup = ref_to_val_lk_gconf;
  hdl->g_proc2 = ref_to_val_ptr_gconf;
  hdl->g_proc3 = gconf_format_block;
  hdl->g_proc3_batch = gconf_format_block_batch;
  hdl->g_proc3_export = gconf_format_block_exp;
  hdl->g_proc4 = g_omfp_norm;
  hdl->ipc_key = IPC_KEY_GCONFLOG;
  hdl->jm_offset = (size_t) &((__d_gconf) NULL)->r_clean;
}

int
gconf_format_block (void *iarg, char *output)
{
  __d_gconf data = (__d_gconf) iarg;

  return print_str("GCONF\x9%s\x9%s\x9%s\x9%s\x9%s\x9%hhd\x9%hhd\x9%s\x9%s\x9%s\x9%s\x9%s\x9%hhd\x9%hhd\x9%hhd\x9%hhd\x9%hhd\x9%s\x9%hhd\n",
      data->r_clean, data->r_postproc, data->r_yearm,data->r_exclude_user , data->r_sects, data->o_use_shared_mem,
      data->o_exec_on_lookup_fail, data->e_lookup_fail_imdb, data->e_lookup_fail_tvrage, data->e_match, data->r_skip_basedir,
      data->r_exclude_user_flags, data->o_lookup_strictness_imdb, data->o_lookup_strictness_tvrage, data->o_logging,
      data->o_imdb_skip_zero_score, data->o_r_clean_icase, data->o_log_string, data->o_imdb_skip_zero_votes);

}

int
gconf_format_block_batch (void *iarg, char *output)
{
  __d_gconf data = (__d_gconf) iarg;

  return printf("GCONF\x9%s\x9%s\x9%s\x9%s\x9%s\x9%hhd\x9%hhd\x9%s\x9%s\x9%s\x9%s\x9%s\x9%hhd\x9%hhd\x9%hhd\x9%hhd\x9%hhd\x9%s\x9%hhd\n",
      data->r_clean, data->r_postproc, data->r_yearm,data->r_exclude_user , data->r_sects, data->o_use_shared_mem,
      data->o_exec_on_lookup_fail, data->e_lookup_fail_imdb, data->e_lookup_fail_tvrage, data->e_match, data->r_skip_basedir,
      data->r_exclude_user_flags, data->o_lookup_strictness_imdb, data->o_lookup_strictness_tvrage, data->o_logging,
      data->o_imdb_skip_zero_score, data->o_r_clean_icase, data->o_log_string, data->o_imdb_skip_zero_votes);

}

int
gconf_format_block_exp (void *iarg, char *output)
{
  __d_gconf data = (__d_gconf) iarg;

  return printf("r_path_clean %s\n"
      "r_path_postproc %s\n"
      "r_year_extract %s\n"
      "r_exclude_user %s\n"
      "paths %s\n"
      "use_shared_mem %hhd\n"
      "execute_on_lookup_fail %hhd\n"
      "path_exec_on_lookup_fail_imdb %s\n"
      "path_exec_on_lookup_fail_tvrage %s\n"
      "path_exec_on_match %s\n"
      "r_skip_basedir %s\n"
      "r_exclude_user_flags %s\n"
      "lookup_match_strictness_imdb %hhd\n"
      "lookup_match_strictness_tvrage %hhd\n"
      "logging %hhd\n"
      "imdb_skip_zero_score %hhd\n"
      "r_path_clean_icase %hhd\n"
      "log_string %s\n"
      "imdb_skip_zero_score %hhd\n\n"
      , data->r_clean, data->r_postproc, data->r_yearm,data->r_exclude_user , data->r_sects, data->o_use_shared_mem,
      data->o_exec_on_lookup_fail, data->e_lookup_fail_imdb, data->e_lookup_fail_tvrage, data->e_match, data->r_skip_basedir,
      data->r_exclude_user_flags, data->o_lookup_strictness_imdb, data->o_lookup_strictness_tvrage, data->o_logging,
      data->o_imdb_skip_zero_score, data->o_r_clean_icase, data->o_log_string, data->o_imdb_skip_zero_votes);

}

void *
ref_to_val_ptr_gconf_e (void *arg, char *match, int *output)
{
  __d_gconf data = (__d_gconf) arg;
  if (!strncmp(match, _MC_GCONF_O_SHM, 14))
    {
      *output = ~((int) sizeof(data->o_use_shared_mem));
      return &data->o_use_shared_mem;
    }
  else if (!strncmp(match, _MC_GCONF_E_OLF, 22))
    {
      *output = ~((int) sizeof(data->o_exec_on_lookup_fail));
      return &data->o_exec_on_lookup_fail;
    }
  else if (!strncmp(match, _MC_GCONF_STRCTNSI, 28))
    {
      *output = ~((int) sizeof(data->o_lookup_strictness_imdb));
      return &data->o_lookup_strictness_imdb;
    }
  else if (!strncmp(match, _MC_GCONF_STRCTNST, 30))
    {
      *output = ~((int) sizeof(data->o_lookup_strictness_tvrage));
      return &data->o_lookup_strictness_tvrage;
    }
  else if (!strncmp(match, _MC_GCONF_LOGGING, 7))
    {
      *output = ~((int) sizeof(data->o_logging));
      return &data->o_logging;
    }
  else if (!strncmp(match, _MC_GCONF_IMDB_SKZERO, 20))
    {
      *output = ~((int) sizeof(data->o_imdb_skip_zero_score));
      return &data->o_imdb_skip_zero_score;
    }
  else if (!strncmp(match, _MC_GCONF_IMDB_SKZEROV, 20))
    {
      *output = ~((int) sizeof(data->o_imdb_skip_zero_votes));
      return &data->o_imdb_skip_zero_votes;
    }
  else if (!strncmp(match, _MC_GCONF_R_CLEAN_ICASE, 18))
    {
      *output = ~((int) sizeof(data->o_r_clean_icase));
      return &data->o_r_clean_icase;
    }

  return NULL;
}

void *
ref_to_val_ptr_gconf (void *arg, char *match, int *output)
{
  REF_TO_VAL_RESOLVE(arg, match, output, ref_to_val_ptr_gconf_e)
}

char *
dt_rval_gconf_shm (void *arg, char *match, char *output, size_t max_size,
		   void *mppd)
{
  snprintf (output, max_size, ((__d_drt_h ) mppd)->direc, ((__d_gconf) arg)->o_use_shared_mem);
  return output;
}

char *
dt_rval_gconf_e_olf (void *arg, char *match, char *output, size_t max_size,
		     void *mppd)
{
  snprintf (output, max_size, ((__d_drt_h ) mppd)->direc, ((__d_gconf) arg)->o_exec_on_lookup_fail);
  return output;
}

char *
dt_rval_gconf_e_strct (void *arg, char *match, char *output, size_t max_size,
		       void *mppd)
{
  snprintf (output, max_size, ((__d_drt_h ) mppd)->direc, ((__d_gconf) arg)->o_lookup_strictness_imdb);
  return output;
}

char *
dt_rval_gconf_e_strct2 (void *arg, char *match, char *output, size_t max_size,
			void *mppd)
{
  snprintf (output, max_size, ((__d_drt_h ) mppd)->direc, ((__d_gconf) arg)->o_lookup_strictness_tvrage);
  return output;
}

char *
dt_rval_gconf_r_clean (void *arg, char *match, char *output, size_t max_size,
		       void *mppd)
{
  return ((__d_gconf) arg)->r_clean;
}

char *
dt_rval_gconf_r_postproc (void *arg, char *match, char *output, size_t max_size,
			  void *mppd)
{
  return ((__d_gconf) arg)->r_postproc;
}

char *
dt_rval_gconf_r_yearm (void *arg, char *match, char *output, size_t max_size,
		       void *mppd)
{
  return ((__d_gconf) arg)->r_yearm;
}

char *
dt_rval_gconf_r_sects (void *arg, char *match, char *output, size_t max_size,
		       void *mppd)
{
  return ((__d_gconf) arg)->r_sects;
}

char *
dt_rval_gconf_e_lf_imdb (void *arg, char *match, char *output, size_t max_size,
			 void *mppd)
{
  return ((__d_gconf) arg)->e_lookup_fail_imdb;
}

char *
dt_rval_gconf_e_lf_tvrage (void *arg, char *match, char *output,
			   size_t max_size, void *mppd)
{
  return ((__d_gconf) arg)->e_lookup_fail_tvrage;
}

char *
dt_rval_gconf_e_m (void *arg, char *match, char *output, size_t max_size,
		   void *mppd)
{
  return ((__d_gconf) arg)->e_match;
}

char *
dt_rval_gconf_sbdir (void *arg, char *match, char *output, size_t max_size,
		     void *mppd)
{
  return ((__d_gconf) arg)->r_skip_basedir;
}

char *
dt_rval_gconf_ex_u (void *arg, char *match, char *output, size_t max_size,
		    void *mppd)
{
  return ((__d_gconf) arg)->r_exclude_user;
}

char *
dt_rval_gconf_ex_uf (void *arg, char *match, char *output, size_t max_size,
		     void *mppd)
{
  return ((__d_gconf) arg)->r_exclude_user_flags;
}

char *
dt_rval_gconf_o_log (void *arg, char *match, char *output, size_t max_size,
		     void *mppd)
{
  snprintf (output, max_size, ((__d_drt_h ) mppd)->direc, ((__d_gconf) arg)->o_logging);
  return output;
}

static char *
dt_rval_gconf_o_imdb_skip_zero_score (void *arg, char *match, char *output,
				      size_t max_size, void *mppd)
{
  snprintf (output, max_size, ((__d_drt_h ) mppd)->direc, ((__d_gconf) arg)->o_imdb_skip_zero_score);
  return output;
}

static char *
dt_rval_gconf_o_imdb_skip_zero_votes (void *arg, char *match, char *output,
				      size_t max_size, void *mppd)
{
  snprintf (output, max_size, ((__d_drt_h ) mppd)->direc, ((__d_gconf) arg)->o_imdb_skip_zero_votes);
  return output;
}

static char *
dt_rval_gconf_o_r_clean_icase (void *arg, char *match, char *output,
			       size_t max_size, void *mppd)
{
  snprintf (output, max_size, ((__d_drt_h ) mppd)->direc, ((__d_gconf) arg)->o_r_clean_icase);
  return output;
}

static char *
dt_rval_gconf_o_log_string (void *arg, char *match, char *output,
			    size_t max_size, void *mppd)
{
  snprintf (output, max_size, ((__d_drt_h ) mppd)->direc, ((__d_gconf) arg)->o_log_string);
  return output;
}

void *
ref_to_val_lk_gconf (void *arg, char *match, char *output, size_t max_size,
		     void *mppd)
{
  PROC_SH_EX(match)

  void *ptr;
  if ((ptr = ref_to_val_lk_generic (arg, match, output, max_size, mppd)))
    {
      return ptr;
    }

  if (!strncmp (match, _MC_GCONF_R_CLEAN, 12) && (l_mppd_gvlen (match) == 12))
    {
      return as_ref_to_val_lk (match, dt_rval_gconf_r_clean, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_GCONF_R_POSTPROC, 15))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_r_postproc, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_GCONF_R_YEARM, 14))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_r_yearm, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_GCONF_R_SECTS, 5))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_r_sects, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_GCONF_O_SHM, 14))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_shm , (__d_drt_h) mppd, "%hhd");
    }
  else if (!strncmp(match, _MC_GCONF_E_LF_IMDB, 29))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_e_lf_imdb , (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_GCONF_E_LF_TVRAGE, 31))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_e_lf_tvrage , (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_GCONF_E_OLF, 22))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_e_olf , (__d_drt_h) mppd, "%hhd");
    }
  else if (!strncmp(match, _MC_GCONF_E_M, 18))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_e_m , (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_GCONF_R_SKIPBDIR, 14))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_sbdir , (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_GCONF_EX_UF, 20))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_ex_uf , (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_GCONF_EX_U, 14))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_ex_u , (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_GCONF_STRCTNSI, 28))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_e_strct , (__d_drt_h) mppd, "%hhd");
    }
  else if (!strncmp(match, _MC_GCONF_STRCTNST, 30))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_e_strct2 , (__d_drt_h) mppd, "%hhd");
    }
  else if (!strncmp(match, _MC_GCONF_LOGGING, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_o_log , (__d_drt_h) mppd, "%hhd");
    }
  else if (!strncmp(match, _MC_GCONF_IMDB_SKZERO, 20))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_o_imdb_skip_zero_score , (__d_drt_h) mppd, "%hhd");
    }
  else if (!strncmp(match, _MC_GCONF_IMDB_SKZEROV, 20))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_o_imdb_skip_zero_votes , (__d_drt_h) mppd, "%hhd");
    }
  else if (!strncmp(match, _MC_GCONF_R_CLEAN_ICASE, 18))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_o_r_clean_icase , (__d_drt_h) mppd, "%hhd");
    }
  else if (!strncmp(match, _MC_GCONF_LOGSTR, 10))
    {
      return as_ref_to_val_lk(match, dt_rval_gconf_o_log_string , (__d_drt_h) mppd, "%s");
    }

  return NULL;
}

int
gcb_gconf (void *buffer, char *key, char *val)
{
  size_t k_l = strlen (key), v_l;
  __d_gconf ptr = (__d_gconf) buffer;
  errno = 0;

  if (k_l == 12 && !strncmp (key, _MC_GCONF_R_CLEAN, 12))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return -1;
	}
      memcpy (ptr->r_clean, val,
	      v_l >= GCONF_MAX_REG_EXPR ? GCONF_MAX_REG_EXPR - 1 : v_l);
      return -1;
    }
  else if (k_l == 10 && !strncmp (key, _MC_GCONF_LOGSTR, 10))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return -1;
	}
      memcpy (
	  ptr->o_log_string,
	  val,
	  v_l >= sizeof(ptr->o_log_string) ?
	      sizeof(ptr->o_log_string) - 1 : v_l);
      return -1;
    }
  else if (k_l == 15 && !strncmp (key, _MC_GCONF_R_POSTPROC, 15))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return -1;
	}
      memcpy (ptr->r_postproc, val,
	      v_l >= GCONF_MAX_REG_EXPR ? GCONF_MAX_REG_EXPR - 1 : v_l);
      return -1;
    }
  else if (k_l == 14 && !strncmp (key, _MC_GCONF_R_YEARM, 14))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return -1;
	}
      memcpy (ptr->r_yearm, val,
	      v_l >= GCONF_MAX_REG_EXPR ? GCONF_MAX_REG_EXPR - 1 : v_l);
      return -1;
    }
  else if (k_l == 5 && !strncmp (key, _MC_GCONF_R_SECTS, 5))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return 0;
	}
      memcpy (ptr->r_sects, val,
	      v_l >= GCONF_MAX_REG_EXPR ? GCONF_MAX_REG_EXPR - 1 : v_l);
      return 1;
    }
  else if (k_l == 14 && !strncmp (key, _MC_GCONF_O_SHM, 14))
    {
      uint8_t v_ui = (uint8_t) strtoul (val, NULL, 10);
      if ( errno == ERANGE)
	{
	  return -1;
	}
      ptr->o_use_shared_mem = v_ui;
      return -1;
    }
  else if (k_l == 22 && !strncmp (key, _MC_GCONF_E_OLF, 22))
    {
      uint8_t v_ui = (uint8_t) strtoul (val, NULL, 10);
      if ( errno == ERANGE)
	{
	  return -1;
	}
      ptr->o_exec_on_lookup_fail = v_ui;
      return -1;
    }
  else if (k_l == 29 && !strncmp (key, _MC_GCONF_E_LF_IMDB, 29))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return -1;
	}
      memcpy (ptr->e_lookup_fail_imdb, val,
	      v_l >= GCONF_MAX_EXEC ? GCONF_MAX_EXEC - 1 : v_l);
      return -1;
    }
  else if (k_l == 31 && !strncmp (key, _MC_GCONF_E_LF_TVRAGE, 31))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return -1;
	}
      memcpy (ptr->e_lookup_fail_tvrage, val,
	      v_l >= GCONF_MAX_EXEC ? GCONF_MAX_EXEC - 1 : v_l);
      return -1;
    }
  else if (k_l == 18 && !strncmp (key, _MC_GCONF_E_M, 18))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return -1;
	}
      memcpy (ptr->e_match, val,
	      v_l >= GCONF_MAX_EXEC ? GCONF_MAX_EXEC - 1 : v_l);
      return -1;
    }
  else if (k_l == 14 && !strncmp (key, _MC_GCONF_R_SKIPBDIR, 14))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return -1;
	}
      memcpy (ptr->r_skip_basedir, val,
	      v_l >= GCONF_MAX_REG_EXPR ? GCONF_MAX_REG_EXPR - 1 : v_l);
      return -1;
    }
  else if (k_l == 20 && !strncmp (key, _MC_GCONF_EX_UF, 20))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return -1;
	}
      memcpy (ptr->r_exclude_user_flags, val,
	      v_l >= GCONF_MAX_UFLAGS ? GCONF_MAX_UFLAGS - 1 : v_l);
      return -1;
    }
  else if (k_l == 14 && !strncmp (key, _MC_GCONF_EX_U, 14))
    {
      if (!(v_l = strlen (val) + 1))
	{
	  return -1;
	}
      memcpy (ptr->r_exclude_user, val,
	      v_l >= GCONF_MAX_REG_USR ? GCONF_MAX_REG_USR - 1 : v_l);
      return -1;
    }
  else if (k_l == 28 && !strncmp (key, _MC_GCONF_STRCTNSI, 28))
    {
      int8_t v_ui = (int8_t) strtol (val, NULL, 10);
      if ( errno == ERANGE)
	{
	  return -1;
	}
      ptr->o_lookup_strictness_imdb = v_ui;
      return 1;
    }
  else if (k_l == 30 && !strncmp (key, _MC_GCONF_STRCTNST, 30))
    {
      int8_t v_ui = (int8_t) strtol (val, NULL, 10);
      if ( errno == ERANGE)
	{
	  return -1;
	}
      ptr->o_lookup_strictness_tvrage = v_ui;
      return 1;
    }
  else if (k_l == 7 && !strncmp (key, _MC_GCONF_LOGGING, 7))
    {
      int8_t v_ui = (int8_t) strtol (val, NULL, 10);
      if ( errno == ERANGE)
	{
	  return -1;
	}
      ptr->o_logging = v_ui;
      return 1;
    }
  else if (k_l == 20 && !strncmp (key, _MC_GCONF_IMDB_SKZERO, 20))
    {
      int8_t v_ui = (int8_t) strtol (val, NULL, 10);
      if ( errno == ERANGE)
	{
	  return -1;
	}
      ptr->o_imdb_skip_zero_score = v_ui;
      return 1;
    }
  else if (k_l == 20 && !strncmp (key, _MC_GCONF_IMDB_SKZEROV, 20))
    {
      int8_t v_ui = (int8_t) strtol (val, NULL, 10);
      if ( errno == ERANGE)
	{
	  return -1;
	}
      ptr->o_imdb_skip_zero_votes = v_ui;
      return 1;
    }
  else if (k_l == 18 && !strncmp (key, _MC_GCONF_R_CLEAN_ICASE, 18))
    {
      int8_t v_ui = (int8_t) strtol (val, NULL, 10);
      if ( errno == ERANGE)
	{
	  return -1;
	}
      ptr->o_r_clean_icase = v_ui;
      return 1;
    }

  return -1;
}
