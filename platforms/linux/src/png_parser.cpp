//
// Created by znix on 25/01/2021.
//

#include "blt/png_parser.hh"

#include "blt/elf_utils.hh"
#include "blt/libcxxstring.hh"

#include "dsl/Archive.hh"
#include "dsl/ImageParser.hh"

#include <assert.h>
#include <dlfcn.h>
#include <string.h>

#include <png.h>

#include <fstream>
#include <util/util.h>
#include <vector>

typedef bool (*can_parse_t)(dsl::Archive*);
typedef dsl::ImageParser* (*create_t)();

typedef void (*archive_ctor_t)(dsl::Archive* this_, blt::libcxxstring const& name, void* datastore, size_t start_pos,
                               size_t size, bool probably_write_flag, void* ukn);
static archive_ctor_t archive_ctor;

class PNGParser final : public dsl::ImageParser
{
  public:
	static bool can_parse_png(dsl::Archive* archive)
	{
		if (archive->size_var <= 8)
			return false;

		size_t old_pos = archive->read_counter;
		uint8_t signature[8];
		archive->checked_read_raw(signature, sizeof(signature));
		archive->read_counter = old_pos;

		return !png_sig_cmp(signature, 0, sizeof(signature));
	}

	static dsl::ImageParser* create_parser()
	{
		return new PNGParser();
	}

	PNGParser()
	{
		archive_ctor(&archive, blt::libcxxstring(""), nullptr, 0, 0, false, nullptr);

		fake_cleanup_vtbl.resize(32);
		image_producer_vtbl = fake_cleanup_vtbl.data();
		memset(fake_cleanup_vtbl.data(), 0, fake_cleanup_vtbl.size());
		*(const void**)(fake_cleanup_vtbl.data() + 16) = (const void*)&cleanup;
	}

	void dtor_thing_a() override
	{
		abort();
	}

	void dtor_thing_b() override
	{
		abort();
	}

	void open() override
	{
		// Read the PNG into memory
		std::vector<uint8_t> data(archive.size_var);
		archive.read_counter = 0;
		archive.checked_read_raw(data.data(), data.size());

		png_image img;
		memset(&img, 0, sizeof(img));
		img.version = PNG_IMAGE_VERSION;
		int res = png_image_begin_read_from_memory(&img, data.data(), data.size());
		if (!res)
		{
			printf("[PNGParser] png_image_begin_read failed: %d %s\n", res, img.message);
			abort();
		}

		// Now we know the image size, allocate the buffer for it and read into it
		img.format = PNG_FORMAT_BGRA;
		image_buffer.resize(4 * img.width * img.height);
		res = png_image_finish_read(&img, nullptr, image_buffer.data(), 4 * img.width, nullptr);
		if (!res)
		{
			printf("[PNGParser] png_image_finish_read failed: %d %s\n", res, img.message);
			abort();
		}

		// Fill out the format data
		memset(&image_fmt, 0, sizeof(image_fmt));
		image_fmt.prob_pix_type = FORMAT_BGRA; // 32bpp BGRA
		image_fmt.width = img.width;
		image_fmt.height = img.height;
		// TODO find out what the other fields do, if they're important
	}

	void close() override
	{
		image_buffer.clear();
		image_buffer.shrink_to_fit();
	}

	void format(Format* out_format) const override
	{
		*out_format = image_fmt;
	}

	void produce(unsigned char* pixel_data, unsigned char* string1, const Format* format) const override
	{
		int bytes_per_pix;
		switch (format->prob_pix_type)
		{
		case 11:
			bytes_per_pix = 4;
			break;
		default:
			char buff[128];
			snprintf(buff, sizeof(buff) - 1, "[PNGLoader] Invalid pixel format %d\n", format->prob_pix_type);
			PD2HOOK_LOG_ERROR(buff);
			abort();
		}

		if (format->prob_pix_type != image_fmt.prob_pix_type || format->width != image_fmt.width ||
		    format->height != image_fmt.height)
		{
			printf("[PNGLoader] Image format mismatch\n");
			abort();
		}

		[[maybe_unused]] int data_size = bytes_per_pix * format->width * format->height;
		assert((int)image_buffer.size() == data_size);
		memcpy(pixel_data, image_buffer.data(), image_buffer.size());
	}

	static void cleanup(void* vtbl_addr)
	{
		// We're supplied the pointer to the image_producer_vtbl pointer - that's because there's an object
		// nested inside this one (due to multiple inheritance) in the base game's definitions, and this
		// is the destructor on one of those. As a result we have to compute the offset, and then use that
		// to find the real PNGParser object.

		PNGParser* test_obj = nullptr;
		int offset = (int)(ptrdiff_t)(&test_obj->image_producer_vtbl);

		auto* obj = (PNGParser*)((char*)vtbl_addr - offset);
		delete obj;
	}

	// Stuff to basically just ignore

	[[nodiscard]] bool dynamic() const override
	{
		return false;
	}

	[[nodiscard]] int mip_levels() const override
	{
		return 1;
	}
	[[nodiscard]] bool srgb() const override
	{
		return true; // AFAIK pngs all use SRGB
	}

	[[nodiscard]] int slices() const override
	{
		return 1;
	}
	void set_slice(int i) override
	{
		// AFAIK noone should set this?
	}

  private:
	std::vector<uint8_t> fake_cleanup_vtbl;
	std::vector<uint8_t> image_buffer;
	Format image_fmt{};
};

typedef void (*register_parser_t)(const blt::libcxxstring& ext, can_parse_t can_parse, create_t create);
static register_parser_t register_parser;

void blt::install_png_parser()
{
	register_parser(libcxxstring("png"), PNGParser::can_parse_png, PNGParser::create_parser);
}

void blt::init_png_parser()
{
	const char* sig = "_ZN3dsl18GeneralImageParser15register_parserERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_"
					  "9allocatorIcEEEEPFbRNS_7ArchiveEEPFPNS_11ImageParserEvE";

	register_parser = (register_parser_t)blt::elf_utils::find_sym(sig);

	const char* archive_sig =
		"_ZN3dsl7ArchiveC1ERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPNS_9DataStoreExxbx";
	archive_ctor = (archive_ctor_t)blt::elf_utils::find_sym(archive_sig);
}
