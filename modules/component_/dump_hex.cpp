#include "dump_hex.h"

std::string DumpHex2String(const void *p, unsigned len)
{
    const unsigned char *bytes = (const unsigned char*)p;
    std::string buf;
    buf.reserve(len*5);
    static const unsigned STEP = 16;

    unsigned base = 0;
    while( base < len)
    {
        char apd[128] = {0};
        snprintf(apd, sizeof(apd), "%04X: ", base);
        buf.append(apd);
        for( unsigned index = 0; index < STEP; ++index)
        {
            if( (base + index) >= len)
            {
                buf += "   ";
                if( (index + 1) == STEP)
                    continue;
                if( ((base + index + 1) % 4) == 0)
                {
                    buf += "  ";
                }
            }
            else
            {
                snprintf(apd, sizeof(apd), "%02X ", bytes[base+index]);
                buf.append(apd);
                if( (index + 1) == STEP)
                    continue;
                if( ((base + index + 1) % 4) == 0)
                {
                    if( (base + index + 1) >= len)
                        buf += "  ";
                    else
                        buf += "- ";
                }
            }
        }
        buf += "[";
        for( unsigned index = 0; index < STEP; ++index)
        {
            if( (base + index) < len)
            {
                unsigned char c = bytes[base + index];
                if( (32 <= c) && (c <= 127))
                    buf.push_back(c);
                else
                    buf += ".";
            }
            else
                break;
        }
        buf += "]\n";
        base += STEP;
    }
    return buf;
}
