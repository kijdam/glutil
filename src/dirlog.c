/*
 * dirlog.c
 *
 *  Created on: Dec 5, 2013
 *      Author: reboot
 */

#include <glutil.h>
#include "config.h"
#include "dirlog.h"

#include <l_error.h>
#include <g_modes.h>

#include <log_io.h>
#include <log_op.h>
#include <str.h>
#include <x_f.h>
#include <xref.h>
#include <m_general.h>
#include <lref_dirlog.h>
#include <errno_int.h>

#include <stdio.h>
#include <errno.h>
#include <regex.h>
#include <libgen.h>
#include <dirent.h>
#include <utime.h>

#define         ACT_WRITE_BUFFER_MEMBERS        10000

int
rebuild_dirlog (void)
{
  g_setjmp (0, "rebuild_dirlog", NULL, NULL);
  char mode[255] =
    { 0 };
  uint32_t flags = 0;
  int rt = 0;

  gfl0 |= F_OPT_STATS;

  if (!(ofl & F_OVRR_NUKESTR))
    {
      print_str (
	  "WARNING: failed extracting nuke string from '%s', nuked dirs might not get detected properly\n",
	  GLCONF);
    }

  if (gfl & F_OPT_NOWRITE)
    {
      strncpy (mode, "r", 1);
      flags |= F_DL_FOPEN_BUFFER;
    }
  else if (gfl & F_OPT_UPDATE)
    {
      strncpy (mode, "a+", 2);
      flags |= F_DL_FOPEN_BUFFER | F_DL_FOPEN_FILE;
    }
  else
    {
      strncpy (mode, "w+", 2);
    }

  if ((gfl & F_OPT_UPDATE) && !(gfl0 & F_OPT_NOHT))
    {
      gfl0 |= F_OPT_MAKEHT;
      ht_field = "dir";
      g_act_1.cb0 = (void*) dirlog_find_ht;
    }
  else
    {
      g_act_1.cb0 = (void*) dirlog_find_a;
    }

  if (gfl & F_OPT_WBUFFER)
    {
      strncpy (g_act_1.mode, "r", 1);
      if (gfl & F_OPT_VERBOSE3)
	{
	  print_str (
	      "NOTICE: %s: allocating %u bytes for references (overhead)\n",
	      DIRLOG, (uint32_t) (ACT_WRITE_BUFFER_MEMBERS * sizeof(md_obj)));
	}
      md_init (&g_act_1.w_buffer, ACT_WRITE_BUFFER_MEMBERS);
      g_act_1.block_sz = DL_SZ;
      g_act_1.flags |= F_GH_FFBUFFER | F_GH_WAPPEND
	  | ((gfl & F_OPT_UPDATE) ? F_GH_DFNOWIPE : 0);
      if (gfl & F_OPT_VERBOSE)
	{
	  print_str ("NOTICE: %s: explicit write pre-caching enabled\n",
		     DIRLOG);
	}
    }
  else
    {
      strncpy (g_act_1.mode, mode, strlen (mode));
    }

  int dfex = file_exists (DIRLOG);

  if ((gfl & F_OPT_UPDATE) && dfex)
    {
      print_str (
	  "WARNING: %s: requested update, but no dirlog exists - removing update flag..\n",
	  DIRLOG);
      gfl ^= F_OPT_UPDATE;
      if (flags & F_DL_FOPEN_BUFFER)
	{
	  flags ^= F_DL_FOPEN_BUFFER;
	}
    }

  int r;

  data_backup_records (DIRLOG);
  g_act_1.flags |= F_GH_NO_BACKUP;

  if (!strncmp (g_act_1.mode, "r", 1) && dfex)
    {
      if (gfl & F_OPT_VERBOSE)
	{
	  print_str (
	      "WARNING: %s: requested read mode access but file not there\n",
	      DIRLOG);
	}

      snprintf (g_act_1.file, sizeof(g_act_1.file), "%s", DIRLOG);
      if (determine_datatype (&g_act_1, DIRLOG))
	{
	  print_str ("ERROR: %s: determine_datatype failed\n", DIRLOG);
	  return 31;
	}
      if (g_proc_mr (&g_act_1))
	{
	  print_str ("ERROR: %s: g_proc_mr failed\n", DIRLOG);
	  return 32;
	}
    }
  else if ((r = g_fopen (DIRLOG, g_act_1.mode, flags, &g_act_1)))
    {
      print_str ("ERROR: [%d] could not open dirlog, mode '%s', flags %u\n", r,
		 g_act_1.mode, flags);
      return errno;
    }

  mda dirchain =
    { 0 }, buffer2 =
    { 0 };

  if (g_proc_mr (&g_act_1))
    {
      return 12;
    }

  if (gfl & F_OPT_UPDATE)
    {
      print_str ("NOTICE: requested dirlog update only\n");
    }

  if (gfl & F_OPT_DIR_FULL_REBUILD)
    {
      print_str ("NOTICE: performing a full siteroot rescan\n");
      print_str ("SCANNING: '%s'\n", SITEROOT);
      update_records (SITEROOT, 0);
      goto rw_end;
    }

  char buffer[V_MB + 1] =
    { 0 };

  md_init (&dirchain, 128);

  if (read_file (DU_FLD, buffer, V_MB, 0, NULL) < 1)
    {
      print_str (
	  "ERROR: unable to read folders file '%s', read MANUAL on how to set it up, or use '--full' option to do a full rescan..\n",
	  DU_FLD, SITEROOT);
      goto r_end;
    }

  int r2;

  if ((r = split_string (buffer, 0x13, &dirchain)) < 1)
    {
      print_str ("ERROR: [%d] could not parse input from %s\n", r, DU_FLD);
      rt = 5;
      goto r_end;
    }

  int i = 0, ib;
  char s_buffer[PATH_MAX] =
    { 0 };
  p_md_obj ptr = dirchain.objects;

  while (ptr)
    {
      if (!sigsetjmp(g_sigjmp.env, 1))
	{
	  g_setjmp (F_SIGERR_CONTINUE, "rebuild_dirlog(loop)", NULL,
	  NULL);
	  if (gfl & F_OPT_KILL_GLOBAL)
	    {
	      rt = EXITVAL;
	      break;
	    }

	  md_init (&buffer2, 6);
	  i++;
	  if ((r2 = split_string ((char*) ptr->ptr, 0x20, &buffer2)) != 2)
	    {
	      print_str ("ERROR: [%d] could not parse line %d from %s\n", r2, i,
			 DU_FLD);
	      goto lend;
	    }
	  bzero (s_buffer, PATH_MAX);
	  snprintf (s_buffer, PATH_MAX, "%s/%s", SITEROOT,
		    (char*) buffer2.objects->ptr);
	  remove_repeating_chars (s_buffer, 0x2F);

	  size_t s_buf_len = strlen (s_buffer);
	  if (s_buffer[s_buf_len - 1] == 0x2F)
	    {
	      s_buffer[s_buf_len - 1] = 0x0;
	    }

	  ib = strtol ((char*) ((p_md_obj) buffer2.objects->next)->ptr,
	  NULL,
		       10);

	  if (errno == ERANGE)
	    {
	      print_str ("ERROR: could not get depth from line %d\n", i);
	      goto lend;
	    }
	  if (dir_exists (s_buffer))
	    {
	      print_str ("ERROR: %s: directory doesn't exist (line %d)\n",
			 s_buffer, i);
	      goto lend;
	    }
	  char *ndup = strdup (s_buffer);
	  char *nbase = basename (ndup);

	  print_str ("SCANNING: '%s', depth: %d\n", nbase, ib);
	  if (update_records (s_buffer, ib) < 1)
	    {
	      print_str ("WARNING: %s: nothing was processed\n", nbase);
	    }

	  free (ndup);
	  lend:

	  md_free (&buffer2);
	}
      ptr = ptr->next;
    }

  rw_end:

  if (g_act_1.flags & F_GH_FFBUFFER)
    {
      if (rebuild_data_file (DIRLOG, &g_act_1))
	{
	  print_str (MSG_GEN_DFRFAIL, DIRLOG);
	}
    }

  r_end:

  md_free (&dirchain);

  g_close (&g_act_1);

  if (((gfl0 & F_OPT_STATS) || (gfl & F_OPT_VERBOSE4))
      && !(gfl0 & F_OPT_NOSTATS))
    {
#ifdef HAVE_ZLIB_H
      if (g_act_1.flags & F_GH_IO_GZIP)
	{
	  fprintf(stderr, MSG_GEN_WROTE2, DIRLOG, (double) dl_stats.bw / 1024.0,
	      (double) g_act_1.bw / 1024.0, (long long unsigned) dl_stats.rw);
	}
      else
	{
	  OPLOG_OUTPUT_NSTATS(g_act_1.file, dl_stats)
	}
#else
      OPLOG_OUTPUT_NSTATS(g_act_1.file, dl_stats)
#endif
    }

  return rt;
}

int
update_records (char *dirname, int depth)
{
  g_setjmp (0, "update_records", NULL, NULL);
  struct dirlog buffer =
    { 0 };
  ear arg =
    { 0 };

  arg.depth = depth;
  arg.dirlog = &buffer;

  _g_eds eds =
    { 0 };

  arg.eds = &eds;

  int r = enum_dir (dirname, proc_section, &arg, 0, &eds, tp_default);

  if (r < 0)
    {
      print_str ("ERROR: enum_dir: %s: [%d] %s\n", dirname, r,
		 ie_tl (r, EMR_enum_dir));
    }

  return r;

}

int
proc_directory (char *name, unsigned char type, void *arg, __g_eds eds)
{
  ear *iarg = (ear*) arg;
  uint32_t crc32 = 0;
  char buffer[PATH_MAX] =
    { 0 };
  char *fn, *fn2, *base;

  if (!reg_match ("(\\/|^)[.]{1,2}", name, 0))
    {
      return 1;
    }

  switch (type)
    {
    case DT_REG:
      fn2 = strdup (name);
      base = g_basename (name);
      if ((gfl & F_OPT_SFV)
	  && (updmode == UPD_MODE_RECURSIVE || updmode == UPD_MODE_SINGLE)
	  && reg_match (PREG_SFV_SKIP_EXT, fn2,
	  REG_ICASE | REG_NEWLINE) && file_crc32 (name, &crc32) > 0)
	{
	  fn = strdup (name);
	  char *dn = g_basename (g_dirname (fn));
	  free (fn2);
	  fn2 = strdup (name);
	  snprintf (iarg->buffer, PATH_MAX, "%s/%s.sfv.tmp", g_dirname (fn2),
		    dn);
	  snprintf (iarg->buffer2, PATH_MAX, "%s/%s.sfv", fn2, dn);
	  char buffer2[PATH_MAX + 10];
	  snprintf (buffer2, 1024, "%s %.8X\n", base, (uint32_t) crc32);
	  if (!(gfl & F_OPT_NOWRITE) || (gfl & F_OPT_FORCEWSFV))
	    {
	      if (!write_file_text (buffer2, iarg->buffer))
		{
		  print_str ("ERROR: %s: failed writing to SFV file: '%s'\n",
			     name, iarg->buffer);
		}
	    }
	  iarg->flags |= F_EARG_SFV;
	  snprintf (buffer, PATH_MAX, "  %.8X", (uint32_t) crc32);
	  free (fn);
	}
      uint64_t fs = (uint64_t) get_file_size (name);
      iarg->dirlog->bytes += fs;
      iarg->dirlog->files++;
      if ((gfl & F_OPT_VERBOSE4))
	{
	  print_str ("     %s  %.2fMB%s\n", name, (double) fs / 1024.0 / 1024.0,
		     buffer);
	}

      free (fn2);
      break;
    case DT_DIR:
      if (gfl0 & F_OPT_DRINDEPTH)
	{
	  int r;
	  if ((gfl & F_OPT_SFV)
	      && (!(gfl & F_OPT_NOWRITE) || (gfl & F_OPT_FORCEWSFV)))
	    {
	      r = enum_dir (name, delete_file, (void*) "\\.sfv(\\.tmp|)$", 0,
	      NULL,
			    tp_default);
	      if (r < 0)
		{
		  print_str (
		      "ERROR: proc_directory->enum_dir->delete_file (clean SFV): %s: [%d] %s\n",
		      name, r, ie_tl (r, EMR_enum_dir));
		}
	    }

	  r = enum_dir (name, proc_directory, iarg, 0, eds, tp_default);
	  if (r < 0)
	    {
	      print_str (
		  "ERROR: proc_directory->enum_dir->proc_directory: %s: [%d] %s\n",
		  name, r, ie_tl (r, EMR_enum_dir));
	    }
	}
      break;
    }

  return 0;
}

static void
proc_sect_edec (char *name, void *callback, ear *iarg, __g_eds eds)
{
  if (!((gfl & F_OPT_MAXDEPTH) && eds->depth >= max_depth))
    {
      int r = enum_dir (name, callback, iarg, 0, eds, tp_default);
      if (r < 0)
	{
	  print_str ("ERROR: proc_sect_edec->enum_dir->(%.16X): %s: [%d] %s\n",
		     callback, name, r, ie_tl (r, EMR_enum_dir));
	}
    }
}

int
proc_section (char *name, unsigned char type, void *arg, __g_eds eds)
{
  ear *iarg = (ear*) arg;
  int r;
  uint8_t ps_f = 0;

  if (!reg_match ("(\\/|^)[.]{1,2}", name, 0))
    {
      return 1;
    }
  /*
   if (!reg_match("\\/[.].*$", name, REG_NEWLINE))
   {
   return 1;
   }
   */

  switch (type)
    {
    case DT_DIR:

      /*if (access(name, R_OK))
       {
       print_str("WARNING: %s: no read access\n", name);
       break;
       }*/

      iarg->depth--;

      if ((gfl & F_OPT_MINDEPTH) && eds->depth < min_depth)
	{
	  proc_sect_edec (name, proc_section, iarg, eds);
	  iarg->depth++;
	  return 1;
	}

      if (!iarg->depth || (gfl & F_OPT_DIR_FULL_REBUILD))
	{
	  if (gfl & F_OPT_UPDATE)
	    {
	      p_md_obj ptr;
	      df_func dff = (df_func) g_act_1.cb0;

	      if ((ptr = dff (&g_act_1, 0, name)) != NULL)
		{
		  if (gfl & F_OPT_VERBOSE2)
		    {
		      print_str (
			  "WARNING: %s: record already exists, not importing\n",
			  name);
		    }

		  if (ptr != (p_md_obj) -1)
		    {
		      if ( NULL != g_act_1.ht_ref)
			{
			  ht_remove (g_act_1.ht_ref, (unsigned char*) name,
				     strlen (name));
			}
		      md_unlink (&g_act_1.buffer, ptr);
		    }

		  goto end;
		}
	    }

	  bzero (iarg->buffer, PATH_MAX);
	  iarg->flags = 0;
	  if ((r = release_generate_block (name, iarg)))
	    {
	      if (r == 12)
		{
		  ps_f |= F_PS_STOP_TRAVERSE;
		}
	      else
		{
		  if (r < 5)
		    {
		      print_str (
			  "ERROR: %s: [%d] generating dirlog data chunk failed\n",
			  name, r);
		    }
		  goto end;
		}
	    }

	  if (gfl & F_OPT_KILL_GLOBAL)
	    {
	      if ((gfl & F_OPT_SFV) && (iarg->flags & F_EARG_SFV)
		  && !(gfl & F_OPT_NOWRITE))
		{
		  if (remove (iarg->buffer))
		    {
		      print_str ( MSG_PS_UWSFV, iarg->buffer);
		    }
		}
	      goto end;
	    }

	  if (g_bmatch (iarg->dirlog, &g_act_1, &g_act_1.buffer))
	    {
	      if ((gfl & F_OPT_SFV) && (iarg->flags & F_EARG_SFV)
		  && !(gfl & F_OPT_NOWRITE))
		{
		  if (remove (iarg->buffer))
		    {
		      print_str ( MSG_PS_UWSFV, iarg->buffer);
		    }
		}
	      if (g_act_1.flags & F_GH_TFD_PROCED)
		{
		  g_act_1.flags ^= F_GH_TFD_PROCED;
		  goto end_skproc;
		}
	      else
		{
		  goto end;
		}

	    }

	  if ((gfl & F_OPT_SFV) && (iarg->flags & F_EARG_SFV)
	      && !(gfl & F_OPT_NOWRITE))
	    {
	      iarg->dirlog->bytes += (uint64_t) get_file_size (iarg->buffer);
	      iarg->dirlog->files++;
	      if (!rename (iarg->buffer, iarg->buffer2))
		{
		  if (gfl & F_OPT_VERBOSE)
		    {
		      print_str (
			  "NOTICE: '%s': succesfully generated SFV file\n",
			  name);
		    }
		}
	      else
		{
		  print_str ("ERROR: failed renaming '%s' to '%s'\n",
			     g_basename (iarg->buffer2),
			     g_basename (iarg->buffer));
		  if (remove (iarg->buffer2))
		    {
		      print_str ( MSG_PS_UWSFV, iarg->buffer);
		    }
		}
	    }

	  if (g_act_1.flags & F_GH_FFBUFFER)
	    {
	      if ((r = g_load_record (&g_act_1, &g_act_1.w_buffer,
				      (const void*) iarg->dirlog,
				      MAX_BWHOLD_BYTES_DL / DL_SZ,
				      F_LOAD_RECORD_ALL)))
		{
		  print_str (MSG_GEN_DFWRITE, iarg->dirlog->dirname, r,
			     (ulint64_t) g_act_1.w_buffer.offset, "wbuffer");
		}
	    }
	  else
	    {
	      if ((r = dirlog_write_record (iarg->dirlog, 0, SEEK_END)))
		{
		  print_str (MSG_GEN_DFWRITE, iarg->dirlog->dirname, r,
			     (ulint64_t) g_act_1.offset - 1, "w");
		  goto end;
		}
	    }

	  if ((gfl & F_OPT_VERBOSE))
	    {
	      dirlog_format_block (iarg->dirlog, NULL);
	    }

	  end: ;

	  if ((gfl & F_OPT_DIR_FULL_REBUILD) && !(ps_f & F_PS_STOP_TRAVERSE))
	    {
	      proc_sect_edec (name, proc_section, iarg, eds);
	    }

	  end_skproc: ;
	}
      else
	{
	  proc_sect_edec (name, proc_section, iarg, eds);
	}

      iarg->depth++;
      return 0;
    }
  return 1;
}

int
release_generate_block (char *name, ear *iarg)
{
  bzero (iarg->dirlog, sizeof(struct dirlog));

  int r, ret = 0;
  struct stat st =
    { 0 }, st2 =
    { 0 };

  if (gfl & F_OPT_FOLLOW_LINKS)
    {
      if (stat (name, &st))
	{
	  return 1;
	}
    }
  else
    {
      if (lstat (name, &st))
	{
	  return 1;
	}

      if ((st.st_mode & S_IFMT) == S_IFLNK)
	{
	  if (gfl & F_OPT_VERBOSE2)
	    {
	      print_str ("WARNING: %s - is symbolic link, skipping..\n", name);
	    }
	  return 6;
	}
    }

  time_t orig_ctime = get_file_creation_time (&st);

  if ((gfl & F_OPT_SFV) && (!(gfl & F_OPT_NOWRITE) || (gfl & F_OPT_FORCEWSFV)))
    {
      if ((r = enum_dir (name, delete_file, (void*) "\\.sfv(\\.tmp|)$", 0, NULL,
			 tp_default)) < 0)
	{
	  print_str (
	      "ERROR: release_generate_block->enum_dir->delete_file (clean SFV): %s: [%d] %s\n",
	      name, r, ie_tl (r, EMR_enum_dir));
	}
    }

  if (((r = enum_dir (name, proc_directory, iarg, 0, iarg->eds, tp_default)) < 1
      || !(iarg->dirlog->files)))
    {
      if (r < 0)
	{
	  print_str (
	      "ERROR: release_generate_block->enum_dir->proc_directory: %s: [%d] %s\n",
	      name, r, ie_tl (r, EMR_enum_dir));
	  ret = 12;
	}
      else
	{
	  if (gfl & F_OPT_VERBOSE2)
	    {
	      print_str ("WARNING: %s: [%d] - empty directory\n", name, r);
	    }
	}
    }

  g_setjmp (0, "release_generate_block(2)", NULL, NULL);

  if ((gfl & F_OPT_SFV) && !(gfl & F_OPT_NOWRITE))
    {
      if (gfl & F_OPT_FOLLOW_LINKS)
	{
	  if (stat (name, &st2))
	    {
	      return 1;
	    }
	}
      else
	{
	  if (lstat (name, &st2))
	    {
	      return 1;
	    }
	}
      time_t c_ctime = get_file_creation_time (&st2);
      if (c_ctime != orig_ctime)
	{
	  if (gfl & F_OPT_VERBOSE4)
	    {
	      print_str (
		  "NOTICE: %s: restoring original folder modification date\n",
		  name);
	    }
	  struct utimbuf utb;
	  utb.actime = 0;
	  utb.modtime = orig_ctime;
	  if (utime (name, &utb))
	    {
	      print_str (
		  "WARNING: %s: SFVGEN failed to restore original modification date\n",
		  name);
	    }
	}

    }

  char *bn = g_basename (name);

  iarg->dirlog->uptime = orig_ctime;
  iarg->dirlog->uploader = (uint16_t) st.st_uid;
  iarg->dirlog->group = (uint16_t) st.st_gid;
  char buffer[PATH_MAX] =
    { 0 };

  if ((r = get_relative_path (name, GLROOT, buffer)))
    {
      print_str ("ERROR: [%s] could not get relative to root directory name\n",
		 bn);
      return 2;
    }

  struct nukelog n_buffer =
    { 0 };

  if (!(gfl0 & F_OPT_NO_CHECK_NUKED))
    {
      if (nukelog_find (buffer, 2, &n_buffer) < MAX_uint64_t)
	{
	  iarg->dirlog->status = n_buffer.status + 1;
	  strcp_s (iarg->dirlog->dirname, sizeof(iarg->dirlog->dirname),
		   n_buffer.dirname);

	  return ret;

	}
      else
	{
	  if (gfl0 & F_OPT_NO_CHECK_NUKED)
	    {
	      print_str (
		  "WARNING: will not be able to determine directory nuke status during operation\n");
	    }
	}
    }

  strcp_s (iarg->dirlog->dirname, sizeof(iarg->dirlog->dirname), buffer);

  return ret;
}

uint64_t
dirlog_find (char *dirn, int mode, uint32_t flags, void *callback)
{
  if (!(ofl & F_OVRR_NUKESTR))
    {
      return dirlog_find_old (dirn, mode, flags, callback);
    }

  int
  (*callback_f) (struct dirlog *) = callback;

  if (g_fopen (DIRLOG, "r", F_DL_FOPEN_BUFFER | flags, &g_act_1))
    {
      return MAX_uint64_t;
    }

  struct dirlog buffer;

  int r;
  uint64_t ur = MAX_uint64_t;

  char buffer_s[PATH_MAX] =
    { 0 }, buffer_s2[PATH_MAX], buffer_s3[PATH_MAX];
  char *dup2, *base, *dir;

  if ((r = get_relative_path (dirn, GLROOT, buffer_s)))
    {
      snprintf (buffer_s, PATH_MAX, "%s", dirn);
    }

  size_t d_l = strlen (buffer_s);

  struct dirlog *d_ptr = NULL;

  while ((d_ptr = (struct dirlog *) g_read (&buffer, &g_act_1, DL_SZ)))
    {
      if (!strncmp (buffer_s, d_ptr->dirname, d_l))
	{
	  goto match;
	}
      base = g_basename (d_ptr->dirname);
      dup2 = strdup (d_ptr->dirname);
      dir = dirname (dup2);
      snprintf (buffer_s2, PATH_MAX, NUKESTR, base);
      snprintf (buffer_s3, PATH_MAX, "%s/%s", dir, buffer_s2);
      remove_repeating_chars (buffer_s3, 0x2F);

      free (dup2);
      if (!strncmp (buffer_s3, buffer_s, d_l))
	{
	  match: ur = g_act_1.offset - 1;
	  if (mode == 2 && callback)
	    {
	      if (callback_f (&buffer))
		{
		  break;
		}

	    }
	  else
	    {
	      break;
	    }
	}
    }

  if (mode != 1)
    {
      g_close (&g_act_1);
      g_cleanup (&g_act_1);
    }

  return ur;
}

uint64_t
dirlog_find_old (char *dirn, int mode, uint32_t flags, void *callback)
{
  int
  (*callback_f) (struct dirlog *data) = callback;

  if (g_fopen (DIRLOG, "r", F_DL_FOPEN_BUFFER | flags, &g_act_1))
    {
      return MAX_uint64_t;
    }

  struct dirlog buffer;

  int r;
  uint64_t ur = MAX_uint64_t;

  char buffer_s[PATH_MAX] =
    { 0 };
  char *dup2, *base, *dir;
  int gi1, gi2;

  if ((r = get_relative_path (dirn, GLROOT, buffer_s)))
    {
      strncpy (buffer_s, dirn, strlen (dirn));
    }

  gi2 = strlen (buffer_s);

  struct dirlog *d_ptr = NULL;

  while ((d_ptr = (struct dirlog *) g_read (&buffer, &g_act_1, DL_SZ)))
    {

      base = g_basename (d_ptr->dirname);
      gi1 = strlen (base);
      dup2 = strdup (d_ptr->dirname);
      dir = dirname (dup2);
      if (!strncmp (&buffer_s[gi2 - gi1], base, gi1)
	  && !strncmp (buffer_s, d_ptr->dirname, strlen (dir)))
	{

	  ur = g_act_1.offset - 1;
	  if (mode == 2 && callback)
	    {
	      if (callback_f (&buffer))
		{

		  free (dup2);
		  break;
		}

	    }
	  else
	    {

	      free (dup2);
	      break;
	    }
	}

      free (dup2);
    }

  if (mode != 1)
    {
      g_close (&g_act_1);
      g_cleanup (&g_act_1);
    }

  return ur;
}

p_md_obj
dirlog_find_a (void *f, uint32_t flags, void *dirn)
{
  __g_handle hdl = (__g_handle) f;

  if (g_fopen (DIRLOG, "r", F_DL_FOPEN_BUFFER | flags, hdl))
    {
      return NULL;
    }

  struct dirlog buffer;

  char *s_ptr;

  s_ptr = get_relative_path_n ((char*) dirn, GLROOT);

  struct dirlog *d_ptr = NULL;
  size_t r_l = strlen (s_ptr);

  while ((d_ptr = (struct dirlog *) g_read (&buffer, hdl, DL_SZ)))
    {
      size_t l_l = strlen (d_ptr->dirname);

      if (l_l == r_l && !strncmp (s_ptr, d_ptr->dirname, r_l))
	{
	  if (hdl->buffer.count)
	    {
	      return hdl->buffer.pos;
	    }
	  else
	    {
	      return (p_md_obj) -1;
	    }
	}
    }

  return NULL;
}

p_md_obj
dirlog_find_ht (void *f, uint32_t flags, void *path)
{
  __g_handle hdl = (__g_handle) f;

  path = get_relative_path_n ((char*) path, GLROOT);

  size_t klen = strlen (path);

  if (0 == klen)
    {
      return NULL;
    }

  pmda shell = ht_get (hdl->ht_ref, (unsigned char*) path, klen + 1);

  if (NULL == shell || shell->offset == 0)
    {
      return NULL;
    }

  p_md_obj ptr = md_first (shell);

  if ( NULL == ptr)
    {
      return NULL;
    }

  return (p_md_obj) ptr->ptr;
}

uint64_t
nukelog_find (char *dirn, int mode, struct nukelog *output)
{
  struct nukelog buffer =
    { 0 };

  uint64_t r = MAX_uint64_t;
  char *dup2, *base, *dir;

  if (g_fopen (NUKELOG, "r", F_DL_FOPEN_BUFFER, &g_act_2))
    {
      gfl0 |= F_OPT_NO_CHECK_NUKED;
      goto r_end;
    }

  int gi1, gi2;
  gi2 = strlen (dirn);

  struct nukelog *n_ptr = NULL;

  while ((n_ptr = (struct nukelog *) g_read (&buffer, &g_act_2, NL_SZ)))
    {

      base = g_basename (n_ptr->dirname);
      gi1 = strlen (base);
      dup2 = strdup (n_ptr->dirname);
      dir = dirname (dup2);

      if (gi1 >= gi2 || gi1 < 2)
	{
	  goto l_end;
	}

      if (!strncmp (&dirn[gi2 - gi1], base, gi1)
	  && !strncmp (dirn, n_ptr->dirname, strlen (dir)))
	{
	  if (output)
	    {
	      memcpy (output, n_ptr, NL_SZ);
	    }
	  r = g_act_2.offset - 1;
	  if (mode != 2)
	    {

	      free (dup2);
	      break;
	    }
	}
      l_end:

      free (dup2);
    }

  if (mode != 1)
    {
      g_close (&g_act_2);
    }

  r_end:

  return r;
}

#define DWR_DO_NORM_WRITE(buffer, fh) { \
    if ((fw = fwrite(buffer, 1, DL_SZ, fh)) < DL_SZ) \
        { \
          print_str("ERROR: could not write dirlog record! %d/%d\n", fw, \
              (int) DL_SZ); \
          return 1; \
        } \
}

int
dirlog_write_record (struct dirlog *buffer, off_t offset, int whence)
{
  g_setjmp (0, "dirlog_write_record", NULL, NULL);
  if (gfl & F_OPT_NOWRITE)
    {
      return 0;
    }

  if (!buffer)
    {
      return 2;
    }

  if (!g_act_1.fh)
    {
      print_str ("ERROR: dirlog handle is not open\n");
      return 1;
    }

  if (whence == SEEK_SET && fseek (g_act_1.fh, offset * DL_SZ, SEEK_SET) < 0)
    {
      print_str ("ERROR: seeking dirlog failed!\n");
      return 1;
    }

  int fw;

#ifdef HAVE_ZLIB_H
  if (g_act_1.flags & F_GH_IO_GZIP)
    {
      if ((fw = gzwrite(g_act_1.gz_fh, buffer, DL_SZ)) < DL_SZ)
	{
	  print_str("ERROR: could not write dirlog record! %d/%d\n", fw,
	      (int) DL_SZ);
	  return 1;
	}
    }
  else
    {
      DWR_DO_NORM_WRITE(buffer, g_act_1.fh)
    }
#else
  DWR_DO_NORM_WRITE(buffer, g_act_1.fh)
#endif

  g_act_1.bw += (off_t) fw;
  g_act_1.rw++;

  if (whence == SEEK_SET)
    g_act_1.offset = offset;
  else
    g_act_1.offset++;

  return 0;
}

int
get_relative_path (char *subject, char *root, char *output)
{
  char *root_dir = root;

  if (!root_dir)
    return 11;

  int i, root_dir_len = strlen (root_dir);

  for (i = 0; i < root_dir_len; i++)
    {
      if (subject[i] != root_dir[i])
	break;
    }

  while (subject[i] != 0x2F && i > 0)
    {
      i--;
    }

  snprintf (output, PATH_MAX, "%s", &subject[i]);
  return 0;
}

char*
get_relative_path_n (char *subject, char *root)
{
  char *o_s = subject;
  while (subject[0] && root[0] && subject[0] == root[0])
    {
      subject++;
      root++;
    }

  if (subject[0] != root[0] && root[0])
    {
      return o_s;
    }

  while (subject[-1] == 0x2F && subject[0])
    {
      subject--;
    }

  return subject;
}

int
dirlog_update_record (char *argv)
{
  g_setjmp (0, "dirlog_update_record", NULL, NULL);

  int r, seek = SEEK_END, ret = 0, dr;
  off_t offset = 0;
  uint64_t rl = MAX_uint64_t;
  struct dirlog dl =
    { 0 };
  ear arg =
    { 0 };
  arg.dirlog = &dl;

  if (gfl & F_OPT_SFV)
    {
      gfl |= F_OPT_NOWRITE | F_OPT_FORCE | F_OPT_FORCEWSFV;
    }

  mda dirchain =
    { 0 };
  p_md_obj ptr;

  md_init (&dirchain, 1024);

  if ((r = split_string (argv, 0x20, &dirchain)) < 1)
    {
      print_str ("ERROR: [dirlog_update_record]: missing arguments\n");
      ret = 1;
      goto r_end;
    }

  data_backup_records (DIRLOG);

  char s_buffer[PATH_MAX];
  ptr = dirchain.objects;
  while (ptr)
    {
      snprintf (s_buffer, PATH_MAX, "%s/%s", SITEROOT, (char*) ptr->ptr);
      remove_repeating_chars (s_buffer, 0x2F);
      size_t s_buf_len = strlen (s_buffer);
      if (s_buffer[s_buf_len - 1] == 0x2F)
	{
	  s_buffer[s_buf_len - 1] = 0x0;
	}

      rl = dirlog_find (s_buffer, 0, 0, NULL);

      char *mode = "a";

      if (!(gfl & F_OPT_FORCE) && rl < MAX_uint64_t)
	{
	  print_str (
	      "WARNING: %s: [%llu] already exists in dirlog (use -f to overwrite)\n",
	      (char*) ptr->ptr, rl);
	  ret = 4;
	  goto end;
	}
      else if (rl < MAX_uint64_t)
	{
	  if (gfl & F_OPT_VERBOSE)
	    {
	      print_str (
		  "WARNING: %s: [%llu] overwriting existing dirlog record\n",
		  (char*) ptr->ptr, rl);
	    }
	  offset = rl;
	  seek = SEEK_SET;
	  mode = "r+";
	}

      if ((r = g_fopen (DIRLOG, mode, 0, &g_act_1)))
	{
	  print_str ("ERROR: %s: could not open log [%d], mode %s\n", DIRLOG, r,
		     mode);
	  goto r_end;
	}

      if ((r = release_generate_block (s_buffer, &arg)))
	{
	  if (r < 5)
	    {
	      print_str (
		  "ERROR: %s: [%d] generating dirlog data chunk failed\n",
		  (char*) ptr->ptr, r);
	    }
	  ret = 3;
	  goto end;
	}

      if ((dr = dirlog_write_record (arg.dirlog, offset, seek)))
	{
	  print_str (
	  MSG_GEN_DFWRITE,
		     (char*) ptr->ptr, dr, (ulint64_t) offset, mode);
	  ret = 6;
	  goto end;
	}

      dirlog_format_block (arg.dirlog, NULL);

      end:

      g_close (&g_act_1);
      ptr = ptr->next;
    }
  r_end:

  md_free (&dirchain);

  if (((gfl0 & F_OPT_STATS) || (gfl & F_OPT_VERBOSE4))
      && !(gfl0 & F_OPT_NOSTATS))
    {
#ifdef HAVE_ZLIB_H
      if (g_act_1.flags & F_GH_IO_GZIP)
	{
	  fprintf(stderr, MSG_GEN_WROTE2, DIRLOG, (double) dl_stats.bw / 1024.0,
	      (double) g_act_1.bw / 1024.0, (long long unsigned) dl_stats.rw);
	}
      else
	{
	  OPLOG_OUTPUT_NSTATS(g_act_1.file, dl_stats)
	}
#else
      OPLOG_OUTPUT_NSTATS(g_act_1.file, dl_stats)
#endif
    }

  return ret;
}

int
dirlog_check_records (void)
{
  g_setjmp (0, "dirlog_check_records", NULL, NULL);
  struct dirlog buffer, buffer4;
  ear buffer3 =
    { 0 };
  char s_buffer[PATH_MAX], s_buffer2[PATH_MAX], s_buffer3[PATH_MAX] =
    { 0 };
  buffer3.dirlog = &buffer4;
  int r = 0, r2;
  char *mode = "r";
  uint32_t flags = 0;
  off_t dsz;

  if (!(gfl0 & F_OPT_GZIP))
    {
      if ((dsz = get_file_size (DIRLOG)) % DL_SZ)
	{
	  print_str (MSG_GEN_DFCORRU, DIRLOG, (ulint64_t) dsz, (int) DL_SZ);
	  print_str ("NOTICE: use -r to rebuild (see --help)\n");
	  return -1;
	}
    }

  if (g_fopen (DIRLOG, mode, F_DL_FOPEN_BUFFER | flags, &g_act_1))
    {
      return 2;
    }

  if (!g_act_1.buffer.count && (gfl & F_OPT_FIX))
    {
      print_str (
	  "ERROR: internal buffering must be enabled when fixing, increase limit with --memlimit (see --help)\n");
    }

  struct dirlog *d_ptr = NULL;
  int ir;

  while ((d_ptr = (struct dirlog *) g_read (&buffer, &g_act_1, DL_SZ)))
    {
      if (!sigsetjmp(g_sigjmp.env, 1))
	{
	  g_setjmp (F_SIGERR_CONTINUE, "dirlog_check_records(loop)",
	  NULL,
		    NULL);

	  if (gfl & F_OPT_KILL_GLOBAL)
	    {
	      break;
	    }
	  snprintf (s_buffer, PATH_MAX, "%s/%s", GLROOT, d_ptr->dirname);
	  remove_repeating_chars (s_buffer, 0x2F);

	  if (d_ptr->status == 1)
	    {
	      char *c_nb, *base, *c_nd, *dir;
	      c_nb = strdup (d_ptr->dirname);
	      base = basename (c_nb);
	      c_nd = strdup (d_ptr->dirname);
	      dir = dirname (c_nd);

	      snprintf (s_buffer2, PATH_MAX, NUKESTR, base);
	      snprintf (s_buffer3, PATH_MAX, "%s/%s/%s", GLROOT, dir,
			s_buffer2);
	      remove_repeating_chars (s_buffer3, 0x2F);
	      free (c_nb);
	      free (c_nd);
	    }

	  if ((d_ptr->status != 1 && dir_exists (s_buffer))
	      || (d_ptr->status == 1 && dir_exists (s_buffer3)))
	    {
	      print_str (
		  "WARNING: %s: listed in dirlog but does not exist on filesystem\n",
		  s_buffer);
	      if (gfl & F_OPT_FIX)
		{
		  if (!md_unlink (&g_act_1.buffer, g_act_1.buffer.pos))
		    {
		      print_str ("ERROR: %s: unlinking ghost record failed\n",
				 s_buffer);
		    }
		  r++;
		}
	      continue;
	    }

	  if (gfl & F_OPT_C_GHOSTONLY)
	    {
	      continue;
	    }

	  struct nukelog n_buffer;
	  ir = r;
	  if (d_ptr->status == 1 || d_ptr->status == 2)
	    {
	      if (nukelog_find (d_ptr->dirname, 2, &n_buffer) == MAX_uint64_t)
		{
		  print_str (
		      "WARNING: %s: was marked as '%sNUKED' in dirlog but not found in nukelog\n",
		      s_buffer, d_ptr->status == 2 ? "UN" : "");
		}
	      else
		{
		  if ((d_ptr->status == 1 && n_buffer.status != 0)
		      || (d_ptr->status == 2 && n_buffer.status != 1)
		      || (d_ptr->status == 0))
		    {
		      print_str (
			  "WARNING: %s: MISMATCH: was marked as '%sNUKED' in dirlog, but nukelog reads '%sNUKED'\n",
			  s_buffer, d_ptr->status == 2 ? "UN" : "",
			  n_buffer.status == 1 ? "UN" : "");
		    }
		}
	      continue;
	    }

	  buffer3.flags |= F_EAR_NOVERB;

	  if ((r2 = release_generate_block (s_buffer, &buffer3)))
	    {
	      if (r2 == 5)
		{
		  if ((gfl & F_OPT_FIX) && (gfl & F_OPT_FORCE))
		    {
		      if (remove (s_buffer))
			{
			  print_str (
			      "WARNING: %s: failed removing empty directory\n",
			      s_buffer);

			}
		      else
			{
			  if (gfl & F_OPT_VERBOSE)
			    {
			      print_str ("FIX: %s: removed empty directory\n",
					 s_buffer);
			    }
			}
		    }
		}
	      else
		{
		  if (r2 < 5)
		    {
		      print_str (
			  "WARNING: %s: [%d] - could not get directory information from the filesystem\n",
			  s_buffer, r2);
		    }
		}
	      r++;
	      continue;
	    }

	  if (d_ptr->files != buffer4.files)
	    {
	      print_str (
		  "WARNING: [%s] file counts in dirlog and on disk do not match (dirlog:%hu, filesystem:%hu)\n",
		  d_ptr->dirname, d_ptr->files, buffer4.files);
	      r++;
	      if (gfl & F_OPT_FIX)
		{
		  d_ptr->files = buffer4.files;
		}
	    }

	  if (d_ptr->bytes != buffer4.bytes)
	    {
	      print_str (
		  "WARNING: [%s] directory sizes in dirlog and on disk do not match ( dirlog: %llu , filesystem: %llu )\n",
		  d_ptr->dirname, (ulint64_t) d_ptr->bytes,
		  (ulint64_t) buffer4.bytes);
	      r++;
	      if (gfl & F_OPT_FIX)
		{
		  d_ptr->bytes = buffer4.bytes;
		}
	    }

	  if (d_ptr->group != buffer4.group)
	    {
	      print_str (
		  "WARNING: [%s] group ids in dirlog and on disk do not match (dirlog:%hu filesystem:%hu)\n",
		  d_ptr->dirname, d_ptr->group, buffer4.group);
	      r++;
	      if (gfl & F_OPT_FIX)
		{
		  d_ptr->group = buffer4.group;
		}
	    }

	  if (d_ptr->uploader != buffer4.uploader)
	    {
	      print_str (
		  "WARNING: [%s] user ids in dirlog and on disk do not match (dirlog:%hu, filesystem:%hu)\n",
		  d_ptr->dirname, d_ptr->uploader, buffer4.uploader);
	      r++;
	      if (gfl & F_OPT_FIX)
		{
		  d_ptr->uploader = buffer4.uploader;
		}
	    }

	  if ((gfl & F_OPT_FORCE) && d_ptr->uptime != buffer4.uptime)
	    {
	      print_str (
		  "WARNING: [%s] folder creation dates in dirlog and on disk do not match (dirlog:%u, filesystem:%u)\n",
		  d_ptr->dirname, d_ptr->uptime, buffer4.uptime);
	      r++;
	      if (gfl & F_OPT_FIX)
		{
		  d_ptr->uptime = buffer4.uptime;
		}
	    }
	  if (r == ir)
	    {
	      if (gfl & F_OPT_VERBOSE2)
		{
		  print_str ("OK: %s\n", d_ptr->dirname);
		}
	    }
	  else
	    {
	      if (gfl & F_OPT_VERBOSE2)
		{
		  print_str ("BAD: %s\n", d_ptr->dirname);
		}

	    }
	}

    }

  if (!(gfl & F_OPT_KILL_GLOBAL) && (gfl & F_OPT_FIX) && r)
    {
      if (rebuild_data_file (DIRLOG, &g_act_1))
	{
	  print_str (MSG_GEN_DFRFAIL, DIRLOG);
	}
      else
	{
	  if (((gfl0 & F_OPT_STATS) || (gfl & F_OPT_VERBOSE4))
	      && !(gfl0 & F_OPT_NOSTATS))
	    {
#ifdef HAVE_ZLIB_H
	      if (g_act_1.flags & F_GH_IO_GZIP)
		{
		  fprintf(stderr, MSG_GEN_WROTE2, DIRLOG,
		      (double) dl_stats.bw / 1024.0,
		      (double) g_act_1.bw / 1024.0,
		      (long long unsigned int) dl_stats.rw);
		}
	      else
		{
		  OPLOG_OUTPUT_NSTATS(g_act_1.file, g_act_1)
		}
#else
	      OPLOG_OUTPUT_NSTATS(g_act_1.file, g_act_1)
#endif
	    }
	}
    }

  g_close (&g_act_1);

  return r;
}

void
g_progress_stats (time_t s_t, time_t e_t, off_t total, off_t done)
{
  register float diff = (float) (e_t - s_t);
  register float rate = ((float) done / diff);

  fprintf (stderr,
	   "PROCESSING: %llu/%llu [ %.2f%s ] | %.2f r/s | ETA: %.1f s\r",
	   (long long unsigned int) done, (long long unsigned int) total,
	   ((float) done / ((float) total / 100.0)), "%", rate,
	   (float) (total - done) / rate);

}

int
dirlog_check_dupe (void)
{
  g_setjmp (0, "dirlog_check_dupe", NULL, NULL);
  struct dirlog buffer, buffer2;
  struct dirlog *d_ptr = NULL, *dd_ptr = NULL;
  char *s_pb, *ss_pb;

  if (g_fopen (DIRLOG, "r", F_DL_FOPEN_BUFFER, &g_act_1))
    {
      return 2;
    }
  off_t st1, st2 = 0, st3 = 0;
  p_md_obj pmd_st1 = NULL, pmd_st2 = NULL;
  g_setjmp (0, "dirlog_check_dupe(loop)", NULL, NULL);
  time_t s_t = time (NULL), e_t = time (NULL), d_t = time (NULL);

  off_t nrec = g_act_1.total_sz / g_act_1.block_sz;

  if (g_act_1.buffer.count)
    {
      nrec = g_act_1.buffer.count;
    }

  if (gfl & F_OPT_VERBOSE)
    {
      g_progress_stats (s_t, e_t, nrec, st3);
    }
  off_t rtt;
  while ((d_ptr = (struct dirlog *) g_read (&buffer, &g_act_1, g_act_1.block_sz)))
    {
      st3++;
      if (gfl & F_OPT_KILL_GLOBAL)
	{
	  break;
	}

      if (g_bmatch (d_ptr, &g_act_1, &g_act_1.buffer))
	{
	  continue;
	}

      rtt = s_string_r (d_ptr->dirname, "/");
      s_pb = &d_ptr->dirname[rtt + 1];
      size_t s_pb_l = strlen (s_pb);

      if (s_pb_l < 4)
	{
	  continue;
	}

      if (gfl & F_OPT_VERBOSE)
	{
	  e_t = time (NULL);

	  if (e_t - d_t)
	    {
	      d_t = time (NULL);
	      g_progress_stats (s_t, e_t, nrec, st3);
	    }
	}

      st1 = g_act_1.offset;

      if (!g_act_1.buffer.count)
	{
	  st2 = (off_t) ftell (g_act_1.fh);
	}
      else
	{

	  pmd_st1 = g_act_1.buffer.r_pos;
	  pmd_st2 = g_act_1.buffer.pos;
	}

      int ch = 0;

      while ((dd_ptr = (struct dirlog *) g_read (&buffer2, &g_act_1,
						 g_act_1.block_sz)))
	{
	  rtt = s_string_r (dd_ptr->dirname, "/");
	  ss_pb = &dd_ptr->dirname[rtt + 1];
	  size_t ss_pb_l = strlen (ss_pb);

	  if (ss_pb_l == s_pb_l && !strncmp (s_pb, ss_pb, s_pb_l))
	    {
	      if (!ch)
		{
		  printf ("\r%s               \n", d_ptr->dirname);
		  ch++;
		}
	      printf ("\r%s               \n", dd_ptr->dirname);
	      if (gfl & F_OPT_VERBOSE)
		{
		  e_t = time (NULL);
		  g_progress_stats (s_t, e_t, nrec, st3);
		}
	    }
	}

      g_act_1.offset = st1;
      if (!g_act_1.buffer.count)
	{
	  fseek (g_act_1.fh, (off_t) st2, SEEK_SET);
	}
      else
	{
	  g_act_1.buffer.r_pos = pmd_st1;
	  g_act_1.buffer.pos = pmd_st2;
	}

    }

  if ((gfl & F_OPT_VERBOSE) && !(gfl0 & F_OPT_NOSTATS))
    {
      d_t = time (NULL);
      g_progress_stats (s_t, e_t, nrec, st3);
      print_str ("\nSTATS: processed %llu/%llu records\n", st3, nrec);
    }
  return 0;
}

