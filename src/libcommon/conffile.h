/*****************************************************************************\
 *  $Id: conffile.h,v 1.1 2004-01-10 00:35:00 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2003 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>
 *  UCRL-CODE-155699
 *  
 *  This file is part of Whatsup, tools and libraries for determining up and
 *  down nodes in a cluster. For details, see http://www.llnl.gov/linux/.
 *  
 *  Whatsup is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by the 
 *  Free Software Foundation; either version 2 of the License, or (at your 
 *  option) any later version.
 *  
 *  Whatsup is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 *  for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with Whatsup; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#ifndef _CONFFILE_H
#define _CONFFILE_H 1

/*
 * This is a generic configuration file parsing library.  Some library
 * ideas were from libdotconf by Lukas Schroder <lukas@azzit.de>.
 * http://www.azzit.de/dotconf/.
 * 
 * This library parses configuration files in the form:
 *
 * optionname arg1 arg2 arg3
 *
 * Different option names are listed on different lines.  They are
 * separated from their arguments by whitespace.  Each argument is
 * also separated by whitespace.  Options may have a specified or
 * varying number of arguments, depending on the option's type
 * specification listed below under "OPTION TYPES"
 *
 * Comments can be listed by the '#' sign.  The '#' and any characters
 * to the right of it are ignored.  Lines can be continued on the next
 * line if a '\' character is the last non-whitespace character on a
 * line.  Strings can be quoted using double quotation marks '"'.  The
 * '#', '"', and '\' characters can be escaped using the '\'
 * character.
 *
 * The '#' character takes precedence over the '"' character in
 * parsing.  Therefore, the following would be a parse error:
 *
 * optionname    "#"
 *
 * The '#' character takes precendence, thus the second '"' character
 * will be ignored.  A parse error occurs because there is no ending
 * quote.  To fix this, the '#' character must be escaped:
 *
 * optionname    "\#"
 *
 * Continuation characters at the end of a line take precedence over
 * escape characters.  Therefore the following results in a parse
 * error:
 *
 * optioname     arg1 arg2 \\
 *
 * The last '\' character is assumed to be a continuation character,
 * removed, and the next line is read assuming it is a continuation of
 * the previous line.  When the arguments are parsed, it is a assumed
 * a stray '\' exists in the arguments.  To fix this, there must be
 * three '\' characters
 *
 * When a parse error with quotes occurs, a PARSE_QUOTE error code is
 * returned.  When a parse error occurs with a '\' character, a
 * PARSE_CONTINUATION error code is returned.  A number of other error
 * codes can be returned.  See ERROR CODES for details.
 *
 */

/* OPTION TYPES
 * 
 * The following are option types an option may take.
 * 
 * IGNORE - up to MAX_ARGS arguments, will not call a callback function
 *        - useful for deprecating old configuration options
 * FLAG - no arguments, returns no arguments
 * INT - 1 argument, returns an integer
 * DOUBLE - 1 argument, returns a double
 * STRING - 1 argument, returns a string
 * BOOL - 1 argument, returns 1 or 0 
 *      - the following indicate 1 - "1", "y", "yes", "on", "t", "true"  
 *      - the following indicate 0 - "0", "n", "no" "off", "f", "false"  
 * LIST - up to MAX_ARGS arguments, each a string up to MAX_ARGLEN in length
 *
 * If an argument is missing a PARSE_NO_ARG error is returned.  If the
 * incorrect number of arguments is listed, PARSE_NUM_ARGS is
 * returned.  If an invalid argument is listed, PARSE_INVALID_ARG is
 * returned.
 *   
 */
#define CONFFILE_OPTION_IGNORE                 0x00
#define CONFFILE_OPTION_FLAG                   0x01
#define CONFFILE_OPTION_INT                    0x02
#define CONFFILE_OPTION_DOUBLE                 0x03
#define CONFFILE_OPTION_STRING                 0x04
#define CONFFILE_OPTION_BOOL                   0x05
#define CONFFILE_OPTION_LIST                   0x06

/* LENGTHS
 *
 * The following are the maximum values and lengths throughout
 * the conffile parser.
 */

#define CONFFILE_MAX_LINELEN                  32778
#define CONFFILE_MAX_OPTIONNAMELEN              256
#define CONFFILE_MAX_ARGS                        64
#define CONFFILE_MAX_ARGLEN                     512
#define CONFFILE_MAX_ERRMSGLEN                 1024

/* ERROR CODES
 * 
 * The following are the error codes that may be returned to the user.
 * The error codes and strings describing the error codes can be
 * accessed through conffile_errnum(), conffile_strerror(), and
 * conffile_errmsg().  The error code can be set using
 * conffile_seterrnum().
 *
 * A number of error codes can be ignored during parsing.  To
 * determine if an error code is ignorable, use the IS_IGNORABLE_ERROR
 * macro listed below.
 */

#define CONFFILE_ERR_SUCCESS                   0x00
#define CONFFILE_ERR_PARSE_OPTION_UNKNOWN      0x01
#define CONFFILE_ERR_PARSE_OPTION_TOOMANY      0x02
#define CONFFILE_ERR_PARSE_OPTION_TOOFEW       0x03
#define CONFFILE_ERR_PARSE_OVERFLOW_LINELEN    0x04
#define CONFFILE_ERR_PARSE_OVERFLOW_OPTIONLEN  0x05
#define CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN     0x06
#define CONFFILE_ERR_PARSE_ARG_MISSING         0x07
#define CONFFILE_ERR_PARSE_ARG_TOOMANY         0x08
#define CONFFILE_ERR_PARSE_ARG_INVALID         0x09
#define CONFFILE_ERR_PARSE_QUOTE               0x0a
#define CONFFILE_ERR_PARSE_CONTINUATION        0x0b
#define CONFFILE_ERR_PARSE_CALLBACK            0x0c
#define CONFFILE_ERR_EXIST                     0x0d
#define CONFFILE_ERR_OPEN                      0x0e
#define CONFFILE_ERR_READ                      0x0f
#define CONFFILE_ERR_OUTMEM                    0x10
#define CONFFILE_ERR_PARAMETERS                0x11
#define CONFFILE_ERR_MAGIC                     0x12
#define CONFFILE_ERR_NULLHANDLE                0x13
#define CONFFILE_ERR_INTERNAL                  0x14
#define CONFFILE_ERR_ERRNUMRANGE               0x15

/* FLAGS
 * 
 * The following flags can be passed to conffile_setup() to alter
 * behavior of conffile parsing.
 *
 * OPTION_CASESENSITIVE - By default option names are case
 *                        insensitive.  This flag informs the parser
 *                        to make option names case sensitive
 * 
 */
 
#define CONFFILE_FLAG_OPTION_CASESENSITIVE      0x00000001

/* DATA TYPES */

/* conffile_t
 * - conffile library handle type
 */
typedef struct conffile *conffile_t;

/* conffile_data
 * 
 * This stores data from an options arguments and is passed to the
 * callback function so the data can be read or copied.
 *
 * Buffers will be destroyed when parsing has completed, therefore a
 * callback function should not save pointers, it should copy any data
 * it wishes to save.
 */
struct conffile_data {
    int intval;
    double doubleval;
    char string[CONFFILE_MAX_ARGLEN];
    int bool;
    char list[CONFFILE_MAX_ARGS][CONFFILE_MAX_ARGLEN];
    int list_len;
};

/* conffile_option_func
 * 
 * This is the callback function type, functions that are called after
 * an option name and its potential arguments have been parsed by the
 * conffile parser.
 *
 * 'optionname' is the option name that was just parsed.
 * 'option_type' is the option type specified in conffile through a 
 *     struct conffile_option type.  See below.
 * 'data' is a pointer to argument data.  The data that should be
 *     accessed depends on the option type.
 * 'option_ptr' is a pointer to data specified through a
 *     struct conffile_option type.  See below.
 * 'app_ptr' is a pointer to data specified in conffile_setup().
 *
 * Typically, the option_ptr will point to a buffer to store argument
 * data or a flag that can be modified to indicate an option was
 * found.  The callback function then stores data into the buffer from
 * the data pointed at by the 'data' pointer.  The app_ptr is a
 * pointer to data passed into the conffile_setup() function.  It can
 * be used for anything.  For example, it could be used to handle
 * separate contexts within a configuration file.
 *
 * The function should return 0 if the argument was read properly and
 * the parser should continue parsing.  Return -1 if an error has
 * occurred and you wish the parser to quit.  If an error code is set
 * in a callback function using conffile_seterrnum() that errnum will
 * be passed back to the original caller of conffile_parse().  If no
 * error code is set, ERR_CALLBACK will be passed back from
 * conffile_parse().
 */
typedef int (*conffile_option_func)(char *optionname,
                                    int option_type,
                                    struct conffile_data *data,
                                    void *option_ptr,
                                    void *app_ptr);

/* conffile_option
 *
 * An array of this structure specifies the options to be searched for
 * in the configuration file.
 *
 * 'optionname' is the option name that should be serached for.
 * 'option_type' is the option type specified in conffile.  See 
 *      OPTION TYPES above.   
 * 'callback_func' is the callback function to be called when the
 *     option has been found.
 * 'max_count' is the maximum number of times this option can be
 *     listed in the configuration file.  Typically this is one.
 * 'required_count' is the required number of times this option should
 *     be listed in the configuration file.  Typically this is 0 for not
 *     required, or identical to 'max_count'.
 * 'count_ptr' points to an integer that will be incremented to the
 *     number of times this option has been listed
 * 'option_ptr' is a pointer to data that will be passed to the callback
 *     function.  Typically, a buffer pointer is passed.  This parameter
 *     is optional and can be set to NULL.
 */
struct conffile_option {
    char *optionname;
    int option_type;
    conffile_option_func callback_func;
    int max_count;
    int required_count;
    int *count_ptr;
    void *option_ptr;
};

/* API */

/* conffile_handle_create
 * 
 * Create a conffile handle.  
 * Returns handle on success, NULL on error.
 */
conffile_t conffile_handle_create(void);

/* conffile_handle_destroy
 * 
 * Destroy a conffile handle.
 * Returns 0 on success, -1 on error.
 */
int conffile_handle_destroy(conffile_t cf);

/* conffile_errnum
 * 
 * Get the most recent error code number.
 * Returns errnum
 */
int conffile_errnum(conffile_t cf);

/* conffile_errmsg
 * 
 * Get an error message of the most recent error.  When appropriate,
 * the error message returned in the buffer will include information
 * on the optionname or line number of the parse error.  Returns 0 on
 * success, -1 if the buffer passed in is not large enough.
 */
int conffile_strerror(conffile_t cf, char *buf, int buflen);

/* conffile_seterrnum
 * 
 * Set the error code number in a conffile handle.  This is primarily
 * used in an error handler or callback function to set error codes
 * when an error has occurred within user space parsing.  Returns 0 on
 * success, -1 if errnum is out of range.
 */   
int conffile_seterrnum(conffile_t cf, int errnum);

/* conffile_parse
 *
 * Parse a configuration file.  
 * Returns 0 on success, -1 on error.
 */
int conffile_parse(conffile_t cf, char *filename,
                   struct conffile_option *options,
                   int options_len, void *app_ptr, int flags);

#endif /* _CONFFILE_H */
