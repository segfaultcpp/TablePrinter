#include "table_printer.hpp"

#include <vector>
#include <fstream>

struct Gpu {
	DECLARE_FIELD(tp::StringField, Vendor, "Vendor");
	DECLARE_FIELD(tp::StringField, Model, "Model");
	DECLARE_FIELD(tp::UsizeField, VRamSize, "VRAM Size (GB)");
};

struct ExcelTest {
	DECLARE_FIELD(tp::StringField, StrValue, "String value");
	DECLARE_FIELD(tp::F64Field, DoubleValue, "Double value");
};

int main() {
	std::vector v1 = {
		Gpu{ "NVIDIA", "GTX 1660 TI", 6 },
		Gpu{ "NVIDIA", "RTX 2070", 8 },
		Gpu{ "AMD", "Radeon RX 580", 8 },
	};

	std::vector v2 = {
		ExcelTest{ "Foo", 12.459 },
		ExcelTest{ "Bar", 42.00001 },
	};

	{	
		tp::TablePrinter printer{ v1, &Gpu::Vendor, &Gpu::Model, &Gpu::VRamSize };
		printer.print_all();
	}
	std::cout << "\n";
	{
		static constexpr auto desc = tp::TablePrinterDesc{}
			.with_separators('+', '-')
			.use_paddings(false);
		tp::TablePrinter printer{ desc, v1, &Gpu::Vendor, &Gpu::Model, &Gpu::VRamSize };
		printer.print_all();
	}

	{
		std::ofstream fout("test.xls");
		tp::TablePrinter printer{ v2, &ExcelTest::StrValue, &ExcelTest::DoubleValue };
		printer.print_all(fout);
		fout.close();
	}
	return 0;
}