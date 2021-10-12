/** Copyright 2021 Tiny Oh, Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "urldecode.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "urldecode.h"

/* Function: urlDecode */
char *urlDecode(const char *str) {
  int d = 0; /* whether or not the string is decoded */

  char *dStr = (char *)(malloc(strlen(str) + 1));
  char eStr[] = "00"; /* for a hex code */

  strcpy(dStr, str);

  while(!d) {
    d = 1;
    for(size_t i = 0; i < strlen(dStr); ++i) {
      if(dStr[i] == '%') {
        if(dStr[i+1] == 0)
        return dStr;
        if(isxdigit(dStr[i+1]) && isxdigit(dStr[i+2])) {
          d = 0;

          /* combine the next to numbers into one */
          eStr[0] = dStr[i+1];
          eStr[1] = dStr[i+2];

          /* convert it to decimal */
          long int x = strtol(eStr, nullptr, 16);

          /* remove the hex */
          memmove(&dStr[i+1], &dStr[i+3], strlen(&dStr[i+3])+1);

          dStr[i] = x;
        }
      }
    }
  }
  return dStr;
}
