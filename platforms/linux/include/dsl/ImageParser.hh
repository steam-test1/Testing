//
// Created by znix on 25/01/2021.
//

#pragma once

namespace dsl
{
	class ImageParser
	{
	  public:
		// Doesn't exist in the basegame, but is convenient for us
		enum FormatType : int
		{
			FORMAT_BGRA = 11,
		};

		struct Format
		{
			FormatType prob_pix_type;
			int width;
			int height;
			int padding[5];
		};
		static_assert(sizeof(Format) == 32);

		virtual void dtor_thing_a() = 0;
		virtual void dtor_thing_b() = 0;
		virtual bool is_type(unsigned int a) const
		{
			using FuncType = bool (*)(const ImageParser*, unsigned int);
			static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("_ZNK3dsl11ImageParser7is_typeEj"));

			return realCall(this, a);
		}

		virtual unsigned int type_id() const
		{
			using FuncType = unsigned int (*)(const ImageParser*);
			static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("_ZNK3dsl11ImageParser7type_idEv"));

			return realCall(this);
		}

		virtual void open() = 0;
		virtual void close() = 0;
		virtual bool dynamic() const = 0;
		virtual void format(Format* out_format) const = 0;
		virtual int mip_levels() const = 0;
		virtual int slices() const = 0;
		virtual void set_slice(int) = 0;
		virtual bool srgb() const = 0;
		virtual void produce(unsigned char*, unsigned char*, const Format* format) const = 0;

		const void* image_producer_vtbl = nullptr;
		dsl::Archive archive;
		char padding[16];
	};
	static_assert(sizeof(ImageParser) == 152);
} // namespace dsl
