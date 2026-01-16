#ifndef ECOHUD_CSV_HPP
#define ECOHUD_CSV_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace ecohud
{
    struct SmartCorridorRecord
    {
        std::string corridor_id;
        std::string region;
        std::string techcluster;
        std::string indicator;
        std::string unit;
        double      baseline_value;
        double      projected_value;
        double      ecoimpact_score_01;
        int         year;
    };

    struct CityKpiRecord
    {
        std::string city_id;
        std::string region;
        std::string kpi_id;
        std::string dimension;
        std::string indicator;
        std::string unit;
        double      baseline_value;
        double      projected_value;
        double      ecoimpact_score_01;
        int         year;
    };

    namespace detail
    {
        inline std::string trim(const std::string& s)
        {
            std::size_t start = 0;
            while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
            {
                ++start;
            }

            std::size_t end = s.size();
            while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
            {
                --end;
            }

            return s.substr(start, end - start);
        }

        inline std::vector<std::string> split_csv_line(const std::string& line)
        {
            std::vector<std::string> result;
            std::string current;
            bool in_quotes = false;

            for (char ch : line)
            {
                if (ch == '"')
                {
                    in_quotes = !in_quotes;
                    continue;
                }

                if (ch == ',' && !in_quotes)
                {
                    result.push_back(trim(current));
                    current.clear();
                }
                else
                {
                    current.push_back(ch);
                }
            }

            result.push_back(trim(current));
            return result;
        }

        inline double parse_double(const std::string& s)
        {
            if (s.empty())
            {
                return 0.0;
            }
            char* end = nullptr;
            const double val = std::strtod(s.c_str(), &end);
            if (end == s.c_str())
            {
                return 0.0;
            }
            return val;
        }

        inline int parse_int(const std::string& s)
        {
            if (s.empty())
            {
                return 0;
            }
            char* end = nullptr;
            const long val = std::strtol(s.c_str(), &end, 10);
            if (end == s.c_str())
            {
                return 0;
            }
            return static_cast<int>(val);
        }
    } // namespace detail

    inline std::vector<SmartCorridorRecord> load_smart_corridor_csv(
        const std::string& filepath
    )
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open SmartCorridor CSV: " + filepath);
        }

        std::string header_line;
        if (!std::getline(file, header_line))
        {
            throw std::runtime_error("SmartCorridor CSV is empty: " + filepath);
        }

        const auto header_cols = detail::split_csv_line(header_line);
        if (header_cols.size() < 9)
        {
            throw std::runtime_error("SmartCorridor CSV header has insufficient columns");
        }

        std::vector<SmartCorridorRecord> result;
        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }

            const auto cols = detail::split_csv_line(line);
            if (cols.size() < 9)
            {
                continue;
            }

            SmartCorridorRecord r;
            r.corridor_id        = cols[0];
            r.region             = cols[1];
            r.techcluster        = cols[2];
            r.indicator          = cols[3];
            r.unit               = cols[4];
            r.baseline_value     = detail::parse_double(cols[5]);
            r.projected_value    = detail::parse_double(cols[6]);
            r.ecoimpact_score_01 = detail::parse_double(cols[7]);
            r.year               = detail::parse_int(cols[8]);

            result.push_back(r);
        }

        return result;
    }

    inline std::vector<CityKpiRecord> load_city_kpi_csv(
        const std::string& filepath
    )
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open City KPI CSV: " + filepath);
        }

        std::string header_line;
        if (!std::getline(file, header_line))
        {
            throw std::runtime_error("City KPI CSV is empty: " + filepath);
        }

        const auto header_cols = detail::split_csv_line(header_line);
        if (header_cols.size() < 10)
        {
            throw std::runtime_error("City KPI CSV header has insufficient columns");
        }

        std::vector<CityKpiRecord> result;
        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }

            const auto cols = detail::split_csv_line(line);
            if (cols.size() < 10)
            {
                continue;
            }

            CityKpiRecord r;
            r.city_id            = cols[0];
            r.region             = cols[1];
            r.kpi_id             = cols[2];
            r.dimension          = cols[3];
            r.indicator          = cols[4];
            r.unit               = cols[5];
            r.baseline_value     = detail::parse_double(cols[6]);
            r.projected_value    = detail::parse_double(cols[7]);
            r.ecoimpact_score_01 = detail::parse_double(cols[8]);
            r.year               = detail::parse_int(cols[9]);

            result.push_back(r);
        }

        return result;
    }

} // namespace ecohud

#endif // ECOHUD_CSV_HPP
