#include "table_printer.hpp"

struct NameField : StringField {
	static constexpr std::string_view field_name = "Name";
};

struct DescField : StringField {
	static constexpr std::string_view field_name = "Descriptoin";
};

struct CountField : UsizeField {
	static constexpr std::string_view field_name = "Count";
};

struct Object {
	NameField name;
	DescField desc;
	CountField count;
};

int main() {
	std::vector v = {
		Object{ "Some object", "We know next to nothing about it", 20},
		Object{ "Some object 2", "Some object 2 idk what write else here", 30},
	};

	TablePrinter tp{ v, &Object::name, &Object::count, &Object::desc };

	tp.print_all();

	return 0;
}