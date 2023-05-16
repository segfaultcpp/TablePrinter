#pragma once
#include <array>
#include <compare>
#include <concepts>
#include <string_view>
#include <string>
#include <tuple>
#include <span>
#include <ranges>
#include <format>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <type_traits>

namespace tp {
		struct FieldTag {
		auto operator<=>(const FieldTag&) const noexcept = default;
	};

	template<typename T>
	concept FieldType = std::derived_from<T, FieldTag>;

	struct StringField : FieldTag {
		std::string value;

		StringField() noexcept = default;

		StringField(const char* str) noexcept
			: value{ str }
		{}

		StringField& operator=(const char* str) noexcept {
			value = str;
			return *this;
		}

		StringField(std::string str) noexcept
			: value{ std::move(str) }
		{}

		StringField& operator=(std::string str) noexcept {
			value = std::move(str);
			return *this;
		}

		StringField(const StringField&) noexcept = default;
		StringField& operator=(const StringField&) noexcept = default;

		StringField(StringField&&) noexcept = default;
		StringField& operator=(StringField&&) noexcept = default;

		std::size_t get_width() const noexcept {
			return value.size();
		}

		// it must be defined to satisfy constraints
		auto operator<=>(const StringField&) const noexcept = default;

		bool operator>(const StringField& rhs) const noexcept {
			return value.size() > rhs.value.size();
		}

		bool operator<(const StringField& rhs) const noexcept {
			return value.size() < rhs.value.size();
		}
	};

	template<typename T> requires std::integral<T> || std::floating_point<T>
	struct NumberField : FieldTag {
		T value;

		NumberField() noexcept = default;

		NumberField(T v) noexcept
			: value{ v }
		{}

		NumberField& operator=(T v) noexcept {
			value = v;
			return *this;
		}

		T get_width() const noexcept {
			return 10u;
		}

		auto operator<=>(const NumberField&) const noexcept = default;
	};

	using U8Field = NumberField<std::uint8_t>;
	using U16Field = NumberField<std::uint16_t>;
	using U32Field = NumberField<std::uint32_t>;
	using U64Field = NumberField<std::uint64_t>;

	using I8Field = NumberField<std::int8_t>;
	using I16Field = NumberField<std::int16_t>;
	using I32Field = NumberField<std::int32_t>;
	using I64Field = NumberField<std::int64_t>;

	using UsizeField = NumberField<std::size_t>;

	using F32Field = NumberField<float>;
	using F64Field = NumberField<double>;

	template<typename T>
	inline constexpr bool is_number_field = false;

	template<typename T>
	inline constexpr bool is_number_field<NumberField<T>> = true;

	template<typename T>
	concept IsNumberField = is_number_field<T>;

	namespace detail {
		template<typename T>
		struct GetProjType;

		template<typename Class, typename T>
		struct GetProjType<T Class::*> {
			using Type = T;
		};

		template<typename T>
		concept IsProj = requires {
			typename GetProjType<T>::Type;
		};
	}

	struct TablePrinterDesc {
		char horizontal_sep = '=';
		char vertical_sep = '|';
		bool paddings = true;

		constexpr TablePrinterDesc() noexcept = default;

		constexpr TablePrinterDesc(char hor, char vert, bool pad) noexcept
			: horizontal_sep{ hor }
			, vertical_sep{ vert }
			, paddings{ pad }
		{}

		constexpr TablePrinterDesc with_separators(char hor, char vert) const noexcept {
			return TablePrinterDesc{ hor, vert, paddings };
		}

		constexpr TablePrinterDesc use_paddings(bool use) const noexcept {
			return TablePrinterDesc{ horizontal_sep, vertical_sep, use };
		}
	};

	template<typename T, typename... FieldTypes>
		requires (detail::IsProj<FieldTypes> && ...)
	struct TablePrinter {
		static constexpr std::size_t field_count = sizeof...(FieldTypes);
		static constexpr std::array field_names = { detail::GetProjType<FieldTypes>::Type::field_name... };
	
	private:
		std::span<T> table_;
		std::tuple<FieldTypes...> fields_;
		TablePrinterDesc desc_;
	
	public:
		TablePrinter(std::span<T> v, FieldTypes... f) noexcept 
			: table_{ v }
			, fields_{ std::make_tuple(f...) }
			, desc_{}
		{}

		TablePrinter(TablePrinterDesc desc, std::span<T> v, FieldTypes... f) noexcept 
			: table_{ v }
			, fields_{ std::make_tuple(f...) }
			, desc_{ desc }
		{}

		void print_all() const noexcept {
			static auto field_widths = [this]<std::size_t... I>(std::index_sequence<I...>) {
				std::array<std::size_t, field_count> ret = {
					(std::invoke(std::get<I>(fields_), std::ranges::max(table_, {}, std::get<I>(fields_))).get_width() + 2)...
				};
				return ret;
			}(std::make_index_sequence<field_count>{});

			for (std::size_t i : std::views::iota(0u, field_count)) {
				if ((field_widths[i] - 2) < field_names[i].size()) {
					field_widths[i] = field_names[i].size() + 2;
				}
			}

			auto table_width = std::accumulate(field_widths.begin(), field_widths.end(), static_cast<std::size_t>(field_count + 1));

			print_separator_(table_width);
			
			for (std::size_t i : std::views::iota(0u, field_count)) {
				std::cout << std::format("{} {:^{}} ", desc_.vertical_sep, field_names[i], field_widths[i] - 2);
			}
			std::cout << desc_.vertical_sep << "\n";

			print_separator_(table_width);

			for (auto& el : table_) {
				print_padding_(field_widths);

				[this, &el] <std::size_t... I>(std::index_sequence<I...>) {
					((std::cout << std::format("{} {:^{}} ", desc_.vertical_sep, std::invoke(std::get<I>(fields_), el).value, field_widths[I] - 2)), ...);
				}(std::make_index_sequence<field_count>{});
				std::cout << desc_.vertical_sep << "\n";

				print_padding_(field_widths);

				print_separator_(table_width);
			}
		}

		void print_all(std::ofstream& fout) const noexcept {
			static constexpr std::string_view head = 
R"str(<?xml version="1.0"?>
<Workbook xmlns="urn:schemas-microsoft-com:office:spreadsheet"
xmlns:o="urn:schemas-microsoft-com:office:office"
xmlns:x="urn:schemas-microsoft-com:office:excel"
xmlns:ss="urn:schemas-microsoft-com:office:spreadsheet"
xmlns:html="http://www.w3.org/TR/REC-html40">
<Worksheet ss:Name="Sheet1">
)str";
			
			static constexpr std::string_view ending = 
R"str(</Table>
</Worksheet>
</Workbook>)str";

			std::string xml = head.data();
			xml += std::format("<Table ss:ExpandedColumnCount=\"{}\" ss:ExpandedRowCount=\"{}\" x:FullColumns=\"1\" x:FullRows=\"1\">\n", field_count, table_.size() + 1);
		
			xml += "<Row>\n";
			for (auto name : field_names) {
				xml += std::format("<Cell><Data ss:Type=\"String\">{}</Data></Cell>\n", name);
			}
			xml += "</Row>\n";

			auto accessor = [&xml](auto&& el) noexcept {
				using Type = std::remove_reference_t<decltype(el)>;
				static constexpr std::string_view type_name = [] {
					if constexpr (std::is_base_of_v<StringField, Type>) {
						return "String";
					}
					else if constexpr (IsNumberField<Type>) { // TODO:
						return "Number";
					}
					return "String";
				}();

				xml += std::format("<Cell><Data ss:Type=\"{}\">{}</Data></Cell>\n", type_name, el.value);
			};

			for (auto el : table_) {
				xml += "<Row>\n";
				[this, &el, &accessor] <std::size_t... I>(std::index_sequence<I...>) {
					(accessor(std::invoke(std::get<I>(fields_), el)), ...);
				}(std::make_index_sequence<field_count>{});
				xml += "</Row>\n";
			}

			xml += ending;
			fout << xml;
		}

	private:
		void print_padding_(std::span<std::size_t> field_widths) const noexcept {
			if (desc_.paddings) {
				for (std::size_t w : field_widths) {
					std::cout << desc_.vertical_sep;

					for (std::size_t i : std::views::iota(0u, w)) {
						std::cout << " ";
					}
				}
				std::cout << desc_.vertical_sep << "\n";
			}
		}

		void print_separator_(std::size_t table_width) const noexcept {
			for (std::size_t i : std::views::iota(0u, table_width)) {
				std::cout << desc_.horizontal_sep;
			}
			std::cout << "\n";
		}

	};

	template<template<typename, typename...> typename C, typename T, typename... FieldTypes>
	TablePrinter(C<T>, FieldTypes...) -> TablePrinter<T, FieldTypes...>;

	template<template<typename, typename...> typename C, typename T, typename... FieldTypes>
	TablePrinter(TablePrinterDesc, C<T>, FieldTypes...) -> TablePrinter<T, FieldTypes...>;
}

#define DECLARE_FIELD(Type, Name, Desc) \
	static_assert(::tp::FieldType<Type>); \
	static_assert(::std::constructible_from<::std::string_view, decltype(Desc)>); \
	struct Name##Field : Type { \
		static constexpr ::std::string_view field_name = Desc; \
	}; \
	Name##Field Name;