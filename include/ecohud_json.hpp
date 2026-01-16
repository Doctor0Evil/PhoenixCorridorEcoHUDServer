#ifndef ECOHUD_JSON_HPP
#define ECOHUD_JSON_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include "ecohud_csv.hpp"

namespace ecohud
{
    inline std::string json_escape(const std::string& s)
    {
        std::ostringstream oss;
        for (unsigned char c : s)
        {
            switch (c)
            {
                case '\"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b";  break;
                case '\f': oss << "\\f";  break;
                case '\n': oss << "\\n";  break;
                case '\r': oss << "\\r";  break;
                case '\t': oss << "\\t";  break;
                default:
                    if (c < 0x20)
                    {
                        oss << "\\u"
                            << std::hex
                            << std::uppercase
                            << std::setw(4)
                            << std::setfill('0')
                            << static_cast<int>(c);
                    }
                    else
                    {
                        oss << c;
                    }
                    break;
            }
        }
        return oss.str();
    }

    inline std::string to_json_corridor_tile(
        const std::string& corridor_id,
        const std::vector<SmartCorridorRecord>& all_records
    )
    {
        std::ostringstream oss;
        oss << "{";
        oss << "\"corridor_id\":\"" << json_escape(corridor_id) << "\",";
        oss << "\"indicators\":[";
        bool first = true;
        for (const auto& r : all_records)
        {
            if (r.corridor_id != corridor_id)
            {
                continue;
            }

            if (!first)
            {
                oss << ",";
            }
            first = false;

            oss << "{";
            oss << "\"techcluster\":\"" << json_escape(r.techcluster) << "\",";
            oss << "\"indicator\":\""   << json_escape(r.indicator)   << "\",";
            oss << "\"unit\":\""        << json_escape(r.unit)        << "\",";
            oss << "\"baseline\":"      << r.baseline_value           << ",";
            oss << "\"projected\":"     << r.projected_value          << ",";
            oss << "\"ecoimpact\":"     << r.ecoimpact_score_01       << ",";
            oss << "\"year\":"          << r.year;
            oss << "}";
        }
        oss << "]";
        oss << "}";
        return oss.str();
    }

    inline std::string to_json_city_kpi_tile(
        const std::string& city_id,
        const std::vector<CityKpiRecord>& all_kpis
    )
    {
        std::ostringstream oss;
        oss << "{";
        oss << "\"city_id\":\"" << json_escape(city_id) << "\",";
        oss << "\"kpis\":[";
        bool first = true;
        for (const auto& r : all_kpis)
        {
            if (r.city_id != city_id)
            {
                continue;
            }

            if (!first)
            {
                oss << ",";
            }
            first = false;

            oss << "{";
            oss << "\"kpi_id\":\""     << json_escape(r.kpi_id)      << "\",";
            oss << "\"dimension\":\""  << json_escape(r.dimension)   << "\",";
            oss << "\"indicator\":\""  << json_escape(r.indicator)   << "\",";
            oss << "\"unit\":\""       << json_escape(r.unit)        << "\",";
            oss << "\"baseline\":"     << r.baseline_value           << ",";
            oss << "\"projected\":"    << r.projected_value          << ",";
            oss << "\"ecoimpact\":"    << r.ecoimpact_score_01       << ",";
            oss << "\"year\":"         << r.year;
            oss << "}";
        }
        oss << "]";
        oss << "}";
        return oss.str();
    }

} // namespace ecohud

#endif // ECOHUD_JSON_HPP
