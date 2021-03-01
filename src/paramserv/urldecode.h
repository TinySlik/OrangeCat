#ifndef URLDECODE_H
#define URLDECODE_H

/**
 * Function: urlDecode
 * Purpose:  Decodes a web-encoded URL. By default, +'s are converted to spaces.
 * Input:    const char* str - the URL to decode
 * Output:   char* - the decoded URL
 */
char *urlDecode(const char *str);

#endif // URLDECODE_H
