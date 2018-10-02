#pragma once
#include <Types.h>
#include <tuple>

#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

namespace Azura {
enum Bits
{
  BitsUnknown = 0,
  Bits1       = 1,
  Bits2       = 2,
  Bits3       = 3,
  Bits4       = 4,
  Bits5       = 5,
  Bits6       = 6,
  Bits7       = 7,
  Bits8       = 8,
  Bits16      = 16,
  Bits32      = 32
};

enum FormatFeature : U32
{
  Compressed = 1,
  MultiBlock = 1 << 1,
};

enum class ChannelDataType
{
  Float = 0,
  Integer,
  UNorm,
  SNorm,
  UScaled,
  SScaled,
  SRGB,
  Typeless
};

enum class Channel
{
  Red = 0,
  Green,
  Blue,
  Alpha,
  Depth,
  Stencil,
  Padding,
  Unknown,
};

using ChannelDesc = std::tuple<Channel, ChannelDataType, U32>;

// Tuple Access Info
#define TUPLE_ACCESS_ENUM 0
#define TUPLE_ACCESS_CHANNEL_DATA 1
#define TUPLE_SIZE 2

#define CHANNEL_TUPLE_ACCESS_CHANNEL 0
#define CHANNEL_TUPLE_ACCESS_TYPE 1
#define CHANNEL_TUPLE_ACCESS_BIT_WIDTH 2
#define CHANNEL_TUPLE_SIZE 3

// Format Iterator
#define RAW_STORAGE_FORMAT_ITERATOR(FUNC)                                                                                                                                                        \
  FUNC((UNKNOWN,                    /* CHANNELS START */ ((Unknown, Typeless, 0))                                                                                          /* CHANNELS END */))  \
  FUNC((B8G8R8A8_SINT,              /* CHANNELS START */ ((Blue, SInt, 8))          ((Green, SInt, 8))          ((Red, SInt, 8))              ((Alpha, SInt, 8))           /* CHANNELS END */))  \
  FUNC((B8G8R8A8_SNORM,             /* CHANNELS START */ ((Blue, SNorm, 8))         ((Green, SNorm, 8))         ((Red, SNorm, 8))             ((Alpha, SNorm, 8))          /* CHANNELS END */))  \
  FUNC((B8G8R8A8_SRGB,              /* CHANNELS START */ ((Blue, SRGB, 8))          ((Green, SRGB, 8))          ((Red, SRGB, 8))              ((Alpha, SRGB, 8))           /* CHANNELS END */))  \
  FUNC((B8G8R8A8_SSCALED,           /* CHANNELS START */ ((Blue, SScaled, 8))       ((Green, SScaled, 8))       ((Red, SScaled, 8))           ((Alpha, SScaled, 8))        /* CHANNELS END */))  \
  FUNC((B8G8R8A8_UINT,              /* CHANNELS START */ ((Blue, UInt, 8))          ((Green, UInt, 8))          ((Red, UInt, 8))              ((Alpha, UInt, 8))           /* CHANNELS END */))  \
  FUNC((B8G8R8A8_UNORM,             /* CHANNELS START */ ((Blue, UNorm, 8))         ((Green, UNorm, 8))         ((Red, UNorm, 8))             ((Alpha, UNorm, 8))          /* CHANNELS END */))  \
  FUNC((B8G8R8A8_USCALED,           /* CHANNELS START */ ((Blue, UScaled, 8))       ((Green, UScaled, 8))       ((Red, UScaled, 8))           ((Alpha, UScaled, 8))        /* CHANNELS END */))  \
  FUNC((D24_UNORM_S8_UINT,          /* CHANNELS START */ ((Depth, UNorm, 24))       ((Stencil, UInt, 8))                                                                   /* CHANNELS END */))  \
  FUNC((D32_FLOAT,                  /* CHANNELS START */ ((Depth, Float, 32))                                                                                              /* CHANNELS END */))  \
  FUNC((D32_FLOAT_S8X24_UINT,       /* CHANNELS START */ ((Depth, Float, 32))       ((Stencil, UInt, 8))        ((Padding, Float, 24))                                     /* CHANNELS END */))  \
  FUNC((R8G8B8A8_UNORM,             /* CHANNELS START */ ((Red, UNorm, 8))          ((Green, UNorm, 8))         ((Blue, UNorm, 8))            ((Alpha, UNorm, 8))          /* CHANNELS END */))  \
  FUNC((R11G11B10_FLOAT,            /* CHANNELS START */ ((Red, Float, 11))         ((Green, Float, 11))        ((Blue, Float, 10))                                        /* CHANNELS END */))  \
  FUNC((R16_FLOAT,                  /* CHANNELS START */ ((Red, Float, 16))                                                                                                /* CHANNELS END */))  \
  FUNC((R16_SINT,                   /* CHANNELS START */ ((Red, SInt, 16))                                                                                                 /* CHANNELS END */))  \
  FUNC((R16_SNORM,                  /* CHANNELS START */ ((Red, SNorm, 16))                                                                                                /* CHANNELS END */))  \
  FUNC((R16_SSCALED,                /* CHANNELS START */ ((Red, SScaled, 16))                                                                                              /* CHANNELS END */))  \
  FUNC((R16_UINT,                   /* CHANNELS START */ ((Red, UInt, 16))                                                                                                 /* CHANNELS END */))  \
  FUNC((R16_UNORM,                  /* CHANNELS START */ ((Red, UNorm, 16))                                                                                                /* CHANNELS END */))  \
  FUNC((R16_USCALED,                /* CHANNELS START */ ((Red, UScaled, 16))                                                                                              /* CHANNELS END */))  \
  FUNC((R16G16_FLOAT,               /* CHANNELS START */ ((Red, Float, 16))         ((Green, Float, 16))                                                                   /* CHANNELS END */))  \
  FUNC((R16G16_SINT,                /* CHANNELS START */ ((Red, SInt, 16))          ((Green, SInt, 16))                                                                    /* CHANNELS END */))  \
  FUNC((R16G16_SNORM,               /* CHANNELS START */ ((Red, SNorm, 16))         ((Green, SNorm, 16))                                                                   /* CHANNELS END */))  \
  FUNC((R16G16_SSCALED,             /* CHANNELS START */ ((Red, SScaled, 16))       ((Green, SScaled, 16))                                                                 /* CHANNELS END */))  \
  FUNC((R16G16_UINT,                /* CHANNELS START */ ((Red, UInt, 16))          ((Green, UInt, 16))                                                                    /* CHANNELS END */))  \
  FUNC((R16G16_UNORM,               /* CHANNELS START */ ((Red, UNorm, 16))         ((Green, UNorm, 16))                                                                   /* CHANNELS END */))  \
  FUNC((R16G16_USCALED,             /* CHANNELS START */ ((Red, UScaled, 16))       ((Green, UScaled, 16))                                                                 /* CHANNELS END */))  \
  FUNC((R16G16B16_FLOAT,            /* CHANNELS START */ ((Red, Float, 16))         ((Green, Float, 16))        ((Blue, Float, 16))                                        /* CHANNELS END */))  \
  FUNC((R16G16B16_SINT,             /* CHANNELS START */ ((Red, SInt, 16))          ((Green, SInt, 16))         ((Blue, SInt, 16))                                         /* CHANNELS END */))  \
  FUNC((R16G16B16_SNORM,            /* CHANNELS START */ ((Red, SNorm, 16))         ((Green, SNorm, 16))        ((Blue, SNorm, 16))                                        /* CHANNELS END */))  \
  FUNC((R16G16B16_SSCALED,          /* CHANNELS START */ ((Red, SScaled, 16))       ((Green, SScaled, 16))      ((Blue, SScaled, 16))                                      /* CHANNELS END */))  \
  FUNC((R16G16B16_UINT,             /* CHANNELS START */ ((Red, UInt, 16))          ((Green, UInt, 16))         ((Blue, UInt, 16))                                         /* CHANNELS END */))  \
  FUNC((R16G16B16_UNORM,            /* CHANNELS START */ ((Red, UNorm, 16))         ((Green, UNorm, 16))        ((Blue, UNorm, 16))                                        /* CHANNELS END */))  \
  FUNC((R16G16B16_USCALED,          /* CHANNELS START */ ((Red, UScaled, 16))       ((Green, UScaled, 16))      ((Blue, UScaled, 16))                                      /* CHANNELS END */))  \
  FUNC((R16G16B16A16_FLOAT,         /* CHANNELS START */ ((Red, Float, 16))         ((Green, Float, 16))        ((Blue, Float, 16))           ((Alpha, Float, 16))         /* CHANNELS END */))  \
  FUNC((R16G16B16A16_SINT,          /* CHANNELS START */ ((Red, SInt, 16))          ((Green, SInt, 16))         ((Blue, SInt, 16))            ((Alpha, SInt, 16))          /* CHANNELS END */))  \
  FUNC((R16G16B16A16_SNORM,         /* CHANNELS START */ ((Red, SNorm, 16))         ((Green, SNorm, 16))        ((Blue, SNorm, 16))           ((Alpha, SNorm, 16))         /* CHANNELS END */))  \
  FUNC((R16G16B16A16_SSCALED,       /* CHANNELS START */ ((Red, SScaled, 16))       ((Green, SScaled, 16))      ((Blue, SScaled, 16))         ((Alpha, SScaled, 16))       /* CHANNELS END */))  \
  FUNC((R16G16B16A16_UINT,          /* CHANNELS START */ ((Red, UInt, 16))          ((Green, UInt, 16))         ((Blue, UInt, 16))            ((Alpha, UInt, 16))          /* CHANNELS END */))  \
  FUNC((R16G16B16A16_UNORM,         /* CHANNELS START */ ((Red, UNorm, 16))         ((Green, UNorm, 16))        ((Blue, UNorm, 16))           ((Alpha, UNorm, 16))         /* CHANNELS END */))  \
  FUNC((R16G16B16A16_USCALED,       /* CHANNELS START */ ((Red, UScaled, 16))       ((Green, UScaled, 16))      ((Blue, UScaled, 16))         ((Alpha, UScaled, 16))       /* CHANNELS END */))  \
  FUNC((R32_FLOAT,                  /* CHANNELS START */ ((Red, Float, 32))                                                                                                /* CHANNELS END */))  \
  FUNC((R32_SINT,                   /* CHANNELS START */ ((Red, SInt, 32))                                                                                                 /* CHANNELS END */))  \
  FUNC((R32_SNORM,                  /* CHANNELS START */ ((Red, SNorm, 32))                                                                                                /* CHANNELS END */))  \
  FUNC((R32_UINT,                   /* CHANNELS START */ ((Red, UInt, 32))                                                                                                 /* CHANNELS END */))  \
  FUNC((R32_UNORM,                  /* CHANNELS START */ ((Red, UNorm, 32))                                                                                                /* CHANNELS END */))  \
  FUNC((R32G32_FLOAT,               /* CHANNELS START */ ((Red, Float, 32))         ((Green, Float, 32))                                                                   /* CHANNELS END */))  \
  FUNC((R32G32_SINT,                /* CHANNELS START */ ((Red, SInt, 32))          ((Green, SInt, 32))                                                                    /* CHANNELS END */))  \
  FUNC((R32G32_UINT,                /* CHANNELS START */ ((Red, UInt, 32))          ((Green, UInt, 32))                                                                    /* CHANNELS END */))  \
  FUNC((R32G32B32_FLOAT,            /* CHANNELS START */ ((Red, Float, 32))         ((Green, Float, 32))        ((Blue, Float, 32))                                        /* CHANNELS END */))  \
  FUNC((R32G32B32_SINT,             /* CHANNELS START */ ((Red, SInt, 32))          ((Green, SInt, 32))         ((Blue, SInt, 32))                                         /* CHANNELS END */))  \
  FUNC((R32G32B32_UINT,             /* CHANNELS START */ ((Red, UInt, 32))          ((Green, UInt, 32))         ((Blue, UInt, 32))                                         /* CHANNELS END */))  \
  FUNC((R32G32B32A32_FLOAT,         /* CHANNELS START */ ((Red, Float, 32))         ((Green, Float, 32))        ((Blue, Float, 32))           ((Alpha, Float, 32))         /* CHANNELS END */))  \
  FUNC((R32G32B32A32_SINT,          /* CHANNELS START */ ((Red, UInt, 32))          ((Green, UInt, 32))         ((Blue, UInt, 32))            ((Alpha, UInt, 32))          /* CHANNELS END */))  \
  FUNC((R32G32B32A32_UINT,          /* CHANNELS START */ ((Red, UInt, 32))          ((Green, UInt, 32))         ((Blue, UInt, 32))            ((Alpha, UInt, 32))          /* CHANNELS END */))  \
  FUNC((R64_FLOAT,                  /* CHANNELS START */ ((Red, Float, 64))                                                                                                /* CHANNELS END */))  \
  FUNC((R64_SINT,                   /* CHANNELS START */ ((Red, SInt, 64))                                                                                                 /* CHANNELS END */))  \
  FUNC((R64_UINT,                   /* CHANNELS START */ ((Red, UInt, 64))                                                                                                 /* CHANNELS END */))  \
  FUNC((R64G64_FLOAT,               /* CHANNELS START */ ((Red, Float, 64))         ((Green, Float, 64))                                                                   /* CHANNELS END */))  \
  FUNC((R64G64_SINT,                /* CHANNELS START */ ((Red, SInt, 64))          ((Green, SInt, 64))                                                                    /* CHANNELS END */))  \
  FUNC((R64G64_UINT,                /* CHANNELS START */ ((Red, UInt, 64))          ((Green, UInt, 64))                                                                    /* CHANNELS END */))  \
  FUNC((R64G64B64_FLOAT,            /* CHANNELS START */ ((Red, Float, 64))         ((Green, Float, 64))        ((Blue, Float, 64))                                        /* CHANNELS END */))  \
  FUNC((R64G64B64_SINT,             /* CHANNELS START */ ((Red, SInt, 64))          ((Green, SInt, 64))         ((Blue, SInt, 64))                                         /* CHANNELS END */))  \
  FUNC((R64G64B64_UINT,             /* CHANNELS START */ ((Red, UInt, 64))          ((Green, UInt, 64))         ((Blue, UInt, 64))                                         /* CHANNELS END */))  \
  FUNC((R64G64B64A64_FLOAT,         /* CHANNELS START */ ((Red, Float, 64))         ((Green, Float, 64))        ((Blue, Float, 64))           ((Alpha, Float, 64))         /* CHANNELS END */))  \
  FUNC((R64G64B64A64_SINT,          /* CHANNELS START */ ((Red, SInt, 64))          ((Green, SInt, 64))         ((Blue, SInt, 64))            ((Alpha, SInt, 64))          /* CHANNELS END */))  \
  FUNC((R64G64B64A64_UINT,          /* CHANNELS START */ ((Red, UInt, 64))          ((Green, UInt, 64))         ((Blue, UInt, 64))            ((Alpha, UInt, 64))          /* CHANNELS END */))

// Macros For: ENUM NAME
#define GET_FORMAT_NAME_FROM_TUPLE(ENUM_TUPLE) BOOST_PP_TUPLE_ELEM(TUPLE_SIZE, TUPLE_ACCESS_ENUM, ENUM_TUPLE)
#define GET_FORMAT_NAME_FOR_LIST_FROM_TUPLE(ENUM_TUPLE) GET_FORMAT_NAME_FROM_TUPLE(ENUM_TUPLE),
#define GET_FULL_FORMAT_NAME_FROM_TUPLE(ENUM_TUPLE) RawStorageFormat::GET_FORMAT_NAME_FROM_TUPLE(ENUM_TUPLE)

// Macros For: CHANNEL WIDTH
#define GET_CHANNEL_WIDTH(CHANNEL_TUPLE)                                                                                                                                                  \
  BOOST_PP_TUPLE_ELEM(CHANNEL_TUPLE_SIZE, CHANNEL_TUPLE_ACCESS_BIT_WIDTH, CHANNEL_TUPLE)
#define GET_CHANNEL_WIDTH_SEQ(R, DATA, CHANNEL_TUPLE)                                                                                                                                     \
  BOOST_PP_TUPLE_ELEM(CHANNEL_TUPLE_SIZE, CHANNEL_TUPLE_ACCESS_BIT_WIDTH, CHANNEL_TUPLE) DATA
#define GET_FORMAT_WIDTH(CHANNEL_LIST) BOOST_PP_SEQ_FOR_EACH(GET_CHANNEL_WIDTH_SEQ, +, CHANNEL_LIST) 0

#define GET_FORMAT_WIDTH_FROM_TUPLE(ENUM_TUPLE)                                                                                                                                           \
  GET_FORMAT_WIDTH(BOOST_PP_TUPLE_ELEM(TUPLE_SIZE, TUPLE_ACCESS_CHANNEL_DATA, ENUM_TUPLE))
#define GET_FORMAT_WIDTH_FOR_LIST_FROM_TUPLE(ENUM_TUPLE) GET_FORMAT_WIDTH_FROM_TUPLE(ENUM_TUPLE),

/**
 *                                                                    \brief This enum represents a platform agnostic way of representing Raw Data.
 * It can be used in Images, Image Views, Buffers etc.
 */
enum class RawStorageFormat
{
  RAW_STORAGE_FORMAT_ITERATOR(GET_FORMAT_NAME_FOR_LIST_FROM_TUPLE)
};

U32 GetFormatSizeBits(RawStorageFormat format);
U32 GetFormatSize(RawStorageFormat format);

}  // namespace Azura