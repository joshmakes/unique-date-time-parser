#include "formatValidator.h"

#include <ctype.h>
#include <string.h>

bool isValidLen(size_t len) {
    return (len == MIN_DATE_TIME_STR_LEN || len == MAX_DATE_TIME_STR_LEN);
}

bool isValidDateTime(const char *const str, size_t len) {
    bool valid = true;

    if (!isValidLen(len) || str == NULL) {
        return false;
    }

    /// Common format
    // YYYY
    const int i0 = isdigit((unsigned char)str[0]); const int i1 = isdigit((unsigned char)str[1]);
    const int i2 = isdigit((unsigned char)str[2]); const int i3 = isdigit((unsigned char)str[3]);
    valid &= (i0 && i1 && i2 && i3);

    // '-'
    valid &= (str[4] == '-');

    // MM
    const int i5 = isdigit((unsigned char)str[5]); const int i6 = isdigit((unsigned char)str[6]);
    valid &= (i5 && i6);

    // '-'
    valid &= (str[7] == '-');

    // DD
    const int i8 = isdigit((unsigned char)str[8]); const int i9 = isdigit((unsigned char)str[9]);
    valid &= (i8 && i9);

    // 'T'
    valid &= (str[10] == 'T');

    // HH
    const int i11 = isdigit((unsigned char)str[11]); const int i12 = isdigit((unsigned char)str[12]);
    valid &= (i11 && i12);

    // ':'
    valid &= (str[13] == ':');

    // MM
    const int i14 = isdigit((unsigned char)str[14]); const int i15 = isdigit((unsigned char)str[15]);
    valid &= (i14 && i15);

    // ':'
    valid &= (str[16] == ':');

    // SS
    const int i17 = isdigit((unsigned char)str[17]); const int i18 = isdigit((unsigned char)str[18]);
    valid &= (i17 && i18);

    if (len == MIN_DATE_TIME_STR_LEN) {
        // Short format: YYYY-MM-DDTHH:MM:SSZ
        valid &= (str[19] == 'Z');
    } else {
        // Long format: YYYY-MM-DDTHH:MM:SS±HH:MM
        // '+' or '-'
        valid &= (str[19] == '+' || str[19] == '-');

        // offset HH
        const int i20 = isdigit((unsigned char)str[20]); const int i21 = isdigit((unsigned char)str[21]);
        valid &= (i20 && i21);

        // ':'
        valid &= (str[22] == ':');

        // offset MM
        const int i23 = isdigit((unsigned char)str[23]); const int i24 = isdigit((unsigned char)str[24]);
        valid &= (i23 && i24);
    }

    return valid;
}
