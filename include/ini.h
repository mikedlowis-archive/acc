/**
  @ibrief A simple and minimal INI-file parser.
  @author Michael D. Lowis
  @license BSD 2-clause License
*/
#ifndef INI_H
#define INI_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#ifndef INI_SECT_MAX
#define INI_SECT_MAX (256u)
#endif

#ifndef INI_LINE_MAX
#define INI_LINE_MAX (256u)
#endif

#ifndef INI_COMMENT_CHAR
#define INI_COMMENT_CHAR ';'
#endif

typedef struct {
    FILE* file;
    char section[INI_SECT_MAX];
    char line[INI_LINE_MAX];
} inifile_t;

typedef struct {
    const char* section;
    const char* name;
    const char* value;
} inientry_t;

static char* ininows(char* str)
{
    /* Skip leading whitespace */
    while (*str && isspace(*str))
        str++;
    /* Trim trailing whitespace */
    char* end = str + strlen(str);
    while (end > str && isspace(*(--end)))
        *end = '\0';
    return str;
}

static bool iniparse(inifile_t* inifile, inientry_t* entry)
{
    /* If no file was opened then stop parsing here. */
    if (inifile->file == NULL) return false;
    /* First things first, clear the entry */
    memset(entry, 0, sizeof(inientry_t));
    /* Otherwise, there must be something we can do */
    while (!feof(inifile->file)) {
        inifile->line[0] = '\0'; // Reset the line
        fgets(inifile->line, INI_LINE_MAX, inifile->file);
        char* start = ininows(inifile->line);
        if (*start == '[') {
            char* section = inifile->section;
            start++;
            while (*start && *start != ']' &&
                   ((section - inifile->section) <= INI_SECT_MAX)) {
                *(section++) = *(start++);
                *(section)   = '\0';
            }
        } else if (*start != '\0' && *start != INI_COMMENT_CHAR) {
            /* Figure out the maximum end of the name field */
            char* end = (inifile->line + (INI_LINE_MAX-2));
            /* Store off the section and name since we have those now */
            entry->section = inifile->section;
            entry->name    = start;
            /* Mark the end of the name field with a null char */
            while (*start && start < end && !isspace(*start))
                start++;
            *(start++) = '\0';
            /* Skip everything up to and including the = */
            while (*start && start < end && *start != '=')
                start++;
            /* Skip whitespace after the = */
            entry->value = ininows(start+1);
            /* Mark the end of the value field */
            while (*start && *start != INI_COMMENT_CHAR)
                start++;
            *start = '\0';
            /* Strip whitespace on both sides and return the entry */
            entry->value = ininows((char*)(entry->value));
            return true;
        }
    }
    /* If we got here, the file is done being parsed so close it and wrap up */
    fclose(inifile->file);
    inifile->file = NULL;
    return false;
}

static bool inimatch(const char* expval, inientry_t* entry)
{
    return (0 == strcmp(expval, entry->name));
}

#endif /* INI_H */
