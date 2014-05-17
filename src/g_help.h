/*
 * g_help.h
 *
 *  Created on: Dec 5, 2013
 *      Author: reboot
 */

#ifndef G_HELP_H_
#define G_HELP_H_

char *hpd_up =
    "glFTPd binary logs utility, version %s-%s\n"
        "%s\n"
        "\n"
        "Main options:\n"
        "\n Output:\n"
        "  -d, [--raw] [--batch] [--export|-E] [-print] [ --r[u]sleep] [--stats] ..\n"
        "                        Read directory log and print to stdout (text/binary)\n"
        "                          (-vv prints dir nuke status from nukelog)\n"
        "                         --batch         Prints with simple formatting\n"
        "                         --export, -E    Prints in glutil export text format\n"
        "                         --raw           Prints raw binary data to stdout\n"
        "  -n, -||-              Read nuke log\n"
        "  -i, -||-              Read dupe file\n"
        "  -l, -||-              Read last-on log\n"
        "  -o, -||-              Read oneliners\n"
        "  -a, -||-              Read iMDB log\n"
        "  -k, -||-              Read game log\n"
        "  -h, -||-              Read tvrage log\n"
        "  -w  -||- [--comp]     Read online users data from shared memory\n"
        "  -q <dirlog|nukelog|dupefile|lastonlog|imdb|game|tvrage|ge1|ge2|ge3|ge4|sconf|gconf> [--raw] [--batch] [-E] [-print] ..\n"
        "                        Read specified log\n"
        "  -t                    Read all user files inside /ftp-data/users\n"
        "  -g                    Read all group files inside /ftp-data/groups\n"
        "  -x <root dir> [-R] ([--dir]|[--file]|[--cdir]) [--maxdepth=<limit>] [--mindepth=<limit>] [--fd] [-print] [ --r[u]sleep]..\n"
        "                        Traverse filesystem and processes each item found with available filters/hooks\n"
        "                          --dir - scan directories only\n"
        "                          --file - scan files only (default is both dirs and files)\n"
        "                          --cdir - process only the root directory itself\n"
        "                          --maxdepth - limit how deep into the directory tree recursor descends\n"
        "                          --mindepth - process only when recursor depth is over <limit>\n"
        "                          --fd - apply filters before recursor descends into subdirectory\n"
        "                          --recursive (-R) - traverse the whole <root dir> directory tree\n"
        "                          --xretry - re-start the operation when no results\n"
        "  -print  <format string>\n"
        "                        Format output using {var} directives (see DATA_STRUCTURES for a field list)\n"
        "  -printf <format string>\n"
        "                        Same as -printf, only does not print a new line character at the end\n"
        "  --stdin               Read data from stdin\n"
        "\n Input:\n"
        "  -e <dirlog|nukelog|dupefile|lastonlog|imdb|game|tvrage|ge1>\n"
        "                        Rebuilds existing data file, based on filtering rules (see --exec,\n"
        "                          --[i]regex[i] and --[i]match\n"
        "  -z <dirlog|nukelog|dupefile|lastonlog|imdb|game|tvrage|ge1> [--infile=<file>] [--binary] [--prune] [--raw]\n"
        "                        Creates a binary record from ASCII data, inserting it into the specified log\n"
        "                          Captures input from stdin, unless --infile is set\n"
        "                        --binary expects a normal binary log as input and merges it\n"
        "                        --prune skips importing duplicate records (full binary compare)\n"
        "                        --raw writes binary result to stdout\n"
        "\n Directory log:\n"
        "  -s <folders>          Import specific directories. Use quotation marks with multiple arguments\n"
        "                           <folders> are passed relative to SITEROOT, separated by space\n"
        "                           Use -f to overwrite existing entries\n"
        "  -r [-u]               Rebuild dirlog based on filesystem data\n"
        "                           .folders file (see MANUAL) defines a list of dirs in SITEROOT to scan\n"
        "                           -u - only import new records and do not truncate existing dirlog\n"
        "                           --full - ignore .folders file and do a full recursive scan\n"
        "                           --indepth - scans each imported directory recursivelly\n"
        "                           --nonukechk - disable checking dir nuke status in nukelog\n"
        "                           --indepth - scan all subdirectories for each record found\n"
        "  -c, --check [--fix] [--ghost]\n"
        "                        Compare dirlog and filesystem records and warn on differences\n"
        "                          --fix attempts to correct dirlog/filesystem\n"
        "                          --ghost only looks for dirlog records with missing directories on filesystem\n"
        "                          Folder creation dates are ignored unless -f is given\n"
        "  -p, --dupechk         Look for duplicate records within dirlog and print to stdout\n"
        "  -b, --backup <dirlog|nukelog|dupefile|lastonlog|imdb|game|tvrage|ge1>\n"
        "                        Perform backup on specified log\n"
        "\n Other:\n"
        "  -m <macro> [--arg[1-3] <string>]\n"
        "                         Searches subdirs for script that has the given macro defined, and executes\n"
        "                         --arg[1-3] sets values that fill {m:arg[1-3]} variables inside a macro\n"
        "\n Hooks:\n"
        "  --exec <command [{field}..{field}..]>\n"
        "                        While parsing data structure/filesystem, execute shell command for each record\n"
        "                          Used with -r, -e, -p, -d, -i, -l, -o, -w, -t, -g, -x, -a, -k, -h, -n, -q\n"
        "                          Operators {..} are overwritten with dirlog values\n"
        "                          If return value is non-zero, the processed record gets filtered\n"
        "                          Uses system() call (man system)\n"
        "  --execv <command [{field}..{field}..]> [--esredir=<file>]\n"
        "                        Same as --exec, only instead of calling system(), this uses execv() (man exec)\n"
        "                          It's generally much faster compared to --exec, since it doesn't fork /bin/sh\n"
        "                          for each individual processed record\n"
        "                        --esredir redirects stdout from executed command to <file>\n"
        "  --preexec  <command [{field}..{field}..]>\n"
        "                        Execute shell <command> before starting main procedure\n"
        "  --postexec <command [{field}..{field}..]>\n"
        "                        Execute shell <command> after main procedure finishes\n"
        "  --loopexec <command [{field}..{field}..]>\n"
        "                        Execute command each loop (see --loop)\n"
        "\n Matching:\n"
        "  --regex [<field>,]<match>\n"
        "                        Regex filter string, used during various operations\n"
        "                          If <field> is set, matching is performed against a specific data log field\n"
        "                            (field names are the same as --exec variable names for logs)\n"
        "                          Used with -r, -e, -p, -d, -i, -l, -o, -w, -t, -g, -x, -a, -k, -h, -n, -q\n"
        "  --regexi [<var>,]<match>\n"
        "                        Case insensitive variant of --regex\n"
        "  --iregex [<var>,]<match> \n"
        "                        Same as --regex with negated match\n"
        "  --iregexi [<var>,]<match>\n"
        "                        Same as --regexi with negated match\n"
        "  --noereg              Disable POSIX Extended Regular Expression syntax (enabled by default)\n"
        "  --match [<field>,]<match>\n"
        "                        Regular filter string (exact matches)\n"
        "                          Used with -r, -e, -p, -d, -i, -l, -o, -w, -t, -g, -x, -a, -k, -h, -n, -q\n"
        "  --imatch [<field>,]<match>\n"
        "                        Inverted --match\n"
        "  --lom <<field> > 5.0 && <field> != 0 || <field> ..>\n"
        "                        Compare values by logical and comparison/relational operators\n"
        "                        Applies to any integer/floating point fields from data sources\n"
        "                          Use quotation marks to avoid collisions with bash operators\n"
        "                        Valid logical operators: && (and), || (or)\n"
        "                        Valid comparison/relational operators: =, !=, >, <, <=, >=\n"
        "  --ilom <expression>   Same as --lom with negated match\n"
        "  --maxhit <limit>      Maximum number of positive filter matches (rest are forced negative)\n"
        "  --maxres <limit>      Maximum number of negative filter matches (rest are forced positive)\n"
        "  --ifhit               Ignore first match\n"
        "  --ifres               Ignore first result\n"
        "  --ifrhe               Takes -exec[v] return value into account with --max[hit|res] and --if[hit|res]\n"
        "  --matchq              Exit on first match\n"
        "  --imatchq             Exit on first result\n"
        "\n"
        "  In between match arguments, logical or|and operators apply:\n"
        "  \".. --<argument1> <or|and> --<margument2> ..\"\n"
        "\n Misc:\n"
        "  --sort <mode>,<order>,<field>\n"
        "                        Sort data log entries\n"
        "                          <mode> can only be 'num' (numeric)\n"
        "                          <order> can be 'asc' (ascending) or 'desc' (descending)\n"
        "                          Sorts by the specified data log <field>\n"
        "                          Used with -e, -d, -i, -l, -o, -w, -a, -k, -h, -n, -q\n"
        "\n"
        "Options:\n"
        "  -f                    Force operation where it applies (use -ff for greater effect)\n"
        "  -v                    Increase verbosity level (use -vv or more for greater effect)\n"
        "  --stats               Output statistics after operation finishes (applies to dump/parse ops)\n"
        "  --nostats             Disable operation statistics output\n"
        "  --nowrite             Perform a dry run, executing normally except no writing is done\n"
        "  --nobuffer            Disable data file memory buffering\n"
        "  -y, --followlinks     Follow symbolic links (default is skip)\n"
        "  --nowbuffer           Disable write pre-caching (faster but less safe), applies to -r\n"
        "  --memlimit=<bytes>    Maximum file size that can be pre-buffered into memory\n"
        "  --memlimita=<bytes>   Maximum ASCII input data file size (ignored)\n"
        "  --shmem [--shmdestroy] [--shmdestonexit] [--shmreload]\n"
        "                        Instead of process memory pages, use the shared memory to buffer log data\n"
        "                           This is usefull as an inter-process caching mechanism, allowing other glutil\n"
        "                            instances (or other processes) rapid access to the log data, without having to\n"
        "                            allocate pages and load from filesystem each time\n"
        "                           If log file is present and it's size doesn't match segment size, glutil\n"
        "                            destroys the segment and re-creates it with new size\n"
        "                           Although this applies globally, it should only be used with dump operations (-d,-n,-i,..)\n"
        "                           DO NOT use, unless you know exactly what you are doing\n"
        "                        --shmdestroy forces the old segment (if any) be destroyed and re-loaded\n"
        "                        --shmdestonexit destroys used segments before exiting process\n"
        "                        --shmreload forces existing records (if any) be reloaded, but segment is not destroyed\n"
        "                           and it's size remains the same (when data log size doesn't match segment size,\n"
        "                           there will be junk/missing records, depending on input size being higher/lower)\n"
        "  --loadq               Quit just after loading data into memory\n"
        "                           Applies to dump operations only\n"
        "  --nofq                Abort data (re)build operation unconditionally, if nothing was filtered\n"
        "  --noglconf            Disable reading settings from glftpd.conf\n"
        "  --nobackup            Disable log backup before rebuilding\n"
        "  --sfv                 Generate new SFV files inside target folders, works with -r [-u] and -s\n"
        "                           Used by itself, triggers -r (fs rebuild) dry run (does not modify dirlog)\n"
        "                           Avoid using this if doing a full recursive rebuild\n"
        "  --xdev                Ignores files/dirs on other filesystems\n"
        "                           Applies to -r, -t, -g, -x (can apply to other modes)\n"
        "  --xblk                Ignores files/dirs on non-block devices\n"
        "                           Applies to -r, -t, -g, -x (can apply to other modes)\n"
        "  --rev                 Reverses the order in which records are enumerated\n"
        "  --ipc <key>           Override gl's shared memory segment key setting\n"
        "  --daemon              Fork process into background\n"
        "  --loop <interval>     Loops the given operation\n"
        "                           Use caution, some operations might fail when looped\n"
        "                           This is usefull when running yourown scripts (--exec)\n"
        "  --rsleep              Timeout in seconds (log parsing/filesystem walking)\n"
        "  --rusleep             Timeout in nanoseconds (log parsing/filesystem walking)\n"
        "  --loglevel <0-6>      Log verbosity level (1: exception only..6: everything)\n"
        "                           Level 0 turns logging off\n"
        "  --silent              Silent mode\n"
        "  --ftime               Prepend formatted timestamps to output\n"
        "  --log                 Force logging enabled\n"
        "  --fork <command>      Fork process into background and execute <command>\n"
        "  --sleep <timeout>     Wait for <timeout> seconds before running\n"
        "  --usleep <timeout>    Wait for <timeout> nanoseconds before running\n"
        "  --version             Print version and exit\n"
        "\n"
        "Directory and file:\n"
        "  --glroot=<path>       glFTPd root path\n"
        "  --siteroot=<path>     Site root path (relative to glFTPd root)\n"
        "  --dirlog=<file>       Path to directory log\n"
        "  --nukelog=<file>      Path to nuke log\n"
        "  --dupefile=<file>     Path to dupe file\n"
        "  --lastonlog=<file>    Path to last-on log\n"
        "  --oneliners=<file>    Path to oneliners file\n"
        "  --imdblog=<file>      Path to iMDB log\n"
        "  --gamelog=<file>      Path to game log\n"
        "  --tvlog=<file>        Path to TVRAGE log\n"
        "  --ge1log=<file>       Path to GENERIC1 log\n"
        "  --ge2log=<file>       Path to GENERIC2 log\n"
        "  --ge3log=<file>       Path to GENERIC3 log\n"
        "  --ge4log=<file>       Path to GENERIC4 log\n"
        "  --sconf=<file>        Path to SCONF file\n"
        "  --gconf=<file>        Path to GCONF file\n"
        "  --glconf=<file>       Path to glftpd.conf (set using glconf.h by default)\n"
        "  --folders=<file>      Path to folders file (contains sections and depths,\n"
        "                           used on recursive imports)\n"
        "  --logfile=<file>      Log file path\n\n"
        "Report bugs to '%s'\n"
        "\n";

#endif /* G_HELP_H_ */
