#include "precomp.h"
#include "adapter.h"

ULONG
ComputeCrc(
    _In_reads_(length) UCHAR const* buffer,
    UINT length
)
{
    ULONG crc = 0xffffffff;

    for (UINT i = 0; i < length; i++)
    {
        UCHAR curByte = buffer[i];

        for (UINT j = 0; j < 8; j++)
        {
            ULONG carry = ((crc & 0x80000000) ? 1 : 0) ^ (curByte & 0x01);
            crc <<= 1;
            curByte >>= 1;

            if (carry)
            {
                crc = (crc ^ 0x04c11db6) | carry;
            }
        }
    }

    return crc;
}

void
GetMulticastBit(
    _In_ NET_ADAPTER_LINK_LAYER_ADDRESS const* address,
    _Out_ _Post_satisfies_(*byte < MAX_NIC_MULTICAST_REG) UCHAR* byte,
    _Out_ UCHAR* value
)
/*++

Routine Description:

    For a given multicast address, returns the byte and bit in
    the card multicast registers that it hashes to. Calls
    ComputeCrc() to determine the CRC value.

--*/
{
    ULONG crc = ComputeCrc(address->Address, address->Length);

    // The bit number is now in the 6 most significant bits of CRC.
    UINT bitNumber = (UINT)((crc >> 26) & 0x3f);
    *byte = (UCHAR)(bitNumber / 8);
    *value = (UCHAR)((UCHAR)1 << (bitNumber % 8));
}