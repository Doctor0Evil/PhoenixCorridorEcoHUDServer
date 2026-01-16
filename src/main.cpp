#include <iostream>
#include <string>

#include "ecohud_csv.hpp"
#include "ecohud_server.hpp"

int main(int argc, char** argv)
{
    try
    {
        const int port = (argc > 1) ? std::stoi(argv[1]) : 8080;

        const std::string smart_corridor_path =
            "../qpudatashards/particles/SmartCorridorEcoImpact2026v1.csv";
        const std::string city_kpi_path =
            "../qpudatashards/particles/CityEcoKPIIntegratedSmartSystems2026v1.csv";

        auto smart_corridor_data = ecohud::load_smart_corridor_csv(smart_corridor_path);
        auto city_kpi_data       = ecohud::load_city_kpi_csv(city_kpi_path);

        ecohud::EcoHudServer server(port, std::move(smart_corridor_data), std::move(city_kpi_data));
        server.start();

        std::cout << "Phoenix Corridor EcoHUD Server running on port " << port << "\n";
        std::cout << "Try: GET /corridor?id=PHX-GRID-001 or GET /city?id=PHX-001\n";
        std::cout << "Press ENTER to stop.\n";
        std::string line;
        std::getline(std::cin, line);

        server.stop();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
