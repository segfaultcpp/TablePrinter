#pragma once
#include <array>
#include <vector>
#include <concepts>
#include <string_view>
#include <string>
#include <tuple>
#include <span>
#include <ranges>
#include <format>
#include <algorithm>
#include <numeric>

template<typename T>
struct GetProjType;

template<typename Class, typename T>
struct GetProjType<T Class::*> {
	using Type = T;
};

template<typename T, typename... FieldTypes>
	requires (requires { typename GetProjType<FieldTypes>::Type; } && ...)
struct TablePrinter {
	std::span<T> table;
	static constexpr std::size_t field_count = sizeof...(FieldTypes);
	static constexpr std::array field_names = { GetProjType<FieldTypes>::Type::field_name... };
	std::tuple<FieldTypes...> fields;

	TablePrinter(std::span<T> v, FieldTypes... f) noexcept 
		: table{ v }
		, fields{ std::make_tuple(f...) }
	{}

	void print_all() noexcept {
		auto field_widths = [this]<std::size_t... I>(std::index_sequence<I...>) {
			std::vector<std::size_t> ret = {
				(std::invoke(std::get<I>(this->fields), std::ranges::max(this->table, {}, std::get<I>(this->fields))).get_width() + 2)...
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
			std::cout << std::format("| {:^{}} ", field_names[i], field_widths[i] - 2);
		}
		std::cout << "|\n";

		print_separator_(table_width);

		for (auto& el : table) {
			print_padding_(field_widths);

			[this, &el, &field_widths] <std::size_t... I>(std::index_sequence<I...>) {
				((std::cout << std::format("| {:^{}} ", std::invoke(std::get<I>(this->fields), el).value, field_widths[I] - 2)), ...);
			}(std::make_index_sequence<field_count>{});
			std::cout << "|\n";

			print_padding_(field_widths);

			print_separator_(table_width);
		}
	}

private:
	void print_padding_(std::span<std::size_t> field_widths) noexcept {
		for (std::size_t w : field_widths) {
			std::cout << "|";

			for (std::size_t i : std::views::iota(0u, w)) {
				std::cout << " ";
			}
		}
		std::cout << "|\n";
	}

	void print_separator_(std::size_t table_width) noexcept {
		for (std::size_t i : std::views::iota(0u, table_width)) {
			std::cout << "=";
		}
		std::cout << "\n";
	}

};

template<typename T, typename... FieldTypes>
TablePrinter(std::vector<T>, FieldTypes...) -> TablePrinter<T, FieldTypes...>;

struct StringField {
	std::string value;

	StringField() = default;

	StringField(const char* str)
		: value{ str }
	{}

	StringField& operator=(const char* str) {
		value = str;
		return *this;
	}

	StringField(std::string str)
		: value{ std::move(str) }
	{}

	StringField& operator=(std::string str) {
		value = std::move(str);
		return *this;
	}

	StringField(const StringField&) = default;
	StringField& operator=(const StringField&) = default;

	StringField(StringField&&) = default;
	StringField& operator=(StringField&&) = default;

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

struct UsizeField {
	std::size_t value;

	UsizeField() = default;

	UsizeField(std::size_t v)
		: value{ v }
	{}

	UsizeField& operator=(std::size_t v) {
		value = v;
		return *this;
	}

	std::size_t get_width() const noexcept {
		return 10u;
	}

	auto operator<=>(const UsizeField&) const noexcept = default;
};