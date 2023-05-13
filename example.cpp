#include "table_printer.hpp"

#include <vector>

struct Gpu {
	DECLARE_FIELD(tp::StringField, Vendor, "Vendor");
	DECLARE_FIELD(tp::StringField, Model, "Model");
	DECLARE_FIELD(tp::UsizeField, VRamSize, "VRAM Size (GB)");
};

int main() {
	std::vector v = {
		Gpu{ "NVIDIA", "GTX 1660 TI", 6 },
		Gpu{ "NVIDIA", "RTX 2070", 8 },
		Gpu{ "AMD", "Radeon RX 580", 0 },
	};

	{	
		tp::TablePrinter printer{ v, &Gpu::Vendor, &Gpu::Model, &Gpu::VRamSize };
		printer.print_all();
	}
	std::cout << "\n";
	{
		static constexpr auto desc = tp::TablePrinterDesc{}
			.with_separators('+', '-')
			.use_paddings(false);
		tp::TablePrinter printer{ desc, v, &Gpu::Vendor, &Gpu::Model, &Gpu::VRamSize };
		printer.print_all();
	}
	return 0;
}