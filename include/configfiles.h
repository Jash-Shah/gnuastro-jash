/*********************************************************************
configfiles -- Read configuration files for each program.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <akhlaghi@gnu.org>
Contributing author(s):
Copyright (C) 2015, Free Software Foundation, Inc.

Gnuastro is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Gnuastro is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with gnuastro. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
#ifndef CONFIGFILES_H
#define CONFIGFILES_H



/**************************************************************/
/************               Macros                *************/
/**************************************************************/
/* Simple macros: */
#define CONFIG_DELIMITERS " ,=:\t\n"



#define STARTREADINGLINE {						\
  ++lineno;								\
  if(*line=='#') continue;						\
  else readnamevalue(line, filename, lineno, &name, &value);		\
  if(name==NULL && value==NULL) continue;				\
}



/* Functional macros: These are not actual functions, because they
   depend on functions that are different for different programs. So
   they have to be written into the functions with a macro. */
#define SAVE_LOCAL_CONFIG(INDIR) {					\
    FILE *fp;								\
    char *outfilename, *command;		 			\
    fp=writelocalconfigstop(INDIR, CONFIG_FILE, SPACK,			\
			     SPACK_NAME, &outfilename);			\
    printvalues(fp, p);							\
    errno=0;								\
    if(fclose(fp)==-1)							\
      error(EXIT_FAILURE, errno, "%s", outfilename);                    \
    command=malloccat("cat ", outfilename);				\
    printf("Values saved in %s:\n\n", outfilename);			\
    if(system(command))                                                 \
      error(EXIT_FAILURE, 0, "The `%s` command could not be run or "    \
            "failed.", command);                                        \
    free(outfilename);							\
    free(command);							\
    exit(EXIT_SUCCESS);							\
  }





#define CHECKSETCONFIG {                                                \
    char *userconfig_dir, *userconfig_file;				\
                                                                        \
    readconfig(CURDIRCONFIG_FILE, p);                                   \
    if(cp->setdirconf)                                                  \
      SAVE_LOCAL_CONFIG(CURDIRCONFIG_DIR);                              \
    if(cp->onlyversionset && strcmp(cp->onlyversion, SPACK_VERSION))    \
      error(EXIT_FAILURE, 0, "The running version of %s is `%s'. "      \
            "However, you have asked for this %s run to be with "       \
            "version `%s'. Either through the command line or in a "    \
            "configuration file with the `--onlyversion' option. "      \
            "Please either remove it, or set it to `%s' with a command " \
            "like:\n\n"                                                 \
            "    %s --onlyversion=%s --setdirconf\n\n"                  \
            "Alternatively, you can install %s %s.\n"                   \
            "NOTE: If this option was in a configuration file (you "    \
            "didn't set it on the command line), then probably it was " \
            "intended for reproducability. If so, to be exactly "       \
            "reproducible, it is advised to install the requested "     \
            "version.", SPACK_NAME, SPACK_VERSION, SPACK_NAME,          \
            cp->onlyversion, SPACK_VERSION, SPACK, SPACK_VERSION,       \
            SPACK_NAME, SPACK_VERSION);                                 \
                                                                        \
    if(cp->onlydirconf==0)                                              \
      {                                                                 \
        userconfig_dir=addhomedir(USERCONFIG_DIR);                      \
        userconfig_file=addhomedir(USERCONFIG_FILEEND);			\
        readconfig(userconfig_file, p);                                 \
        if(cp->setusrconf) SAVE_LOCAL_CONFIG(userconfig_dir);           \
        readconfig(SYSCONFIG_FILE, p);                                  \
        free(userconfig_file);						\
        free(userconfig_dir);						\
      }                                                                 \
  }







#define REPORT_NOTSET(var_name) {					\
    if(intro==0)							\
      {									\
	fprintf(stderr, SPACK": Parameter(s) not set: %s", (var_name));	\
	intro=1;							\
      }									\
    else								\
      fprintf(stderr, ", %s", (var_name));				\
  }





#define END_OF_NOTSET_REPORT {			                        \
    if(intro)								\
      {									\
	char *userconfig_file;						\
	fprintf(stderr, ".\n\n");					\
	fprintf(stderr, "You can assign values in the local, user or "	\
		"system wide default files. Otherwise you have to "	\
		"explicitly call them each time. See `"SPACK" --help` "	\
		"or `info "SPACK"` for more information.\n\n");		\
	userconfig_file=addhomedir(USERCONFIG_FILEEND);			\
	fprintf(stderr, "Default files checked (existing or not):\n"	\
		"   %s\n   %s\n   %s\n", CURDIRCONFIG_FILE,		\
		userconfig_file, SYSCONFIG_FILE);			\
	free(userconfig_file);						\
	exit(EXIT_FAILURE);						\
      }									\
  }





#define REPORT_PARAMETERS_SET {			                        \
    fprintf(stdout, "# "SPACK_STRING"\n");				\
    fprintf(stdout, "# Configured on "CONFIGDATE" at "CONFIGTIME"\n");	\
    fprintf(stdout, "# Written on %s", ctime(&p->rawtime));	        \
    printvalues(stdout, p);						\
    exit(EXIT_SUCCESS);							\
  }





/* Read the options that are common to all programs from the
   configuration file. Since these two checks are within an if-else
   structure, they should not be placed within an `{' and `}'. */
#define READ_COMMONOPTIONS_FROM_CONF                                    \
    else if(strcmp(name, "numthreads")==0)                              \
      {                                                                 \
        if(cp->numthreadsset) continue;                                 \
        sizetlzero(value, &cp->numthreads, name, key, SPACK,            \
                   filename, lineno);                                   \
        cp->numthreadsset=1;                                            \
      }                                                                 \
    else if(strcmp(name, "onlydirconf")==0)                             \
      {                                                                 \
        if(cp->onlydirconf==0)                                          \
          intzeroorone(value, &cp->onlydirconf, name, key, SPACK,       \
                       filename, lineno);                               \
      }                                                                 \
    else if(strcmp(name, "onlyversion")==0)                             \
        allocatecopyset(value, &cp->onlyversion, &cp->onlyversionset);  \





/* Write common options: */
#define PRINT_COMMONOPTIONS {                                           \
    fprintf(fp, "\n# Operating modes:\n");                              \
    if(cp->numthreadsset)                                               \
      fprintf(fp, CONF_SHOWFMT"%lu\n", "numthreads", p->cp.numthreads); \
    if(cp->onlyversionset)                                              \
      PRINTSTINGMAYBEWITHSPACE("onlyversion", cp->onlyversion);         \
  }









/**************************************************************/
/************       Function declarations         *************/
/**************************************************************/
char *
addhomedir(char *dir);

void
readnamevalue(char *line, char *filename, size_t lineno,
	      char **name, char **value);

FILE *
writelocalconfigstop(char *indir, char *filename, char *spack,
		     char *spack_name, char **outfilename);

#endif
