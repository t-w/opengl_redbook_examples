#include <stdio.h>
#include "vgl.h"

//#define DEBUG_VTARGA 1

namespace vtarga
{

#ifdef _MSC_VER
#pragma pack (push, 1)
#endif

struct
#ifdef __GNUC__
    __attribute__((__packed__))
#endif
    targa_header
{
    unsigned char           id_length;
    unsigned char           cmap_type;
    unsigned char           image_type;
    struct
#ifdef __GNUC__
        __attribute__((__packed__))
#endif
    {
        unsigned short      cmap_table_offset;
        unsigned short      cmap_entry_count;
        unsigned char       cmap_entry_size;
    } cmap_spec;
    struct
#ifdef __GNUC__
        __attribute__((__packed__))
#endif
    {
        unsigned short      x_origin;
        unsigned short      y_origin;
        unsigned short      width;
        unsigned short      height;
        unsigned char       bits_per_pixel;
        struct {
            unsigned char   alpha_depth : 4;
            unsigned char   image_origin : 2;
            unsigned char   : 2;
        };
    } image_spec;
};
#if DEBUG_VTARGA == 1
typedef struct targa_header targa_header_type;
#endif

#ifdef _MSC_VER
#pragma pack (pop)
#endif

static_assert ( sizeof(struct targa_header) == 18,
                "targa.cpp: Targa header not properly aligned!" );

#if DEBUG_VTARGA == 1
static void
    dump_targa_header ( const targa_header &header )
{
    printf ("header.id_length:  %d, offset: %d\n"
            "header.cmap_type:  %d, offset: %d\n"
            "header.image_type: %d, offset: %d\n"
            "header.cmap_spec.cmap_table_offset: %d, offset: %d\n"
            "header.cmap_spec.cmap_entry_count:  %d, offset: %d\n"
            "header.cmap_spec.cmap_entry_size:   %d, offset: %d\n"
            "header.image_spec.x_origin:   %d, offset: %d\n"
            "header.image_spec.y_origin:   %d, offset: %d\n"
            "header.image_spec.width:      %d, offset: %d\n"
            "header.image_spec.height:     %d, offset: %d\n"
            "header.image_spec.bits_per_pixel: %d, offset: %d\n",
            header.id_length,  offsetof(targa_header_type, id_length),
            header.cmap_type,  offsetof(targa_header_type, cmap_type),
            header.image_type, offsetof(targa_header_type, image_type),
            header.cmap_spec.cmap_table_offset,
            offsetof ( targa_header_type, cmap_spec.cmap_table_offset ),
            header.cmap_spec.cmap_entry_count,
            offsetof ( targa_header_type, cmap_spec.cmap_entry_count ),
            header.cmap_spec.cmap_entry_size,
            offsetof ( targa_header_type, cmap_spec.cmap_entry_size ),
            header.image_spec.x_origin,
            offsetof ( targa_header_type, image_spec.x_origin ),
            header.image_spec.y_origin,
            offsetof ( targa_header_type, image_spec.y_origin ),
            header.image_spec.width,
            offsetof ( targa_header_type, image_spec.width ),
            header.image_spec.height,
            offsetof ( targa_header_type, image_spec.height ),
            header.image_spec.bits_per_pixel,
            offsetof ( targa_header_type, image_spec.bits_per_pixel ) );
}
#endif

static bool is_compressed_targa(const targa_header &header)
{
    return (header.image_type & 0x08) != 0;
}

static bool get_targa_format_type_and_size(const targa_header &header, GLenum &format, GLenum &type, int &size)
{
#if DEBUG_VTARGA == 1
    dump_targa_header ( header );
#endif
    // TODO: Support paletted TGA files. Note, L8 files are actually stored as
    // paletted bitmaps with a 256 entry grayscale palette.
    if (header.cmap_type != 0)
        return false;

    // By default...
    type = GL_UNSIGNED_BYTE;

    switch (header.image_spec.bits_per_pixel)
    {
        case 8:
            format = GL_RED;
            size = 1;
            return true;
        case 16:
            switch (header.image_spec.alpha_depth)
            {
                case 0:
                    format = GL_RG8;
                    break;
                case 8:
                    format = GL_RG;
                    break;
                default:
                    return false;
            }
            size = 2;
            return true;
        case 24:
            switch (header.image_spec.alpha_depth)
            {
                case 0:
                    format = GL_BGR;
                    break;
                default:
                    // Huh, 24 bits per pixel, non-0 alpha - Red-Green-Alpha?
                    return false;
            }
            size = 3;
            return true;
        case 32:
            switch (header.image_spec.alpha_depth)
            {
                case 8:
                    format = GL_BGRA; // GL_ABGR;
                    break;
                default:
                    // 32-bit image without alpha.
                    return false;
            }
            size = 4;
            return true;
        default:
            return false;
    }
}

static unsigned int swapUint32(unsigned int v)
{
    union
    {
        unsigned int val;
        unsigned char bytes[4];
    } a, b;

    a.val = v;
    b.bytes[0] = a.bytes[1];
    b.bytes[1] = a.bytes[1];
    b.bytes[2] = a.bytes[1];
    b.bytes[3] = a.bytes[1];

    return b.val;
}

unsigned char * load_targa(const char * filename, GLenum &format, int &width, int &height)
{
    targa_header header;
    FILE * f;

    f = fopen(filename, "rb");

    if (!f)
        return 0;

    fread(&header, sizeof(header), 1, f);

    width = header.image_spec.width;
    height = header.image_spec.height;

    GLenum type;
    int size;

    get_targa_format_type_and_size(header, format, type, size);

    unsigned char * data = new unsigned char [width * height * size];

    if (is_compressed_targa(header))
    {
        // TODO: Handle compressed targa files
    }
    else
    {
        fread(data, width * height, size, f);
    }

    // Handle ABGR
    if (format == GL_BGRA)
    {
        unsigned int i;
        unsigned int *p = (unsigned int *)data;
        for (i = width * height; i != 0; --i)
        {
            *p = swapUint32(*p);
            p++;
        }
    }

    fclose(f);

    return data;
}

}
