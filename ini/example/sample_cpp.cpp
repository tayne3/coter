#include <iostream>

#include "coter/ini/ini.h"

int main() {
    try {
        coter::ini::Ini ini;
        auto            section = ini.getSection("section");
        section.addKey("key", "1");
        section.addKey("enabled", "true").setValue(true);
        section.addKey("timeout").setValue(30.5);
        section.addKey("name", "coter");

        auto key = ini.getKey("section", "key");
        std::cout << "Int value: " << key.getInt(0) << std::endl;
        std::cout << "Bool value: " << ini.findKey("section", "enabled").getBool(false) << std::endl;
        std::cout << "Double value: " << ini.findSection("section").findKey("timeout").getDouble(0.0) << std::endl;
        std::cout << "String value: " << ini.findKey("section", "name").getString("default") << std::endl;

        std::cout << std::endl << "Total sections: " << ini.sections().size() << std::endl;
        for (auto s = ini.firstSection(); s; s = s.next()) {
            std::cout << "[" << s.name() << "]" << std::endl;
            for (auto k = s.firstKey(); k; k = k.next()) {
                std::cout << "  " << k.name() << " = " << k.getValue() << std::endl;
            }
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
