/*
 * main.c
 *
 *  Created on: Dec 5, 2013
 *      Author: reboot
 */



#include <glutil.h>

#include <l_sb.h>
#include <t_glob.h>
#include <l_error.h>
#include <cfgv.h>
#include <g_modes.h>
#include <macros_t.h>
#include <arg_proc.h>
#include <misc.h>
#include <signal_t.h>
#include <lref_gen.h>
#include <log_op.h>
#include <arg_opts.h>
#include <dirlog.h>
#include <log_io.h>
#include <exec_t.h>
#include <m_general.h>
#include <sort_hdr.h>
#include <misc.h>
#include <str.h>
#include <xref.h>
#include <x_f.h>
#include <omfp.h>


#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

int
(*print_str)(const char * volatile buf, ...);

int
main(int argc, char *argv[])
{
  char **p_argv = (char**) argv;
  int r;

  print_str = g_print_str;

  g_setjmp(0, "main", NULL, NULL);
  if ((r = setup_sighandlers()))
    {
      print_str(
          "WARNING: UNABLE TO SETUP SIGNAL HANDLERS! (this is weird, please report it!) [%d]\n",
          r);
      sleep(5);
    }

  _p_macro_argc = argc;

  if ((r = parse_args(argc, argv, prio_f_ref)) > 0)
    {
      print_str(MSG_INIT_CMDLINE_ERROR, r);
      EXITVAL = 2;
      g_shutdown(NULL);
    }

  enable_logging();

  switch (updmode)
    {
  case PRIO_UPD_MODE_MACRO:
    ;
    uint64_t gfl_s = (gfl & (F_OPT_WBUFFER | F_OPT_PS_LOGGING | F_OPT_NOGLCONF));
    char **ptr;
    ptr = process_macro(prio_argv_off, NULL);
    if (ptr)
      {
        _p_macro_argv = p_argv = ptr;
        gfl = gfl_s;
      }
    else
      {
        g_shutdown(NULL);
      }
    break;
  case PRIO_UPD_MODE_INFO:
    g_print_info();
    g_shutdown(NULL);
    break;
    }

  updmode = 0;

  g_init(_p_macro_argc, p_argv);

  g_shutdown(NULL);

  return EXITVAL;
}


int
g_init(int argc, char **argv)
{
  g_setjmp(0, "g_init", NULL, NULL);
  int r;

  if (strlen(LOGFILE))
    {
      gfl |= F_OPT_PS_LOGGING;
    }
  r = parse_args(argc, argv, f_ref);

  if (r == -2 || r == -1)
    {
      print_version_long(NULL, 0);
      print_str("Use --help for command info\n");
      EXITVAL = 4;
      return EXITVAL;
    }

  if (r > 0)
    {
      print_str(MSG_INIT_CMDLINE_ERROR, r);
      EXITVAL = 2;
      return EXITVAL;
    }

  enable_logging();

  if (ofl & F_ESREDIRFAILED)
    {
      print_str("ERROR: could not open file to redirect execv stdout to\n");
      EXITVAL = 2;
      return EXITVAL;
    }

  if (updmode && updmode != UPD_MODE_NOOP && !(gfl & F_OPT_FORMAT_BATCH)
      && !(gfl & F_OPT_FORMAT_COMP) && (gfl & F_OPT_VERBOSE2))
    {
      print_str("INIT: %s %s-%s starting [PID: %d]\n",
      PACKAGE_NAME, PACKAGE_VERSION, __STR_ARCH, getpid());
    }

  if (!(gfl & F_OPT_NOGLCONF))
    {
      if (strlen(GLCONF_I))
        {
          if ((r = load_cfg(&glconf, GLCONF_I, F_LCONF_NORF, NULL))
              && (gfl & F_OPT_VERBOSE))
            {
              print_str("WARNING: %s: could not load GLCONF file [%d]\n",
                  GLCONF_I, r);
            }

          if ((gfl & F_OPT_VERBOSE4) && glconf.offset)
            {
              print_str("NOTICE: %s: loaded %d config lines into memory\n",
                  GLCONF_I, (int) glconf.offset);
            }

          p_md_obj ptr = get_cfg_opt("ipc_key", &glconf, NULL);

          if (ptr && !(ofl & F_OVRR_IPC))
            {
              SHM_IPC = (key_t) strtol(ptr->ptr, NULL, 16);
            }

          ptr = get_cfg_opt("rootpath", &glconf, NULL);

          if (ptr && !(ofl & F_OVRR_GLROOT))
            {
              snprintf(GLROOT, PATH_MAX, "%s", (char*) ptr->ptr);
              if ((gfl & F_OPT_VERBOSE5))
                {
                  print_str("NOTICE: GLCONF: using 'rootpath': %s\n", GLROOT);
                }
            }

          ptr = get_cfg_opt("min_homedir", &glconf, NULL);

          if (ptr && !(ofl & F_OVRR_SITEROOT))
            {
              snprintf(SITEROOT_N, PATH_MAX, "%s", (char*) ptr->ptr);
              if ((gfl & F_OPT_VERBOSE5))
                {
                  print_str("NOTICE: GLCONF: using 'min_homedir': %s\n",
                      SITEROOT_N);
                }
            }

          ptr = get_cfg_opt("ftp-data", &glconf, NULL);

          if (ptr)
            {
              snprintf(FTPDATA, PATH_MAX, "%s", (char*) ptr->ptr);
              if ((gfl & F_OPT_VERBOSE5))
                {
                  print_str("NOTICE: GLCONF: using 'ftp-data': %s\n", FTPDATA);
                }
            }

          ptr = get_cfg_opt("nukedir_style", &glconf, NULL);

          if (ptr)
            {
              NUKESTR = string_replace(ptr->ptr, "%N", "%s", NUKESTR, 255);
              if ((gfl & F_OPT_VERBOSE5))
                {
                  print_str("NOTICE: GLCONF: using 'nukedir_style': %s\n",
                      NUKESTR);
                }
              ofl |= F_OVRR_NUKESTR;
            }
        }
      else
        {
          print_str("WARNING: GLCONF not defined in glconf.h\n");
        }
    }

  if (!strlen(GLROOT))
    {
      print_str("ERROR: glftpd root directory not specified!\n");
      return 2;
    }

  if (!strlen(SITEROOT_N))
    {
      print_str("ERROR: glftpd site root directory not specified!\n");
      return 2;
    }

  snprintf(SITEROOT, PATH_MAX, "%s%s", GLROOT, SITEROOT_N);
  remove_repeating_chars(SITEROOT, 0x2F);

  if (dir_exists(SITEROOT) && !dir_exists(SITEROOT_N))
    {
      strcp_s(SITEROOT, PATH_MAX, SITEROOT_N);
    }

  if ((gfl & F_OPT_VERBOSE) && dir_exists(SITEROOT))
    {
      print_str("WARNING: no valid siteroot!\n");
    }

  if (!updmode && (gfl & F_OPT_SFV))
    {
      updmode = UPD_MODE_RECURSIVE;
      if (!(gfl & F_OPT_NOWRITE))
        {
          gfl |= F_OPT_FORCEWSFV | F_OPT_NOWRITE;
        }
      if (ofl & F_OVRR_GLROOT)
        {
          print_str(MSG_INIT_PATH_OVERR, "GLROOT", GLROOT);
        }

      if (ofl & F_OVRR_SITEROOT)
        {
          print_str(MSG_INIT_PATH_OVERR, "SITEROOT", SITEROOT);
        }
      if ((gfl & F_OPT_VERBOSE))
        {
          print_str(
              "NOTICE: switching to non-destructive filesystem rebuild mode\n");
        }
    }

  if ((gfl & F_OPT_VERBOSE))
    {
      if (gfl & F_OPT_NOBUFFER)
        {
          print_str("NOTICE: disabling memory buffering\n");
          if (gfl & F_OPT_SHAREDMEM)
            {
              print_str(
                  "WARNING: --shmem: shared memory segment buffering option is invalid when --nobuffer specified\n");
            }
        }
      if (SHM_IPC && SHM_IPC != shm_ipc)
        {
          print_str("NOTICE: IPC key set to '0x%.8X'\n", SHM_IPC);
        }

      if ((gfl & F_OPT_VERBOSE4) && (gfl & F_OPT_PS_LOGGING))
        {
          print_str("NOTICE: Logging enabled: %s\n", LOGFILE);
        }
    }

  if ((gfl & F_OPT_VERBOSE) && (gfl & F_OPT_NOWRITE))
    {
      print_str("WARNING: performing dry run, no writing will be done\n");
    }

  if (gfl & F_OPT_DAEMONIZE)
    {
      print_str("NOTICE: forking into background.. [PID: %d]\n", getpid());
      if (daemon(1, 0) == -1)
        {
          print_str(
              "ERROR: [%s] could not fork into background, terminating..\n",
              strerror(errno));
          EXITVAL = errno;
          return errno;
        }
    }

  if (updmode && (gfl & F_OPT_PREEXEC))
    {
      if (gfl & F_OPT_VERBOSE)
        {
          print_str("PREEXEC: running: '%s'\n", GLOBAL_PREEXEC);
        }
      int r_e = 0;
      if ((r_e = g_do_exec(NULL, ref_to_val_generic, GLOBAL_PREEXEC, NULL))
          == -1 || WEXITSTATUS(r_e))
        {
          if (gfl & F_OPT_VERBOSE5)
            {
              print_str("WARNING: [%d]: PREEXEC returned non-zero: '%s'\n",
                  WEXITSTATUS(r_e), GLOBAL_PREEXEC);
            }
          EXITVAL = WEXITSTATUS(r_e);
          return 1;
        }
    }

  if (g_usleep)
    {
      usleep(g_usleep);
    }
  else if (g_sleep)
    {
      sleep(g_sleep);
    }

  uint64_t mloop_c = 0;
  char m_b1[128];
  int m_f = 0x1;

  g_setjmp(0, "main(start)", NULL, NULL);

  enter:

  if ((m_f & 0x1))
    {
      snprintf(m_b1, 127, "main(loop) [c:%llu]",
          (long long unsigned int) mloop_c);
      g_setjmp(0, m_b1, NULL, NULL);
      m_f ^= 0x1;
    }

  switch (updmode)
    {
  case UPD_MODE_RECURSIVE:
    EXITVAL = rebuild_dirlog();
    break;
  case UPD_MODE_SINGLE:
    EXITVAL = dirlog_update_record(argv_off);
    break;
  case UPD_MODE_CHECK:
    EXITVAL = dirlog_check_records();
    break;
  case UPD_MODE_DUMP:
    EXITVAL = g_print_stats(DIRLOG, 0, 0);
    break;
  case UPD_MODE_DUMP_NUKE:
    EXITVAL = g_print_stats(NUKELOG, 0, 0);
    break;
  case UPD_MODE_DUMP_DUPEF:
    EXITVAL = g_print_stats(DUPEFILE, 0, 0);
    break;
  case UPD_MODE_DUMP_LON:
    EXITVAL = g_print_stats(LASTONLOG, 0, 0);
    break;
  case UPD_MODE_DUMP_ONEL:
    EXITVAL = g_print_stats(ONELINERS, 0, 0);
    break;
  case UPD_MODE_DUMP_IMDB:
    EXITVAL = g_print_stats(IMDBLOG, 0, 0);
    break;
  case UPD_MODE_DUMP_GAME:
    EXITVAL = g_print_stats(GAMELOG, 0, 0);
    break;
  case UPD_MODE_DUMP_TV:
    EXITVAL = g_print_stats(TVLOG, 0, 0);
    break;
  case UPD_MODE_DUMP_GENERIC:
    EXITVAL = d_gen_dump(p_argv_off);
    break;
  case UPD_MODE_DUPE_CHK:
    EXITVAL = dirlog_check_dupe();
    break;
  case UPD_MODE_REBUILD:
    EXITVAL = rebuild(p_argv_off);
    break;
  case UPD_MODE_DUMP_ONL:
    EXITVAL = g_print_stats("ONLINE USERS", F_DL_FOPEN_SHM, ON_SZ);
    break;
  case UPD_MODE_FORK:
    if (p_argv_off)
      {
        if ((EXITVAL = WEXITSTATUS(system(p_argv_off))))
          {
            if (gfl & F_OPT_VERBOSE)
              {
                print_str("WARNING: '%s': command failed, code %d\n",
                    p_argv_off, EXITVAL);
              }
          }
      }
    break;
  case UPD_MODE_BACKUP:
    EXITVAL = data_backup_records(g_dgetf(p_argv_off));
    break;
  case UPD_MODE_DUMP_USERS:
    EXITVAL = g_dump_ug(DEFPATH_USERS);
    break;
  case UPD_MODE_DUMP_GRPS:
    EXITVAL = g_dump_ug(DEFPATH_GROUPS);
    break;
  case UPD_MODE_DUMP_GEN:
    EXITVAL = g_dump_gen(p_argv_off);
    break;
  case UPD_MODE_WRITE:
    EXITVAL = d_write((char*) p_argv_off);
    break;
  case PRIO_UPD_MODE_INFO:
    g_print_info();
    break;
  case UPD_MODE_NOOP:
    break;
  default:
    print_help(NULL, -1);
    print_str("ERROR: no mode specified\n");
    break;
    }

  if ((gfl & F_OPT_LOOP) && !(gfl & F_OPT_KILL_GLOBAL)
      && (!loop_max || mloop_c < loop_max - 1))
    {
      g_cleanup(&g_act_1);
      g_cleanup(&g_act_2);
      free_cfg_rf(&cfg_rf);
      sleep(loop_interval);
      if (gfl & F_OPT_LOOPEXEC)
        {
          g_do_exec(NULL, ref_to_val_generic, LOOPEXEC, NULL);
        }
      mloop_c++;
      goto enter;
    }

  if (updmode && (gfl & F_OPT_POSTEXEC))
    {
      if (gfl & F_OPT_VERBOSE)
        {
          print_str("POSTEXEC: running: '%s'\n", GLOBAL_POSTEXEC);
        }
      if (g_do_exec(NULL, ref_to_val_generic, GLOBAL_POSTEXEC, NULL) == -1)
        {
          if (gfl & F_OPT_VERBOSE)
            {
              print_str("WARNING: POSTEXEC failed: '%s'\n", GLOBAL_POSTEXEC);
            }
        }
    }

  return EXITVAL;
}


int
g_shutdown(void *arg)
{
  g_setjmp(0, "g_shutdown", NULL, NULL);

  g_cleanup(&g_act_1);
  g_cleanup(&g_act_2);
  free_cfg_rf(&cfg_rf);
  free_cfg(&glconf);

  if ((gfl & F_OPT_PS_LOGGING) && fd_log)
    {
      fclose(fd_log);
    }

  if (_p_macro_argv)
    {
      free(_p_macro_argv);
    }

  if (GLOBAL_PREEXEC)
    {
      free(GLOBAL_PREEXEC);
    }

  if (GLOBAL_POSTEXEC)
    {
      free(GLOBAL_POSTEXEC);
    }

  if (LOOPEXEC)
    {
      free(LOOPEXEC);
    }

  if (exec_str)
    {
      free(exec_str);
    }

  if (exec_v)
    {
      int i;

      for (i = 0; i < exec_vc && exec_v[i]; i++)
        {
          free(exec_v[i]);
        }
      free(exec_v);

    }

  if (execv_stdout_redir != -1)
    {
      close(execv_stdout_redir);
    }

  md_g_free(&_match_rr);
  md_g_free(&_md_gsort);

  _p_macro_argc = 0;

  exit(EXITVAL);
}
