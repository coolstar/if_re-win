#pragma once

enum
{
    MEDIUM_INDEX_AUTO = 0,
    MEDIUM_INDEX_10HD,
    MEDIUM_INDEX_10FD,
    MEDIUM_INDEX_100HD,
    MEDIUM_INDEX_100FD,
    MEDIUM_INDEX_100FDFC,
    MEDIUM_INDEX_1000FD,
    MEDIUM_INDEX_1000FDFC,
    MEDIUM_INDEX_100FDEEE,
    MEDIUM_INDEX_100FDFCEEE,
    MEDIUM_INDEX_1000FDEEE,
    MEDIUM_INDEX_1000FDFCEEE,
    MEDIUM_INDEX_2500FD,
    MEDIUM_INDEX_2500FDFC,
    MEDIUM_INDEX_COUNT
};

#define MBit 1000000ULL

enum {
    kSpeed2500MBit = 2500 * MBit,
    kSpeed1000MBit = 1000 * MBit,
    kSpeed100MBit = 100 * MBit,
    kSpeed10MBit = 10 * MBit,
};

enum {
    kFlowControlOff = 0,
    kFlowControlOn = 0x01
};

enum {
    kEEETypeNo = 0,
    kEEETypeYes = 1,
    kEEETypeCount
};