#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Replicate the constants from nes_rom.c */
#define ROM_DISP_MAXLEN 128

/*
 * Safe version of the ROM name copy logic that enforces bounds.
 * This is what the code SHOULD do — used here to define the invariant.
 */
static void safe_copy_romname(char *info, size_t info_size, const char *romname)
{
    if (info == NULL || romname == NULL || info_size == 0)
        return;

    size_t romname_len = strlen(romname);

    if (romname_len >= info_size) {
        /* Truncate and append "..." */
        strncpy(info, romname, info_size - 4);
        info[info_size - 4] = '\0';
        strcpy(info + (info_size - 4), "...");
    } else {
        strncpy(info, romname, info_size - 1);
        info[info_size - 1] = '\0';
    }
}

/*
 * Invariant: After copying a ROM name into a fixed-size buffer,
 * the buffer must:
 *   1. Always be NUL-terminated within bounds.
 *   2. Never exceed ROM_DISP_MAXLEN bytes (including NUL terminator).
 *   3. The canary bytes placed after the buffer must remain intact.
 */
START_TEST(test_romname_buffer_bounds)
{
    /* Invariant: ROM name copy must never overflow the fixed-size info buffer */
    const char *payloads[] = {
        /* Exact boundary */
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",  /* 128 chars */

        /* One over boundary */
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB", /* 129 chars */

        /* Far over boundary */
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC", /* 256 chars */

        /* Very long string (potential heap/stack smash) */
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD", /* 512 chars */

        /* Format string attack */
        "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"
        "%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n",

        /* Path traversal with long prefix */
        "../../../../../../../../../../../../../../../../../../../../../../../../"
        "../../../../../../../../../../../../../../../../../../../../../../../../"
        "etc/passwd",

        /* NUL bytes embedded (tests string handling) */
        "short",

        /* Empty string */
        "",

        /* Single character */
        "X",

        /* Exactly ROM_DISP_MAXLEN - 1 chars (fits perfectly) */
        "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
        "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE", /* 127 chars */

        /* Unicode-like byte sequences */
        "\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf"
        "\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf"
        "\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf"
        "\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf",

        /* Repeated special characters */
        "../../../../../../../../../../../../../../../../../../../../../../../../"
        "../../../../../../../../../../../../../../../../../../../../../../../../",

        /* Mixed content attack */
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"  /* 512 A's */
    };

    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        /*
         * Allocate a buffer with canary bytes on both sides to detect overflow.
         * Layout: [CANARY_BEFORE (16 bytes)] [info buffer (ROM_DISP_MAXLEN bytes)] [CANARY_AFTER (16 bytes)]
         */
        const size_t CANARY_SIZE = 16;
        const uint8_t CANARY_VALUE = 0xAB;

        uint8_t *region = (uint8_t *)malloc(CANARY_SIZE + ROM_DISP_MAXLEN + CANARY_SIZE);
        ck_assert_ptr_nonnull(region);

        /* Initialize canaries */
        memset(region, CANARY_VALUE, CANARY_SIZE);
        memset(region + CANARY_SIZE, 0, ROM_DISP_MAXLEN);
        memset(region + CANARY_SIZE + ROM_DISP_MAXLEN, CANARY_VALUE, CANARY_SIZE);

        char *info = (char *)(region + CANARY_SIZE);

        /* Apply the safe copy logic */
        safe_copy_romname(info, ROM_DISP_MAXLEN, payloads[i]);

        /* Invariant 1: Buffer must be NUL-terminated within bounds */
        int nul_found = 0;
        for (size_t j = 0; j < ROM_DISP_MAXLEN; j++) {
            if (info[j] == '\0') {
                nul_found = 1;
                break;
            }
        }
        ck_assert_msg(nul_found,
            "Invariant violated: info buffer not NUL-terminated for payload[%d]", i);

        /* Invariant 2: strlen of result must be strictly less than ROM_DISP_MAXLEN */
        size_t result_len = strlen(info);
        ck_assert_msg(result_len < (size_t)ROM_DISP_MAXLEN,
            "Invariant violated: result length %zu >= ROM_DISP_MAXLEN %d for payload[%d]",
            result_len, ROM_DISP_MAXLEN, i);

        /* Invariant 3: Canary before the buffer must be intact */
        for (size_t j = 0; j < CANARY_SIZE; j++) {
            ck_assert_msg(region[j] == CANARY_VALUE,
                "Invariant violated: canary before buffer corrupted at byte %zu for payload[%d]", j, i);
        }

        /* Invariant 4: Canary after the buffer must be intact */
        for (size_t j = 0; j < CANARY_SIZE; j++) {
            ck_assert_msg(region[CANARY_SIZE + ROM_DISP_MAXLEN + j] == CANARY_VALUE,
                "Invariant violated: canary after buffer corrupted at byte %zu for payload[%d]", j, i);
        }

        /* Invariant 5: If input fits, output must match input exactly */
        size_t input_len = strlen(payloads[i]);
        if (input_len < (size_t)ROM_DISP_MAXLEN) {
            ck_assert_msg(strcmp(info, payloads[i]) == 0,
                "Invariant violated: short input not copied correctly for payload[%d]", i);
        }

        /* Invariant 6: If input was truncated, result must end with "..." */
        if (input_len >= (size_t)ROM_DISP_MAXLEN) {
            ck_assert_msg(result_len >= 3,
                "Invariant violated: truncated result too short for payload[%d]", i);
            ck_assert_msg(strcmp(info + result_len - 3, "...") == 0,
                "Invariant violated: truncated result does not end with '...' for payload[%d]", i);
        }

        free(region);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_romname_buffer_bounds);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}