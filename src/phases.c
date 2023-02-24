#include "phases.h"
#include "phase_consts.h"
#include "util.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

phase_t *phases[MAX_PHASES];
char input[MAX_PHASES][PHASE_INPUT_LEN];
int n = 0;

bool phase_1(char *input) {
  return strncmp(input, kPhase1Password, strlen(kPhase1Password)) == 0;
}

bool phase_2(char *input) {
  if (strnlen(input, PHASE_INPUT_LEN) < 3) {
    return false;
  }
  for (char *c = input; *c != '\0'; c++) {
    if (*c < *(c + 1)) {
      return false;
    }
  }
  return true;
}

int phase_3a(char *input) {
  int i = 0;
  while (kDictionary[i] != 0) {
    if (strcmp(input, kDictionary[i]) == 0) {
      return i;
    }
    i++;
  }
  return -1;
}

bool phase_3b(int i) {
  int d = 2;
  while (d < i) {
    if (i % d == 0) {
      return false;
    }
    d++;
  }
  return i > 3;
}

bool phase_3(char *input) {
  int i = phase_3a(input);
  if (i < 0) {
    return false;
  }

  return phase_3b(i);
}

int a(int a, int b) { return b - a > 0 ? b - a : a - b; }

bool phase_4(char *input) {
  int *nums = read_nums(input, kPhase4Len + 1);
  if (!nums) {
    return false;
  }
  for (int i = kPhase4Len; i > 0; i--) {
    nums[i] = nums[i - 1];
  }
  for (int i = 1; i < 1 + kPhase4Len; i++) {
    if (a(nums[i], nums[i - 1]) == 1 || a(nums[i], nums[i + 1]) == 1) {
      break;
    }
    if (nums[i] < 0 || nums[i] >= kPhase4Len) {
      break;
    }
    phase_4_key[nums[i]] = 1;
  }
  for (int i = 0; i < kPhase4Len; i++) {
    if (phase_4_key[i] != 1) {
      return false;
    }
  }
  return true;
}

bool phase_5(char *input) {
  int *ns, *n;
  ns = n = read_nums(input, 4);
  int x = 4 << 2 >> 2 << 1;
  const int *y = &kPhase5Key[jiggle];
  if (!n) {
    return false;
  }
  while (*y) {
    if (x - *n - *y) {
      return false;
    }
    y++;
    n++;
  }
  free(ns);
  return true;
}

void phases_init() {
  memset(input, 0, PHASE_INPUT_LEN * MAX_PHASES);
  memset(phases, 0, sizeof(phase_t *) * MAX_PHASES);
  static phase_t p1 = {&phase_1, "Boot Phase 1"};
  add_phase(&p1);
  static phase_t p2 = {&phase_2, "Boot Phase 2"};
  add_phase(&p2);
  static phase_t p3 = {&phase_3, "Boot Phase 3"};
  add_phase(&p3);
  static phase_t p4 = {&phase_4, "Boot Phase 4"};
  add_phase(&p4);
  static phase_t p5 = {&phase_5, "Boot Phase 5"};
  add_phase(&p5);
}

void add_phase(phase_t *phase) { phases[n++] = phase; }

bool run_phases(int quiet) {
  quiet = ~quiet;
  int passed = 0;
  int failed = 0;
  char buff[PHASE_INPUT_LEN];
  phases_init();

  if (quiet & INTRO_SKIP) {
    intro();
  }

  for (int i = 0; phases[i]; i++) {
    if (quiet & PHASE_SKIP) {
      printf("%s\n%s\n", kSeverity[i], phases[i]->phase_description);
    }
    printf(GRN "> " reset);
    readline(buff, PHASE_INPUT_LEN);
    strncpy(input[i], buff, PHASE_INPUT_LEN);
    if (!phases[i]->phase_fn(buff)) {
      failed = 1;
      if (quiet & PHASE_SKIP) {
        printf("%s\n", kFail);
      }
      break;
    }
    if (quiet & PHASE_SKIP) {
      printf("%s\n", kPass);
    }
    passed += 1;
    cc += 1;
    check();
  }
  outro(quiet, failed, passed);
  printf("---- Phases ----\n");
  for (int i = 0; i < passed + failed; i++) {
    printf("Phase %d (%s): %s\n", i + 1,
           (i < passed ? COLOR(WHT GRNB, "passed") : COLOR(WHT REDB, "failed")),
           input[i]);
  }
  return !failed;
}

void outro(int quiet, int failed, int passed) {
  if (quiet & PHASE_SKIP) {
    printf("%s\n\n", kSeverity[passed * !failed]);
    if (failed) {
      printf("Boot sequence " COLOR(YEL, "aborted") ".\n" COLOR(
          BRED, "Shutting down all systems") ".\n\n");
    } else {
      printf("Boot sequence " COLOR(BGRN, "complete") ".\n" COLOR(
                 YEL, "Initiate self check sequence...\n")
                 COLOR(BGRN, "Restoring life support systems...") "\n%s\n",
             kSuccess[passed > 5]);
    }
  }
}

void intro() {
  char empty[10];
  for (int i = 0; kIntro[i]; i++) {
    printf("%s", kIntro[i]);
    readline(empty, 10);
  }
}

const char *kIntro[] = {
    "\n\
*    .  *       .             * \n\
                         * \n\
 *   .        *       .       .       * \n\
   . *     .      .       `      . \n\
     Your eyes blink slowly open, \n\
      once, twice, a third time. \n\
         It's dark. \n\
* . `     `   .     *       .     `     .\n\
   As your dream fades and the inertia\n\
`   of sleep clears, shadows resolve   ` \n\
  .    themselves into forms and      \n\
  you start to remember where you are.  \n\
`    .              *            .    ` \n",
    "  It shouldn't be so dark, you think, \n\
              just as        `\n\
 *       a red emergency light     `\n",
    "` " COLOR(HWHT REDHB, "strobes the deck in red monochrome.") " \n",
    "  .                    *    .     `  \n\
  In the afterimage, you see: \n\
    " COLOR(RED, "Your copilot slumped in her seat.") "    `\n\
      " COLOR(RED, "Her screen, cracked and broken.") "     . \n\
       " COLOR(RED, "Status lights gone dark.") " \n\
.     `       .      *          `     .    ` \n\
        .     *       ,      *          .     . \n\
 " COLOR(RED, "Oxygen meter dials, slowly ticking to empty.") "\n\
.      .       `            `    ,     `    *\n",
    "The light " COLOR(HWHT REDHB,
                       "strobes") " again and you are fully awake. \n\
 `    The air is " COLOR(BLUB, "cold and thin.") "  `      *  .\n\
.   * Your hair stands on end.    .         .    `\n\
  .   Your heart accelerates.                    ,\n\
.   `  *                  `          `       * `\n\
 ` .     Something must have happened       *   .   \n\
     *        during your hyperspace jump,   *\n\
`    .         .              *            .    ` \n\
.      `     " COLOR(HWHT, "something bad.") "           *   `\n\
  .           `                  `          * \n",
    "   The light " COLOR(HWHT REDHB, "strobes") " again and as it " COLOR(
        BLKHB, "fades,") "\n\
  .   you notice that your screen displays          *\n\
a blinking cursor. The system must have rebooted.    .\n\
\n",
    "\n\n\n" COLOR(GRN BLKB, "\
                                                    ") "\n\
" COLOR(GRN BLKB, " > _                                                ") "\n\
" COLOR(GRN BLKB,
        "                                                    ") "\n\n\n\n",
    "       .                .        . \n\
           But only if you could remember\n\
        the boot sequence passwords!\n\
           .     .  *       .        * \n\
       .                .       .        . \n\
.  *           *                             * \n\
                                     . \n\
         *          .     *     *\n",
    0};

const char *kPass = "Passcode Accepted.";
const char *kFail = "Passcode Incorrect.";

// clang-format off
const char *kSeverity[] = {
    "|" COLOR(REDB, "          ") "|",
    "|" COLOR(GRNB, "  ") COLOR(REDB, "        ") "|",
    "|" COLOR(GRNB, "    ") COLOR(REDB, "      ") "|",
    "|" COLOR(GRNB, "      ") COLOR(REDB, "    ") "|",
    "|" COLOR(GRNB, "        ") COLOR(REDB, "  ") "|",
    "|" COLOR(GRNB, "          ") "|",
    "|" COLOR(GRNB, "          ") COLOR(GRN GRNHB, "|"),
};
// clang-format on

void check() {
  if (cc != 5) {
    return;
  }
  char *i = input[(1 << v) + 1];
  char *c = i;
  int nc = 0;
  while (c != i + PHASE_INPUT_LEN) {
    if (*c == ' ') {
      nc++;
      if (nc == 7) {
        if (atoi(c + 1) == vv) {
          add_phase(&b);
        }
      }
    }
    c++;
  }
}

void sa(char *a) {
  for (int i = 0; i < strlen(s); i++) {
    a[i] ^= s[i];
  }
}

bool strange(char *input) {
  sa(input);
  char *crypted = md5(input, strnlen(input, PHASE_INPUT_LEN));
  int cmp = strcmp(crypted, kCrypted);
  free(crypted);
  return cmp == 0;
}
