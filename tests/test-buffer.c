/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2009-2010, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the LICENSE.txt file in this distribution for license
 * details.
 * ----------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <check.h>

#include "libcork/core/allocator.h"
#include "libcork/core/types.h"
#include "libcork/ds/buffer.h"
#include "libcork/ds/managed-buffer.h"


/*-----------------------------------------------------------------------
 * Buffers
 */

START_TEST(test_buffer)
{
    cork_allocator_t  *alloc = cork_allocator_new_debug();

    static char  SRC[] =
        "Here is some text.";
    size_t  SRC_LEN = strlen(SRC);

    cork_buffer_t  buffer1;
    cork_buffer_init(alloc, &buffer1);
    fail_unless(cork_buffer_set(&buffer1, SRC, SRC_LEN+1),
                "Could not set contents of buffer");

    cork_buffer_t  *buffer2 = cork_buffer_new(alloc);
    fail_if(buffer2 == NULL,
            "Could not allocate buffer");
    fail_unless(cork_buffer_set_string(buffer2, SRC),
                "Could not set string contents of buffer");

    fail_unless(cork_buffer_equal(&buffer1, buffer2),
                "Buffers should be equal: got %zu:%s, expected %zu:%s",
                buffer1.size, buffer1.buf, buffer2->size, buffer2->buf);

    cork_buffer_t  *buffer3 = cork_buffer_new(alloc);
    fail_if(buffer3 == NULL,
            "Could not allocate buffer");
    fail_unless(cork_buffer_printf(buffer3, "Here is %s text.", "some"),
                "Could not format into buffer");

    fail_unless(cork_buffer_equal(&buffer1, buffer3),
                "Buffers should be equal: got %zu:%s, expected %zu:%s",
                buffer1.size, buffer1.buf, buffer3->size, buffer3->buf);

    cork_buffer_done(&buffer1);
    cork_buffer_free(buffer2);
    cork_buffer_free(buffer3);
    cork_allocator_free(alloc);
}
END_TEST


START_TEST(test_buffer_append)
{
    cork_allocator_t  *alloc = cork_allocator_new_debug();

    static char  SRC1[] = "abcd";
    size_t  SRC1_LEN = 4;
    static char  SRC2[] = "efg";
    size_t  SRC2_LEN = 3;
    static char  SRC3[] = "hij";
    static char  SRC4[] = "kl";

    cork_buffer_t  buffer1;
    cork_buffer_init(alloc, &buffer1);

    /*
     * Let's try setting some data, then clearing it, before we do our
     * appends.
     */

    fail_unless(cork_buffer_set(&buffer1, SRC2, SRC2_LEN),
                "Could not set contents of buffer");
    cork_buffer_clear(&buffer1);

    /*
     * Okay now do the appends.
     */

    fail_unless(cork_buffer_append(&buffer1, SRC1, SRC1_LEN),
                "Could not set contents of buffer");
    fail_unless(cork_buffer_append(&buffer1, SRC2, SRC2_LEN),
                "Could not append contents of buffer");
    fail_unless(cork_buffer_append_string(&buffer1, SRC3),
                "Could not append first string");
    fail_unless(cork_buffer_append_string(&buffer1, SRC4),
                "Could not append second string");

    static char  EXPECTED[] = "abcdefghijkl";

    cork_buffer_t  buffer2;
    cork_buffer_init(alloc, &buffer2);
    cork_buffer_set_string(&buffer2, EXPECTED);

    fail_unless(cork_buffer_equal(&buffer1, &buffer2),
                "Buffers should be equal: got %zu:%s, expected %zu:%s",
                buffer1.size, buffer1.buf, buffer2.size, buffer2.buf);

    cork_buffer_t  *buffer3 = cork_buffer_new(alloc);
    cork_buffer_set(buffer3, SRC1, SRC1_LEN);
    fail_unless(cork_buffer_append_printf(buffer3, "%s%s%s",
                                          SRC2, SRC3, SRC4),
                "Could not append formatted string into buffer");

    fail_unless(cork_buffer_equal(&buffer1, buffer3),
                "Buffers should be equal: got %zu:%s, expected %zu:%s",
                buffer1.size, buffer1.buf, buffer3->size, buffer3->buf);

    cork_buffer_done(&buffer1);
    cork_buffer_done(&buffer2);
    cork_buffer_free(buffer3);
    cork_allocator_free(alloc);
}
END_TEST


START_TEST(test_buffer_slicing)
{
    cork_allocator_t  *alloc = cork_allocator_new_debug();

    static char  SRC[] =
        "Here is some text.";

    cork_buffer_t  *buffer = cork_buffer_new(alloc);
    fail_unless(cork_buffer_set_string(buffer, SRC),
                "Could not set string contents of buffer");

    cork_managed_buffer_t  *managed =
        cork_buffer_to_managed_buffer(buffer);
    fail_if(managed == NULL,
            "Cannot manage buffer");
    cork_managed_buffer_unref(managed);

    buffer = cork_buffer_new(alloc);
    fail_unless(cork_buffer_set_string(buffer, SRC),
                "Could not set string contents of buffer");

    cork_slice_t  slice;
    fail_unless(cork_buffer_to_slice(buffer, &slice),
                "Cannot slice buffer");
    cork_slice_finish(&slice);

    cork_allocator_free(alloc);
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("buffer");

    TCase  *tc_buffer = tcase_create("buffer");
    tcase_add_test(tc_buffer, test_buffer);
    tcase_add_test(tc_buffer, test_buffer_append);
    tcase_add_test(tc_buffer, test_buffer_slicing);
    suite_add_tcase(s, tc_buffer);

    return s;
}


int
main(int argc, const char **argv)
{
    int  number_failed;
    Suite  *suite = test_suite();
    SRunner  *runner = srunner_create(suite);

    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0)? EXIT_SUCCESS: EXIT_FAILURE;
}
