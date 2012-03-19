/*-----------------------------------------------------------------------------
 *  int_utils.cpp - Utilities used by encoders/decodes
 *
 *  Coding-Style:
 *      emacs) Mode: C, tab-width: 8, c-basic-offset: 8, indent-tabs-mode: nil
 *      vi) tabstop: 8, expandtab
 *
 *  Authors:
 *      Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *      Fabrizio Silvestri <fabrizio.silvestri_at_isti.cnr.it>
 *      Rossano Venturini <rossano.venturini_at_isti.cnr.it>
 *-----------------------------------------------------------------------------
 */

#include "util/compression/int/int_utils.hpp"

int
int_utils::get_msb(uint32_t v)
{
        return (v != 0)? __log2_uint32(v) : 0;
}

uint32_t
int_utils::div_roundup(uint32_t v, uint32_t div)
{
        return (v + (div - 1)) / div;
}

double
int_utils::get_time(void)
{
        double  utime;
        double  stime;
        struct rusage   usage;

        getrusage (RUSAGE_SELF, &usage);

        utime = (double)usage.ru_utime.tv_sec +
                (double)usage.ru_utime.tv_usec / 1000000.0;

        stime = (double)usage.ru_stime.tv_sec +
                (double)usage.ru_stime.tv_usec / 1000000.0;

        return (utime + stime);
}

uint32_t
*int_utils::open_and_mmap_file(char *filen,
                bool write, uint64_t &len) {
        int             file;
        int             ret;
        uint32_t        *addr;
        struct stat     sb;

        if (write)
                file = open(filen, O_RDWR);
        else
                file = open(filen, O_RDONLY);

        if (file == -1)
                eoutput("oepn(): Can't open the file");

        /* Do mmap() for file */
        ret = fstat(file, &sb);
        if (ret == -1 || sb.st_size == 0)
                eoutput("fstat(): Unknown the file size");

        len = sb.st_size;

        __fadvise_sequential(file, len);

        if (write)
                addr = (uint32_t *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, file, 0);
        else
                addr = (uint32_t *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, file, 0);

        if (addr == MAP_FAILED)
                eoutput("mmap(): Can't map the file to memory");

        close(file);

        return addr;
}

void
int_utils::close_file(uint32_t *adr, uint64_t len)
{
        munmap(adr, len);
}
