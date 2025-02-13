/*
 SPDX-FileCopyrightText: © 2014 Siemens AG

 SPDX-License-Identifier: GPL-2.0-only
*/
/**
 * \file
 * \brief Test cases for Doctored buffer
 */

#include <stdbool.h>

#include "nomos.h"
#include "doctorBuffer_utils.h"
#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <CUnit/CUnit.h>

#include <stdarg.h>
#include "nomos_utils.h"

#include "nomos.h"
#include "util.h"
#include "list.h"
#include "licenses.h"
#include "process.h"
#include "nomos_regex.h"
#include "_autodefs.h"

/**
 * \brief Test for doctorBuffer()
 * \test
 * -# Create a dirty string
 * -# Initialize scanner and call doctorBuffer() on dirty string
 * -# Check if the string is sanitized
 */
void test_doctorBuffer()
{
  licenseInit();
  char* buf, *fer;

  fer = g_strdup_printf("//Th- is is     a li-\n// cence of the test string");
  buf = g_strdup_printf("This is a li cence of the test string");
  printf("%s \n", buf);
  printf("%s \n", fer);
  initializeCurScan(&cur);
  doctorBuffer(fer, 0, 0, 0);
  printf("%s \n", buf);
  printf("%s \n", fer);
  CU_ASSERT_STRING_EQUAL(buf, fer);
  freeAndClearScan(&cur);
  g_free(buf);
  g_free(fer);
}

/**
 * \brief Test for uncollapsePosition()
 * \test
 * -# Create a dirty string
 * -# Initialize scanner and call doctorBuffer() on dirty string
 * -# Check if uncollapsePosition() refers to original position
 */
void test_doctorBuffer_uncollapse()
{

  licenseInit();
  char *fer, *cfer;
  fer = g_strdup_printf("//This              is         the test string");
  cfer= g_strdup( fer);
  printf("%s \n", fer);
  initializeCurScan(&cur);
  doctorBuffer(fer, 0, 0, 0);
  printf("Before %d, after %d", (int) strlen(cfer), (int) strlen(fer));

  for (int i = 0; i < strlen(fer); i++)
  {
    CU_ASSERT_EQUAL(*(fer + i), *(cfer + uncollapsePosition(i, cur.docBufferPositionsAndOffsets)));
  }

  g_free(cfer);
  g_free(fer);
}

/**
 * \brief Helper function to match licenses and highlight info
 */
static void report_Match(char* buf)
{
  printf("I have %i matches \n", cur.theMatches->len);
  for (int i = 0; i < cur.theMatches->len; ++i)
  {
    LicenceAndMatchPositions* licenceAndMatch =  getLicenceAndMatchPositions(cur.theMatches, i);
    for (int k=0; k < licenceAndMatch->matchPositions->len; ++k) {
      MatchPositionAndType* PaT = getMatchfromHighlightInfo(
          licenceAndMatch->matchPositions, k);
      printf("Match from %d to %d: ", PaT->start, PaT->end);

      for (int j = PaT->start; j < PaT->end; ++j)
      {
        printf("%c", *(buf + j));
      }

      printf("\n");
    }
  }
}

/**
 * \brief Test for idxGrep_recordPosition()
 * \test
 * -# Load a license info from a file
 * -# Add license to the current matches
 * -# Call idxGrep_recordPosition() to match license regex
 * -# Call report_Match() to verify the match
 */
void test_doctorBuffer_fromFile()
{

  licenseInit();
  char* buf, *undoc;
  buf = (char*) malloc(3000);

  int f = open("../testdata/NomosTestfiles/WXwindows/WXwindows.txt", O_RDONLY);
  int whatIread = read(f, buf, 3000);
  close(f);

  CU_ASSERT_EQUAL(whatIread, 2496);
  undoc = g_strdup(buf);

  printf("\n%s\n", undoc);
  fflush(stdout);
  int licence_index = _PHR_WXWINDOWS;
  int licence_index2 = _LT_LGPLref1;

  initializeCurScan(&cur);
  cur.currentLicenceIndex=0;
  g_array_append_val(cur.indexList, licence_index);
  g_array_append_val(cur.indexList, licence_index2);
  addLicence(cur.theMatches, "WXwindows");
  idxGrep_recordPosition(licence_index, undoc, REG_ICASE | REG_EXTENDED);
  idxGrep_recordPosition(licence_index2, undoc, REG_ICASE | REG_EXTENDED);
  report_Match(undoc);
  freeAndClearScan(&cur);

  report_Match(undoc);

  initializeCurScan(&cur);
  cur.currentLicenceIndex=0;
  g_array_append_val(cur.indexList, licence_index);
  g_array_append_val(cur.indexList, licence_index2);
  addLicence(cur.theMatches, "WXwindows");
  doctorBuffer(buf, 0, 0, 0);

  printf("\n%s\n", buf);
  idxGrep_recordPositionDoctored(licence_index, buf, REG_ICASE | REG_EXTENDED);
  idxGrep_recordPositionDoctored(licence_index2, buf, REG_ICASE | REG_EXTENDED);
  report_Match(undoc);
  freeAndClearScan(&cur);

  initializeCurScan(&cur);
  cur.currentLicenceIndex=0;
  g_array_append_val(cur.indexList, licence_index);
  g_array_append_val(cur.indexList, licence_index2);
  addLicence(cur.theMatches, "WXwindows");
  idxGrep_recordPosition(licence_index, buf, REG_ICASE | REG_EXTENDED);
  idxGrep_recordPosition(licence_index2, buf, REG_ICASE | REG_EXTENDED);
  report_Match(buf);
  freeAndClearScan(&cur);

  free(buf);
  g_free(undoc);

}

//! The test are in the order of the function calls in doctorBuffer
//! While the test result does not depend on the order.
//! If possible the output string of the first
//! test is the input string of the second one and so on.
//! Clear exceptions: Input cannot be MarkUp and PostScript simultaneously..
//! Please do not change the test order in this source file

/**
 * \brief Test for removeHtmlComments()
 * \test
 * Step 1: take care of embedded HTML/XML and special HTML-chars like
 * &quot; and &nbsp; -- but DON'T remove the text in an HTML comment.
 * There might be licensing text/information in the comment!
 *****
 * Later on (in parseLicenses()) we search for URLs in the raw-text
 */
void test_1_removeHtmlComments()
{
  initializeCurScan(&cur);
  char* textBuffer = g_strdup_printf("&quot the big\t(C) and long\\n &quot\\s-1234,"
              " test &copy; string \n con-\n// tains losts; of . <string test> &nbsp;"
              "  <body> \" compli-\n cated /* COMMENT s and funny */ Words as it \n "
              "mimi-cs printf(\"Licence\"); and so on\n &quot \n ");
  removeHtmlComments(textBuffer);
  char* te22Buffer = g_strdup_printf(" quot the big\t(C) and long\\n  quot\\s-1234,"
              " test &copy  string \n con-\n// tains losts; of . <string test   nbsp "
              "   body  \" compli-\n cated /* COMMENT s and funny */ Words as it \n "
              "mimi-cs printf(\"Licence\"); and so on\n  quot \n ");
  CU_ASSERT_STRING_EQUAL(te22Buffer, textBuffer);
  g_free(textBuffer);
  g_free(te22Buffer);
}

/**
 * \brief Test for removeLineComments()
 * \test
 * Step 2: remove comments that start at the beginning of a line, * like
 * ^dnl, ^xcomm, ^comment, and //
 */
void test_2_removeLineComments()
{
  initializeCurScan(&cur);
  char* textBuffer = g_strdup_printf(" quot the big\t(C) and long\\n  quot\\s-1234,"
              " test &copy  string \n con-\n// tains losts; of . <string test   nbsp "
              "   body  \" compli-\n cated /* COMMENT s and funny */ Words as it \n "
              "mimi-cs printf(\"Licence\"); and so on\n  quot \n ");

  removeLineComments(textBuffer);

  char* te22Buffer = g_strdup_printf(" quot the big\t(C) and long\\n  quot\\s-1234,"
              " test &copy  string \n con-\n\377\377 tains losts; of . <string test   nbsp"
              "    body  \" compli-\n cated /* COMMENT s and funny */ Words as it \n "
              "mimi-cs printf(\"Licence\"); and so on\n  quot \n ");
  CU_ASSERT_STRING_EQUAL(te22Buffer, textBuffer);
  g_free(textBuffer);
  g_free(te22Buffer);
}

/**
 * \brief Test for cleanUpPostscript()
 * \test
 * Step 3 - strip out crap at end-of-line on postscript documents
 */
void test_3_cleanUpPostscript()
{
  initializeCurScan(&cur);
  char* textBuffer=g_strdup_printf("(8) (89) -9.- A %%!PS-Adobe-3.0 (12) EPSF-3.0 --8. -9.- A");

  cleanUpPostscript(textBuffer);

  char* te22Buffer = g_strdup_printf("                %%!PS-Adobe-3.0 (12) EPSF-3.0            ");
  CU_ASSERT_STRING_EQUAL(te22Buffer, textBuffer);
  g_free(textBuffer);
  g_free(te22Buffer);
}


/**
 * \brief Test for removeBackslashesAndGTroffIndicators()
 * \test
 * Step 4: remove groff/troff font-size indicators, the literal
 *         string backslash-n and all backslahes, ala:
 * ==>  `perl -pe 's,\\s[+-][0-9]*,,g;s,\\s[0-9]*,,g;s/\\n//g;' |f`
 */
void test_4_removeBackslashesAndGTroffIndicators()
{
  initializeCurScan(&cur);
  char* textBuffer= g_strdup_printf(" quot the big\t(C) and long\\n  quot\\s-1234, test"
              " &copy  string \n con-\n\377\377 tains losts; of . <string test   nbsp  "
              "  body  \" compli-\n cated /* COMMENT s and funny */ Words as it \n "
              "mimi-cs printf(\"Licence\"); and so on\n  quot \n ");

  removeBackslashesAndGTroffIndicators(textBuffer);

  char* te22Buffer = g_strdup_printf(" quot the big\t(C) and long    quot       , test"
              " &copy  string \n con-\n\377\377 tains losts; of . <string test   nbsp  "
              "  body  \" compli-\n cated /* COMMENT s and funny */ Words as it \n "
              "mimi-cs printf(\"Licence\"); and so on\n  quot \n ");
  CU_ASSERT_STRING_EQUAL(te22Buffer, textBuffer);
  g_free(textBuffer);
  g_free(te22Buffer);
}

/**
 * \brief Test for convertWhitespaceToSpaceAndRemoveSpecialChars()
 * \test
 * Step 5: convert white-space to real spaces, and remove
 *         unnecessary punctuation, ala:
 * ==>  `tr -d '*=+#$|%.,:;!?()\\][\140\047\042' | tr '\011\012\015' '   '`
 *****
 * \note we purposely do NOT process backspace-characters here.  Perhaps
 * there's an improvement in the wings for this?
 */
void test_5_convertWhitespaceToSpaceAndRemoveSpecialChars()
{
  initializeCurScan(&cur);
  char* textBuffer;
  int isCR = NO; // isCR switches the replacment of ';' and '.', in the project it is always NO

  textBuffer = g_strdup_printf(" quot the big\t(C) and long    quot       , test"
              " &copy  string \n con-\n\377\377 tains losts; of . <string test   nbsp  "
              "  body  \" compli-\n cated /* COMMENT s and funny */ Words as it \n "
              "mimi-cs printf(\"Licence\"); and so on\n  quot \n ");

  convertWhitespaceToSpaceAndRemoveSpecialChars(textBuffer, isCR);

  char* te22Buffer = g_strdup_printf(" quot the big (C) and long    quot         test"
              " &copy  string   con- \377\377 tains losts  of \377         test   nbsp  "
              "  body    compli-  cated /  COMMENT s and funny  / Words as it   "
              "mimi-cs printf  Licence    and so on   quot   ");
  CU_ASSERT_STRING_EQUAL(te22Buffer, textBuffer);
  g_free(textBuffer);
  g_free(te22Buffer);

}


/**
 * \brief Test for dehyphen()
 *
 * Look for hyphenations of words, to compress both halves into a sin-
 * gle (sic) word.  Regex == "[a-z]- [a-z]".
 * \test
 * -# Create dirty string with hyphens
 * -# Call dehyphen() on the dirty string
 * -# Check if the string is cleaned
 *****
 * \note not sure this will work based on the way we strip punctuation
 * out of the buffer above -- work on this later.
 */
void test_6_dehyphen()
{
  initializeCurScan(&cur);
  char* buf, *fer;
  fer= g_strdup_printf("This- is  the-test str- ing");
  buf= g_strdup_printf("This\377\377is  the-test str\377\377ing");
  dehyphen(fer);
  CU_ASSERT_STRING_EQUAL(buf, fer);
  g_free(buf);
  g_free(fer);
}

/**
 * \brief Test for dehyphen()
 * \test
 * -# Create a string with mix of hyphens and INVISIBLE
 * -# Call dehyphen() and check the result
 */
void test_6a_dehyphen()
{
  initializeCurScan(&cur);
  char* textBuffer= g_strdup_printf(" quot the big (C) and long    quot       , "
              "test &copy  string   con- \377\377 tains losts  of \377         test"
              "   nbsp    body    compli-  cated /  COMMENT s and funny  / Words as it   "
              "mimi-cs printf  Licence    and so on   quot   ");
  dehyphen(textBuffer);

  char* te22Buffer = g_strdup_printf(" quot the big (C) and long    quot       , test &copy  "
              "string   con- \377\377 tains losts  of \377         test   nbsp    body    "
              "compli\377\377\377cated /  COMMENT s and funny  / Words as it   "
              "mimi-cs printf  Licence    and so on   quot   ");
  CU_ASSERT_STRING_EQUAL(te22Buffer, textBuffer);
  g_free(textBuffer);
  g_free(te22Buffer);

}

/**
 * \brief Test for removePunctuation()
 * \test
 * Step 6: clean up miscellaneous punctuation, ala:
 * ==>     `perl -pe 's,[-_/]+ , ,g;s/print[_a-zA-Z]* //g;s/  / /g;'`
 */
void test_7_removePunctuation()
{
  initializeCurScan(&cur);
  char* textBuffer= g_strdup_printf(" quot the big (C) and long    quot       , "
              "test &copy  string   con- \377\377 tains losts  of \377         test   "
              "nbsp    body    compli\377\377\377cated /  COMMENT s and funny  / Words as it   "
              "mimi-cs printf  Licence    and so on   quot   ");
 //! Note the added ',' so removePunctuation does ignore commas...
  removePunctuation(textBuffer);

  char* te22Buffer = g_strdup_printf(" quot the big (C) and long    quot       , test "
              "&copy  string   con  \377\377 tains losts  of \377         test   nbsp    body    "
              "compli\377\377\377cated    COMMENT s and funny    Words as it   "
              "mimi-cs printf  Licence    and so on   quot   ");
  CU_ASSERT_STRING_EQUAL(te22Buffer, textBuffer);
  g_free(textBuffer);
  g_free(te22Buffer);
}

/**
 * \brief Test for ignoreFunctionCalls()
 * \test
 * Ignore function calls to print routines: only concentrate on what's being
 * printed (sometimes programs do print licensing information) -- but don't
 * ignore real words that END in 'print', like footprint and fingerprint.
 * Here, we take a risk and just look for a 't' (in "footprint"), or for an
 * 'r' (in "fingerprint").  If someone has ever coded a print routine that
 * is named 'rprint' or tprint', we're spoofed.
 */
void test_8_ignoreFunctionCalls()
{
  initializeCurScan(&cur);
  char* textBuffer= g_strdup_printf(" quot the big (C) and long    quot       , test &copy  "
              "string   con  \377\377 tains losts  of \377         test   nbsp    body    "
              "compli\377\377\377cated    COMMENT s and funny    Words as it   "
              "mimi-cs printf  Licence    and so on   quot   ");
  ignoreFunctionCalls(textBuffer);

  char* te22Buffer = g_strdup_printf(" quot the big (C) and long    quot       , test &copy  "
              "string   con  \377\377 tains losts  of \377         test   nbsp    body    "
              "compli\377\377\377cated    COMMENT s and funny    Words as it   "
              "mimi-cs         Licence    and so on   quot   ");
  CU_ASSERT_STRING_EQUAL(te22Buffer, textBuffer);
  g_free(textBuffer);
  g_free(te22Buffer);
}

/**
 * \brief Test for convertSpaceToInvisible()
 * \test
 * -# Create string with many empty spaces
 * -# Call convertSpaceToInvisible()
 * -# Check if all spaces (>2) are replaced with INVISIBLE
 */
void test_9_convertSpaceToInvisible()
{
  initializeCurScan(&cur);
  char* textBuffer= g_strdup_printf(" quot the big (C) and long    quot       , test &copy  "
              "string   con  \377\377 tains losts  of \377         test   nbsp    body    "
              "compli\377\377\377cated    COMMENT s and funny    Words as it   "
              "mimi-cs         Licence    and so on   quot   ");
  convertSpaceToInvisible(textBuffer);

  char* te22Buffer = g_strdup_printf(" quot the big (C) and long \377\377\377quot \377\377\377\377\377\377, "
              "test &copy \377string \377\377con \377\377\377\377tains losts \377of \377\377\377\377\377\377\377\377\377\377"
              "test \377\377nbsp \377\377\377body \377\377\377compli\377\377\377cated \377\377\377COMMENT s "
              "and funny \377\377\377Words as it \377\377mimi-cs \377\377\377\377\377\377\377\377Licence \377\377\377"
              "and so on \377\377quot \377\377");
//  printf("%s\n", te22Buffer);
  CU_ASSERT_STRING_EQUAL(te22Buffer, textBuffer);
  g_free(textBuffer);
  g_free(te22Buffer);
}

/**
 * \brief Test for compressDoctoredBuffer()
 * \test
 * -# garbage collect: eliminate all INVISIBLE characters in the buffer
 */
void test_10_compressDoctoredBuffer()
{
  initializeCurScan(&cur);
  char* textBuffer= g_strdup_printf(" quot the big (C) and long \377\377\377quot \377\377\377\377\377\377, "
              "test &copy \377string \377\377con \377\377\377\377tains losts \377of \377\377\377\377\377\377\377\377\377\377"
              "test \377\377nbsp \377\377\377body \377\377\377compli\377\377\377cated \377\377\377COMMENT s "
              "and funny \377\377\377Words as it \377\377mimi-cs \377\377\377\377\377\377\377\377Licence \377\377\377"
              "and so on \377\377quot \377\377");

  compressDoctoredBuffer(textBuffer);

  char* te22Buffer = g_strdup_printf(" quot the big (C) and long quot , test &copy string "
              "con tains losts of test nbsp body complicated COMMENT s and funny Words as "
              "it mimi-cs Licence and so on quot ");

  CU_ASSERT_STRING_EQUAL(te22Buffer, textBuffer);
  g_free(textBuffer);
  g_free(te22Buffer);
}

CU_TestInfo doctorBuffer_testcases[] =
{
{ "Testing doctorBuffer:", test_doctorBuffer },
{ "Testing doctorBufer_uncollapse", test_doctorBuffer_uncollapse },
{ "Testing doctorBuffer_fromFile", test_doctorBuffer_fromFile },
{ "Testing removeHtmlComents:", test_1_removeHtmlComments },
{ "Testing removeLineComments:", test_2_removeLineComments },
{ "Testing cleanUpPostscript:", test_3_cleanUpPostscript },
{ "Testing removeBackslashes:", test_4_removeBackslashesAndGTroffIndicators },
{ "Testing convertWhitespace:", test_5_convertWhitespaceToSpaceAndRemoveSpecialChars },
{ "Testing dehyphen:", test_6_dehyphen },
{ "Testing dehyphen2:", test_6a_dehyphen },
{ "Testing removePunctuation:", test_7_removePunctuation },
{ "Testing ignoreFunctionCalls:", test_8_ignoreFunctionCalls },
{ "Testing convertSpaceToInvisible:", test_9_convertSpaceToInvisible },
{ "Testing compressDoctoredBuffer:", test_10_compressDoctoredBuffer },
CU_TEST_INFO_NULL };

