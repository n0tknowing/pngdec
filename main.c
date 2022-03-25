/*
 * chunkinfo - show information of PNG chunks
 *
 * chunkinfo has been placed in the public domain.
 * for details, see https://github.com/n0tknowing/chunkinfo
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static uint8_t plt[256][3];
static uint8_t bit_depth, color_type;
static uint32_t plt_entry;

enum COLOR_TYPE {
	GRAY = 0,
	RGB = 2,
	INDEXED = 3,
	GRAY_ALPHA = 4,
	RGB_ALPHA = 6
};

// crc from https://github.com/skeeto/scratch/blob/master/pngattach/pngattach.c
static uint32_t pd_crc32(uint32_t crc, const void *buf, size_t len)
{
	static const uint32_t crc32_table[] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
		0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
		0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
		0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
		0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
		0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
		0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
		0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
		0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
		0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
		0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
		0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
		0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
		0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
		0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
		0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
		0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
		0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
		0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
		0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
		0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
		0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
		0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
		0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
		0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
		0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
		0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
		0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
		0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
		0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
		0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
		0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
		0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
		0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
		0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
		0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
		0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
		0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
		0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
		0x2d02ef8d
	};

	const unsigned char *p = buf;
	crc ^= 0xffffffff;
	for (size_t n = 0; n < len; n++)
		crc = crc32_table[(crc ^ p[n]) & 0xff] ^ (crc >> 8);

	return crc ^ 0xffffffff;
}

static uint32_t fread_u32(FILE *f)
{
	uint32_t ret = 0;
	if (fread(&ret, sizeof(uint32_t), 1, f) != 1)
		errno = ferror(f) ? EIO : EINVAL;

	return __builtin_bswap32(ret);
}

static char *get_name_or_keyword(const uint8_t *data, uint32_t *len)
{
	size_t i = 0;
	char *ret = calloc(80, 1);
	if (!ret)
		return NULL;

	while (*data) {
		if (isprint(*data))
			ret[i] = *data;
		data++; i++;
	}
	*len = i;

	return ret;
}

static int png_ok(FILE *f)
{
	const uint8_t sign[8] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
	uint8_t buf[8];
	if (fread(buf, 1, 8, f) != 8)
		return 0;

	if (!memcmp(sign, buf, 8))
		return 1;

	return 0;
}

static void check_bdepth_coltype(void)
{
	switch (color_type) {
	case GRAY:
		if (bit_depth != 1 && bit_depth != 2 && bit_depth != 4 &&
		    bit_depth != 8 && bit_depth != 16) {
			fprintf(stderr, "IHDR: invalid bit depth for grayscale\n");
			fprintf(stderr, "expected 1, 2, 4, 8, 16, found %u\n", bit_depth);
			exit(1);
		}
		break;
	case RGB:
		if (bit_depth != 8 && bit_depth != 16) {
			fprintf(stderr, "IHDR: invalid bit depth for RGB\n");
			fprintf(stderr, "expected 8, 16, found %u\n", bit_depth);
			exit(1);
		}
		break;
	case INDEXED:
		if (bit_depth != 1 && bit_depth != 2 && bit_depth != 4 && bit_depth != 8) {
			fprintf(stderr, "IHDR: invalid bit depth for indexed-color\n");
			fprintf(stderr, "expected 1, 2, 4, 8, found %u\n", bit_depth);
			exit(1);
		}
		break;
	case GRAY_ALPHA:
		if (bit_depth != 8 && bit_depth != 16) {
			fprintf(stderr, "IHDR: invalid bit depth for grayscale+alpha\n");
			fprintf(stderr, "expected 8, 16, found %u\n", bit_depth);
			exit(1);
		}
		break;
	case RGB_ALPHA:
		if (bit_depth != 8 && bit_depth != 16) {
			fprintf(stderr, "IHDR: invalid bit depth for RGB+alpha\n");
			fprintf(stderr, "expected 8, 16, found %u\n", bit_depth);
			exit(1);
		}
		break;
	default:
		fprintf(stderr, "IHDR: invalid color type\n");
		exit(1);
		break;
	}
}

/*
 * IHDR
 *
 * offset   type    length   value
 * -------------------------------
 * 0 - 3    uint32    4      width (min=1,max=2^31)
 * 4 - 7    uint32    4      height (min=1,max=2^31)
 *   8      uint8     1      bit depth (1,2,4,8,16)
 *   9      uint8     1      color type (0,2,3,4,6)
 *  10      uint8     1      compression method (0)
 *  11      uint8     1      filter method (0)
 *  12      uint8     1      interlace method (0,1)
 */
static void decode_ihdr(const uint8_t *data, const uint32_t len)
{
	if (len != 13) {
		fprintf(stderr, "IHDR: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected 13, but found %u\n", len);
		exit(1);
	}

	bit_depth = data[8], color_type = data[9];
	check_bdepth_coltype();

	uint32_t w = 0, h = 0;
	memcpy(&w, data, sizeof(uint32_t));
	memcpy(&h, data + sizeof(uint32_t), sizeof(uint32_t));
	w = __builtin_bswap32(w);
	h = __builtin_bswap32(h);

	if ((w > INT_MAX - 1) || (h > INT_MAX - 1)) {
		fprintf(stderr, "IHDR: width/height is too large %u/%u\n", w, h);
		fprintf(stderr, "maximum value is %d\n", INT_MAX - 1);
		exit(1);
	}

	printf("\tWidth = %u\n", w);
	printf("\tHeight = %u\n", h);
	printf("\tBit depth = %u per channel\n", bit_depth);
	printf("\tColor type = ");
	switch (color_type) {
	case GRAY:
		printf("Grayscale\n");
		printf("\tChannels = 1 per pixel (%u bytes)\n", bit_depth * 1);
		break;
	case RGB:
		printf("RGB\n");
		printf("\tChannels = 3 per pixel (%u bytes)\n", bit_depth * 3);
		break;
	case INDEXED:
		printf("Indexed color\n");
		printf("\tChannels = 1 per pixel (%u bytes)\n", bit_depth * 1);
		break;
	case GRAY_ALPHA:
		printf("Grayscale with Alpha channel\n");
		printf("\tChannels = 2 per pixel (%u bytes)\n", bit_depth * 2);
		break;
	case RGB_ALPHA:
		printf("RGB with Alpha channel\n");
		printf("\tChannels = 4 per pixel (%u bytes)\n", bit_depth * 4);
		break;
	}

	const char *compression = data[10] == 0 ? "zlib deflate/inflate" : "unknown";
	const char *filter = data[11] == 0 ? "adaptive filtering" : "unknown";
	const char *interlace = data[12] == 0 ? "no" : "Adam7";
	printf("\tCompression = %u (%s)\n", data[10], compression);
	printf("\tFilter = %u (%s)\n", data[11], filter);
	printf("\tInterlace = %u (%s interlace)", data[12], interlace);
}

static void decode_plte(const uint8_t *data, const uint32_t len)
{
	if (len % 3 != 0) {
		fprintf(stderr, "PLTE: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected divisible by 3, but found %u\n",
				len);
		exit(1);
	}

	plt_entry = len / 3;
	printf("\tEntries = %u\n", plt_entry);

	// fill color
	for (uint32_t i = 0, col = 0; i < plt_entry; i++, col += 3) {
		plt[i][0] = data[col];
		plt[i][1] = data[col+1];
		plt[i][2] = data[col+2];
	}

	// print color
	for (uint32_t i = 0, nl = 1; i < plt_entry; i++, nl++) {
		printf("\t[%03u]", i);
		printf(" #%02x%02x%02x ", plt[i][0], plt[i][1], plt[i][2]);
		if ((nl % 6) == 0)
			printf("\n");
	}
}

static void decode_idat(const uint8_t *data, const uint32_t len)
{
	printf("\t");

#if _DECODE_IDAT
	FILE *f = fopen("idat.z", "ab");
	if (!f) {
		perror("IDAT: failed to open idat.z");
		exit(1);
	}
	if (fwrite(data, sizeof(uint8_t), len, f) != len) {
		fclose(f);
		exit(1);
	}

	fclose(f);
	printf("See idat.z");
#else
	(void)data; (void)len;
	printf("Image data");
#endif
}

static void decode_time(const uint8_t *data, const uint32_t len)
{
	if (len != 7) {
		fprintf(stderr, "tIME: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected 7, but found %u\n", len);
		exit(1);
	}

	uint16_t year = 0;
	memcpy(&year, data, sizeof(uint16_t));

	struct tm t = {
		.tm_sec = data[6],
		.tm_min = data[5],
		.tm_hour = data[4],
		.tm_mday = data[3],
		.tm_mon = data[2],
		.tm_year = __builtin_bswap16(year) - 1900
	};

	char buf[100] = {0};
	if (strftime(buf, 99, "%A, %d %b %Y - %I:%M %p", &t))
		printf("\tLast modification = %s", buf);
}

static void decode_phys(const uint8_t *data, const uint32_t len)
{
	if (len != 9) {
		fprintf(stderr, "pHYs: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected 9, but found %u\n", len);
		exit(1);
	}

	uint32_t x = 0, y = 0;

	memcpy(&x, data, sizeof(uint32_t));
	memcpy(&y, data + sizeof(uint32_t), sizeof(uint32_t));
	x = __builtin_bswap32(x);
	y = __builtin_bswap32(y);

	uint32_t dpi = x * 0.0254;
	const char *unit = !!data[8] ? " per meter" : "";

	printf("\t%u x %u pixels%s (approx. %u DPI)", x, y, unit, dpi);
}

static void decode_srgb(const uint8_t *data, const uint32_t len)
{
	if (len != 1) {
		fprintf(stderr, "sRGB: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected 1, but found %u\n", len);
		exit(1);
	}

	const char *srgb_data[5] = {
		"Perceptual",
		"Relative colorimetric",
		"Saturation",
		"Absolute colorimetric",
		NULL
	};

	printf("\t");

	int intent = data[0];
	if (intent >= 0 && intent <= 3)
		printf("%d intent (%s)", intent, srgb_data[intent]);
	else
		printf("%d (Unknown intent)", intent);
}

static void decode_gama(const uint8_t *data, const uint32_t len)
{
	if (len != 4) {
		fprintf(stderr, "gAMA: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected 4, but found %u\n", len);
		exit(1);
	}

	uint32_t gama = 0;
	memcpy(&gama, data, sizeof(uint32_t));

	printf("\tGamma = %01.05f", (float)__builtin_bswap32(gama) / 100000);
}

static void decode_chrm(const uint8_t *data, const uint32_t len)
{
	if (len != 32) {
		fprintf(stderr, "cHRM: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected 32, but found %u\n", len);
		exit(1);
	}

	int offset = 0;
	uint32_t res[8] = {0};

	for (uint8_t i = 0; i < 8; i++) {
		memcpy(&res[i], data + offset, sizeof(uint32_t));
		res[i] = __builtin_bswap32(res[i]);
		offset += sizeof(uint32_t);
	}

	printf("\tWhite point x = %01.05f\n", (float)res[0] / 100000);
	printf("\tWhite point y = %01.05f\n", (float)res[1] / 100000);
	printf("\tRed x = %01.05f\n", (float)res[2] / 100000);
	printf("\tRed y = %01.05f\n", (float)res[3] / 100000);
	printf("\tBlue x = %01.05f\n", (float)res[4] / 100000);
	printf("\tBlue y = %01.05f\n", (float)res[5] / 100000);
	printf("\tGreen x = %01.05f\n", (float)res[6] / 100000);
	printf("\tGreen y = %01.05f", (float)res[7] / 100000);
}

static void decode_iccp(const uint8_t *data, const uint32_t len)
{
	uint32_t l = len, i = 0;
	char *profile_name = get_name_or_keyword(data, &i);
	if (!profile_name) {
		fprintf(stderr, "iCCP: cannot get profile name\n");
		exit(1);
	}

	printf("\tProfile name = %s\n", profile_name);
	free(profile_name);
	data += i; l -= i;

	data++; l--;
	printf("\tCompression method = %u (zlib deflate/inflate)\n", *data);

	data++; l--;
	printf("\tProfile (compressed) = ");
	if (l > 1)
		printf(".....");
}

static void decode_text(const uint8_t *data, const uint32_t len)
{

	uint32_t l = len, i = 0;
	char *keyword = get_name_or_keyword(data, &i);
	if (!keyword) {
		fprintf(stderr, "tEXt: cannot get keyword\n");
		exit(1);
	}

	printf("\tKeyword = %s\n", keyword);
	free(keyword);
	data += i; l -= i;

	data++; l--;
	printf("\tText = ");
	while (l > 0) {
		if (isprint(*data))
			putchar(*data);
		data++; l--;
	}
}

static void decode_itxt(const uint8_t *data, const uint32_t len)
{
	uint32_t l = len, i = 0;
	char *keyword = get_name_or_keyword(data, &i);
	if (!keyword) {
		fprintf(stderr, "iTXt: cannot get keyword\n");
		exit(1);
	}

	printf("\tKeyword = %s\n", keyword);
	free(keyword);
	data += i; l -= i;

	data++; l--;
	uint8_t comp_flag = *data;
	char *comp = !!comp_flag ? "compressed" : "uncompressed";
	printf("\tCompression flag = %u (%s)\n", comp_flag, comp);

	data++; l--;
	printf("\tCompression method = %u (zlib deflate/inflate)\n", *data);

	data++; l--;
	printf("\tLanguage tag = ");
	while (*data) {
		if (isprint(*data))
			putchar(*data);
		data++; l--;
	}
	putchar('\n');

	data++; l--;
	printf("\tTranslated keyword (UTF-8) = ...\n");
	printf("\tText (UTF-8) = ...");
}

static void decode_ztxt(const uint8_t *data, const uint32_t len)
{
	uint32_t l = len, i = 0;
	char *keyword = get_name_or_keyword(data, &i);
	if (!keyword) {
		fprintf(stderr, "zTXt: cannot get keyword\n");
		exit(1);
	}

	printf("\tKeyword = %s\n", keyword);
	free(keyword);
	data += i; l -= i;

	data++; l--;
	printf("\tCompression method = %u (zlib deflate/inflate)\n", *data);

	data++; l--;
	printf("\tText (compressed) = ");
	if (l > 1)
		printf(".....");
}

static void decode_bkgd(const uint8_t *data, const uint32_t len)
{
	printf("\t");

	switch (color_type) {
	case GRAY: case GRAY_ALPHA:
		if (len != 2) {
			fprintf(stderr, "bKGD: corrupted chunk length\n");
			fprintf(stderr, "expected 2 for color type gray and gray+alpha\n");
			exit(1);
		}

		if (bit_depth < 16) {
			printf("Gray level = %u", data[1]);
		} else {
			uint16_t val = 0;
			memcpy(&val, data, sizeof(uint16_t));
			printf("Gray level = %u", __builtin_bswap16(val));
		}
		break;
	case RGB: case RGB_ALPHA:
		if (len != 6) {
			fprintf(stderr, "bKGD: corrupted chunk length\n");
			fprintf(stderr, "expected 6 for color type rgb and rgba+alpha\n");
			exit(1);
		}

		printf("Color = ");
		if (bit_depth < 16) {
			printf("#%02x%02x%02x", data[1], data[3], data[5]);
		} else {
			printf("#%02x%02x", data[0], data[1]);
			printf("%02x%02x", data[2], data[3]);
			printf("%02x%02x", data[4], data[5]);
		}
		break;
	case INDEXED: {
		if (len != 1) {
			fprintf(stderr, "bKGD: corrupted chunk length\n");
			fprintf(stderr, "expected 1 for color type indexed-color\n");
			exit(1);
		}

		uint32_t i = data[0];
		printf("index %u, with color ", i);
		printf("#%02x%02x%02x", plt[i][0], plt[i][1], plt[i][2]);
		break;
	}
	default:
		fprintf(stderr, "bKGD: invalid color type");
		exit(1);
		break;
	}
}

static void decode_sbit(const uint8_t *data, const uint32_t len)
{
	printf("\tSignificant bits = ");

	switch (color_type) {
	case GRAY:
		if (len != 1) {
			fprintf(stderr, "sBIT: corrupted chunk length\n");
			fprintf(stderr, "expected 1 for color type gray\n");
			exit(1);
		}

		printf("gray(%u)", data[0]);
		break;
	case GRAY_ALPHA:
		if (len != 2) {
			fprintf(stderr, "sBIT: corrupted chunk length\n");
			fprintf(stderr, "expected 2 for color type gray+alpha\n");
			exit(1);
		}

		printf("gray(%u), alpha(%u)", data[0], data[1]);
		break;
	case INDEXED: case RGB:
		if (len != 3) {
			fprintf(stderr, "sBIT: corrupted chunk length\n");
			fprintf(stderr, "expected 3 for color type indexed-color"
					"and rgb\n");
			exit(1);
		}

		printf("red(%u), green(%u), blue(%u)", data[0], data[1], data[2]);
		break;
	case RGB_ALPHA:
		if (len != 4) {
			fprintf(stderr, "sBIT: corrupted chunk length\n");
			fprintf(stderr, "expected 4 for color type rgb+alpha\n");
			exit(1);
		}

		printf("red(%u) green(%u) blue(%u) alpha(%u)",
				data[0], data[1], data[2], data[3]);
		break;
	default:
		fprintf(stderr, "sBIT: invalid color type");
		exit(1);
		break;
	}
}

static void decode_trns(const uint8_t *data, const uint32_t len)
{
	printf("\t");

	switch (color_type) {
	case GRAY: {
		if (len != 2) {
			fprintf(stderr, "tRNS: corrupted chunk length\n");
			fprintf(stderr, "expected 2 for color type gray\n");
			exit(1);
		}

		uint16_t val = 0;
		memcpy(&val, data, sizeof(uint16_t));
		val = __builtin_bswap16(val);
		printf("gray(%u)", val);
		break;
	}
	case RGB:
		if (len != 6) {
			fprintf(stderr, "tRNS: corrupted chunk length\n");
			fprintf(stderr, "expected 6 for color type rgb\n");
			exit(1);
		}

		if (bit_depth < 16) {
			printf("red(%02x), green(%02x), blue(%02x)",
					data[1], data[3], data[5]);
		} else {
			printf("red(%02x%02x), ", data[0], data[1]);
			printf("green(%02x%02x), ", data[2], data[3]);
			printf("blue(%02x%02x)", data[4], data[5]);
		}
		break;
	case INDEXED:
		for (uint32_t i = 0, nl = 1; i < len; i++, nl++) {
			printf("[%03u] %02x    ", i, data[i]);
			if ((nl % 6) == 0)
				printf("\n");
		}
		break;
	default:
		fprintf(stderr, "tRNS: only grayscale, rgb, and indexed-color are allowed");
		exit(1);
		break;
	}
}

static void decode_splt(const uint8_t *data, const uint32_t len)
{
	uint32_t l = len, i = 0;
	char *palette_name = get_name_or_keyword(data, &i);
	if (!palette_name) {
		fprintf(stderr, "sPLT: cannot get palette name\n");
		exit(1);
	}

	printf("\tPalette name = %s\n", palette_name);
	free(palette_name);
	data += i; l -= i;

	data++; l--;
	uint8_t sample_depth = *data;
	printf("\tSample depth = %u\n", sample_depth);

	data++; l--;
	printf("\tEntry = ");
	if (sample_depth == 8 && (l % 6) == 0) {
		uint32_t entry = l / 6;
		printf("%u\n", entry);
		for (uint32_t i = 0, col = 0, nl = 1; i < entry; i++, col += 3, nl++) {
			printf("\t[%03u]", i);
			printf(" #%02x%02x%02x ", data[col], data[col+1], data[col+2]);
			if ((nl % 6) == 0)
				printf("\n");
		}
	} else if (sample_depth == 16 && (l % 10) == 0) {
		uint32_t entry = l / 10;
		printf("%u\n", entry);
		for (uint32_t i = 0, col = 0, nl = 1; i < entry; i++, col += 10, nl++) {
			printf("\t[%03u]", i);
			printf(" #%02x%02x", data[col], data[col+1]); // red
			printf("%02x%02x", data[col+2], data[col+3]); // green
			printf("%02x%02x", data[col+4], data[col+5]); // blue
			printf("%02x%02x", data[col+6], data[col+7]); // alpha
			printf(" (%02x%02x)", data[col+8], data[col+9]); // frequency
			if ((nl % 5) == 0)
				printf("\n");
		}
	} else {
		fprintf(stderr, "sPLT: corrupted data");
		exit(1);
	}
}

static void decode_hist(const uint8_t *data, const uint32_t len)
{
	if (len % 2 != 0) {
		fprintf(stderr, "hIST: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected divisible by 2, but found %u\n",
				len);
		exit(1);
	}

	if (plt_entry == 0) {
		fprintf(stderr, "hIST: cannot find PLTE chunk\n");
		exit(1);
	}

	const uint32_t entry = len / 2;
	printf("\tEntries = %u\n", entry);

	for (uint32_t i = 0, nl = 1; i < entry; i++, nl++) {
		printf("\t[%03u] %u  ", i, data[i]);
		if ((nl % 6) == 0)
			printf("\n");
	}
}

static void decode_ext_offs(const uint8_t *data, const uint32_t len)
{
	if (len != 9) {
		fprintf(stderr, "oFFs: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected 9, but found %u\n", len);
		exit(1);
	}

	uint32_t x = 0, y = 0;
	memcpy(&x, data, sizeof(uint32_t));
	memcpy(&y, data + sizeof(uint32_t), sizeof(uint32_t));

	x = __builtin_bswap32(x);
	y = __builtin_bswap32(y);
	char *unit = !!data[8] ? "micrometres" : "pixels";

	printf("\tImage position = %d x %d %s", (int32_t)x, (int32_t)y, unit);
}

static void decode_ext_scal(const uint8_t *data, const uint32_t len)
{
	uint32_t l = len;

	char *unit = data[0] == 1 ? "meters" : data[0] == 2 ? "radians" : "invalid unit";
	data++; l--;

	printf("\tPhysical scale = ");

	while (*data) {
		if (isprint(*data))
			putchar(*data);
		data++; l--;
	}
	data++; l--;
	printf(" x ");

	while (l > 0) {
		if (isprint(*data))
			putchar(*data);
		data++; l--;
	}
	printf(" (%s)", unit);
}

static void decode_ext_pcal(const uint8_t *data, const uint32_t len)
{
	uint32_t l = len, i = 0;
	char *name = get_name_or_keyword(data, &i);
	if (!name) {
		fprintf(stderr, "pCAL: cannot get calibration name\n");
		exit(1);
	}

	printf("\tCalibration name = %s\n", name);
	free(name); name = NULL;
	data += i; l -= i; i = 0;

	data++; l--;
	uint32_t x0 = 0, x1 = 0;
	memcpy(&x0, data, sizeof(uint32_t));
	x0 = __builtin_bswap32(x0);
	data += 4; l-= 4;
	memcpy(&x1, data, sizeof(uint32_t));
	x1 = __builtin_bswap32(x1);
	data += 4; l-= 4;

	printf("\tLinear conversion = %d x %d\n", (int32_t)x0, (int32_t)x1);

	uint8_t eq_type = *data;
	char *eqs[] = {
		"linear", "exponential",
		"exponential arbitrary base", "hyperbolic sinusoidal",
		NULL
	};
	char *eq_s = eq_type <= 3 ? eqs[eq_type] : "invalid";
	data++; l--;
	printf("\tEquation type = %u (%s)\n", eq_type, eq_s);

	uint8_t params = *data;
	data++; l--;
	printf("\tParameters = %u\n", params);

	name = get_name_or_keyword(data, &i);
	if (!name) {
		fprintf(stderr, "pCAL: cannot get unit name\n");
		exit(1);
	}

	printf("\tUnit name = %s\n", name);
	free(name);
	data += i; l -= i;

	data++; l--;
	printf("\tValues = ");
	for (uint8_t i = 0; i < params; i++) {
		while (l > 0) {
			if (isprint(*data))
				putchar(*data);
			else
				printf(", ");
			data++; l--;
		}
	}
}

static void decode_apng_actl(const uint8_t *data, const uint32_t len)
{
	if (len != 8) {
		fprintf(stderr, "acTL: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected 8, but found %u\n", len);
		exit(1);
	}

	uint32_t nframes = 0, nplays = 0;
	memcpy(&nframes, data, sizeof(uint32_t));
	memcpy(&nplays, data + sizeof(uint32_t), sizeof(uint32_t));

	nframes = __builtin_bswap32(nframes);
	nplays = __builtin_bswap32(nplays);

	printf("\tNumber of frames = %u\n", nframes);
	printf("\tNumber of plays = %u %s", nplays, nplays == 0 ? "(infinite)" : "");
}

static void decode_apng_fctl(const uint8_t *data, const uint32_t len)
{
	if (len != 26) {
		fprintf(stderr, "fcTL: corrupted chunk length\n");
		fprintf(stderr, "chunk length is expected 26, but found %u\n", len);
		exit(1);
	}

	uint32_t buf1[4] = {0}; // width, height, x_offset, y_offset;
	uint16_t buf2[2] = {0}; // delay_nums, delay_den;
	uint8_t buf3[2] = {0}; // dispose_op, blend_op;
	char *dstr[4] = {
		"None", "Background", "Previous",
		NULL
	};

	int offset = 4; // skip sequence_number for now
	for (uint8_t i = 0; i < 4; i++) {
		memcpy(&buf1[i], data + offset, sizeof(uint32_t));
		buf1[i] = __builtin_bswap32(buf1[i]);
		offset += sizeof(uint32_t);
	}

	for (uint8_t i = 0; i < 2; i++) {
		memcpy(&buf2[i], data + offset, sizeof(uint16_t));
		buf2[i] = __builtin_bswap16(buf2[i]);
		offset += sizeof(uint16_t);
	}

	buf3[0] = data[offset];
	buf3[1] = data[offset + 1];
	char *dispose = (buf3[0] <= 3) ? dstr[buf3[0]] : "Invalid";
	char *blend = buf3[1] == 0 ? "Source" : buf3[1] == 1 ? "Over" : "Invalid";

	printf("\tWidth = %u\n", buf1[0]);
	printf("\tHeight = %u\n", buf1[1]);
	printf("\tX offset = %u\n", buf1[2]);
	printf("\tY offset = %u\n", buf1[3]);
	printf("\tDelays = %u (denominator %u)\n", buf2[0], buf2[1]);
	printf("\tDisposal = %u (%s)\n", buf3[0], dispose);
	printf("\tBlend = %u (%s)", buf3[1], blend);
}

static void decode_ext_gifx(const uint8_t *data, const uint32_t len)
{
	if (len < 11) {
		fprintf(stderr, "gIFx: invalid chunk length\n");
		fprintf(stderr, "chunk length is expected at least 11, but found %u\n",
				len);
		exit(1);
	}

	printf("\tApplication ID = %.*s\n", 8, (char *)data);
	printf("\tAuthentication code = %02x%02x%02x\n", data[8], data[9], data[10]);
	printf("\tApplication data = .....");
}

static void decode_ext_gifg(const uint8_t *data, const uint32_t len)
{
	if (len != 4) {
		fprintf(stderr, "gIFg: invalid chunk length\n");
		fprintf(stderr, "chunk length is expected 4, but found %u\n", len);
		exit(1);
	}

	uint16_t delay_time = 0;
	memcpy(&delay_time, data + 2, sizeof(uint16_t));
	delay_time = __builtin_bswap16(delay_time);

	printf("\tDisposal method = %u\n", data[0]);
	printf("\tUser input = %u\n", data[1]);
	printf("\tDelay time = %lf seconds", (double).01 * delay_time);
}

static void decode_ext_ster(const uint8_t *data, const uint32_t len)
{
	if (len != 1) {
		fprintf(stderr, "sTER: invalid chunk length\n");
		fprintf(stderr, "chunk length is expected 1, but found %u\n", len);
		exit(1);
	}

	char *layout = !!data[0] ? "Diverging-fuse layout" : "Cross-fuse layout";
	printf("\tLayout = %u (%s)", !!data[0], layout);
}

#define decode_if(what, decode_func) \
	do {\
		if (!strcmp(type, what)) { \
			decode_func(data, len); \
			return; \
		} \
	} while (0)

static void decode_chunk_data(const uint8_t *data, const char *type, const uint32_t len)
{
	/* Critical chunks */
	decode_if("IHDR", decode_ihdr);
	decode_if("PLTE", decode_plte);
	decode_if("IDAT", decode_idat);

	/* ancillary chunks */
	decode_if("tIME", decode_time);
	decode_if("pHYs", decode_phys);
	decode_if("sRGB", decode_srgb);
	decode_if("gAMA", decode_gama);
	decode_if("cHRM", decode_chrm);
	decode_if("iCCP", decode_iccp);
	decode_if("tEXt", decode_text);
	decode_if("iTXt", decode_itxt);
	decode_if("zTXt", decode_ztxt);
	decode_if("bKGD", decode_bkgd);
	decode_if("sBIT", decode_sbit);
	decode_if("tRNS", decode_trns);
	decode_if("sPLT", decode_splt);
	decode_if("hIST", decode_hist);

	/* official PNG extension chunks */
	decode_if("oFFs", decode_ext_offs);
	decode_if("sCAL", decode_ext_scal);
	decode_if("pCAL", decode_ext_pcal);
	decode_if("gIFx", decode_ext_gifx);
	decode_if("gIFg", decode_ext_gifg);
	decode_if("sTER", decode_ext_ster);

	/* APNG */
	decode_if("acTL", decode_apng_actl);
	decode_if("fcTL", decode_apng_fctl);

	/* no decoder yet... */
	printf("\t.....");
}

static void read_chunk(FILE *f)
{
	int not_iend = 1, i = 1;
	while (not_iend) {
		errno = 0;
		// read chunk size
		uint32_t size = fread_u32(f);
		if (errno) {
			perror("failed to read chunk length");
			exit(1);
		}

		// get current chunk offset
		long offset = ftell(f);
		if (offset < 0) {
			perror("failed to get chunk offset");
			exit(1);
		}

		// read chunk type
		char type[5];
		if (fread(type, sizeof(char), 4, f) != 4) {
			fprintf(stderr, "failed to read chunk type\n");
			exit(1);
		}
		type[4] = 0;

		if (i == 1 && strcmp(type, "IHDR")) {
			fprintf(stderr, "PNG image must be started with IHDR chunk\n");
			fprintf(stderr, "but found %s instead\n", type);
			exit(1);
		}

		if (!strcmp(type, "IEND"))
			not_iend = 0;

		if (size > INT_MAX - 1) {
			fprintf(stderr, "%s: chunk length is too large: %u\n",
					type, size);
			fprintf(stderr, "maximum length is %u\n", INT_MAX - 1);
			exit(1);
		}

		uint32_t check = pd_crc32(0u, type, 4); // type

		// read chunk data
		uint8_t *data = NULL;
		if (size > 0) {
			data = malloc(size);
			if (!data) {
				fprintf(stderr, "%s: ", type);
				perror("failed to read chunk data");
				exit(1);
			}
			if (fread(data, sizeof(uint8_t), size, f) != size) {
				fprintf(stderr, "%s: ", type);
				perror("failed to read chunk data");
				free(data);
				exit(1);
			}
			check = pd_crc32(check, data, size);  // data
		}

		// read chunk crc
		uint32_t chunk_crc = fread_u32(f);
		if (errno) {
			fprintf(stderr, "%s: ", type);
			perror("failed to read chunk crc");
			exit(1);
		}
		if (chunk_crc == check) {
			printf("   [%s] length %u at offset 0x%08lx (%04x)\n",
					type, size, offset, chunk_crc);
			if (data) {
				decode_chunk_data(data, type, size);
				free(data);
			} else {
				printf("\tNo data");
			};
			putchar('\n');
		} else {
			fprintf(stderr, "%s: corrupted CRC\n", type);
			fprintf(stderr, "expected %x, got %x\n", check, chunk_crc);
			if (data)
				free(data);
			exit(1);
		}

		i++;
		printf("\n");
	}

	printf("All OK.\n");
	printf("Found %d chunks from file ", i);
}

static int run(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s file.png\n", argv[0]);
		return 1;
	}

	errno = 0;
	FILE *f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "cannot open %s: %s\n", argv[1], strerror(errno));
		return !!errno;
	}

	if (png_ok(f)) {
		read_chunk(f);
		printf("%s\n", argv[1]);
	} else {
		errno = EINVAL;
		fprintf(stderr, "%s is not a valid PNG file\n", argv[1]);
	}

	fclose(f);
	return !!errno;
}

int main(int argc, char **argv)
{
	return run(argc, argv);
}
