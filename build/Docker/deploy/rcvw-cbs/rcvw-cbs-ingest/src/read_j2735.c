#include <MessageFrame.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    char buf[4096];
    if (argc > 1)
        memcpy(buf, argv[1], strlen(argv[1]));
    else
        scanf("%s", buf);

    // Convert to bytes
    int len = strlen(buf);
    int i, x;
    for (i = 0, x = 0; i < len; i+=2, x++) {
        char tmp[3];
        tmp[0] = buf[i];
        tmp[1] = buf[i+1];
        tmp[2] = '\0';

        buf[x] = (char)strtol(tmp, NULL, 16);
    }

    struct MessageFrame *frame = NULL;
    asn_dec_rval_t ret = asn_decode(NULL, ATS_UNALIGNED_BASIC_PER, &asn_DEF_MessageFrame, (void **)&frame, buf, x);
    if (ret.code)
        return (-ret.code);

    xer_fprint(stdout, &asn_DEF_MessageFrame, frame);

    ASN_STRUCT_FREE(asn_DEF_MessageFrame, frame);
}
