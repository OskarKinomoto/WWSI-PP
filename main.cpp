#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <numeric>
#include <sstream>
#include <iomanip>

struct RawStat{
    int date; ///< date
    double s; ///< time past midnight
    std::string ip_source;
    std::string ip_destination;
    double ntp_s_origin_timestamp;
    double ntp_s_receive_timestamp;
    double ntp_s_transmit_timestamp;
    double ntp_s_destination_timestamp;

    double calc;

    void calculate() {
        calc = (ntp_s_receive_timestamp - ntp_s_origin_timestamp) +
               (ntp_s_destination_timestamp - ntp_s_transmit_timestamp);
    }
};

typedef std::vector<RawStat> RawStats;
typedef std::map<double, size_t, std::greater<double> > WeightedMedianData;

std::string timestampToTImeDate(double timestamp_ms) {
    int s = timestamp_ms;
    int z = s / 86400 + 719468;
    int era = (z >= 0 ? z : z - 146096) / 146097;
    unsigned doe = static_cast<unsigned>(z - era * 146097);
    unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
    int y = static_cast<int>(yoe) + era * 400;
    unsigned doy = doe - (365*yoe + yoe/4 - yoe/100);
    unsigned mp = (5*doy + 2)/153;
    unsigned d = doy - (153*mp+2)/5 + 1;
    unsigned m = mp + (mp < 10 ? 3 : -9);
    y += (m <= 2);
    int sec_in_day = s % (24*60*60);
    int sec = sec_in_day % 60;
    int min_in_day = sec_in_day / 60;
    int min = min_in_day % 60;
    int hr = min_in_day / 60;
    std::stringstream stream;
    stream << y << std::setfill('0') << std::setw(2) << m << "-" << d << " "
      << hr << ":" << min << ":" << sec << std::endl;
    return stream.str();
}

std::istream &operator>>(std::istream& stream, RawStat& rawstat) {
    stream >> rawstat.date >> rawstat.s >> rawstat.ip_source >> rawstat.ip_destination
           >> rawstat.ntp_s_origin_timestamp >> rawstat.ntp_s_receive_timestamp >> rawstat.ntp_s_transmit_timestamp
           >> rawstat.ntp_s_destination_timestamp;

    return stream;
}

WeightedMedianData getWeightedMedianData(RawStats::iterator begin, RawStats::iterator end, std::vector<double> weights) {
    WeightedMedianData data;

    size_t i = 0;
    for (auto iter = begin; iter < end; ++iter) {
        data[iter->calc] += weights.at(i);
        ++i;
    }

    return data;
}

size_t getWeightsSum(const WeightedMedianData& data) {
    return std::accumulate(data.begin(), data.end(), 0ul,
                    [](const size_t previous, const std::pair<double,size_t>& p) { return previous+p.second; });
}

double weightedMedian(const WeightedMedianData& data) {
    const size_t weight_sum = getWeightsSum(data);
    double x_05 = weight_sum * 0.5;
    double sum = 0;
    double median = 0;
    size_t i = 0;
    for (const auto &pair : data) {
        sum += pair.second;

        if (sum >= x_05) {
            median = pair.first;
            break;
        }
    }
    return median;
}

struct Params {
    size_t median_back_log;
    bool is_weights = false;
    std::vector<double> v;
    std::string path = "rawstats";
    std::string out_path = "out";
};

// kroczaca srednia -> EWMA

int main(int argc, char** argv) {

    RawStats stats;
    Params p{.median_back_log = 10};

    if (argc > 1) {
        p.path = argv[1];
    }

    if (argc > 2) {
        p.out_path = argv[2];
    }

    if (argc > 3) {
        p.median_back_log = std::atoi(argv[3]);
    }

    if (argc > 4) {
        p.is_weights = true;
        for (int i = 4; i < argc; ++i) {
            p.v.emplace_back(std::atof(argv[i]));
        }
    }

    std::ifstream file(p.path);
    auto& input = file;

    while (input.good()) {
        RawStat rawStat;
        input >> rawStat;
        rawStat.calculate();
        stats.push_back(rawStat);
    }

    auto weightedMedianData = getWeightedMedianData(stats.begin(), stats.end(), std::vector<double>(stats.size(), 1));

    std::cout << stats.size() << std::endl;
    std::cout << weightedMedian(weightedMedianData) << std::endl;

    std::vector<double> weights_median(p.median_back_log, 1);

    if (p.is_weights)
        weights_median = p.v;

    std::ofstream out(p.out_path);

    for (int i = p.median_back_log, j = 0; i < stats.size(); ++i, ++j) {
        auto data = getWeightedMedianData(stats.begin() + j, stats.begin() + i, weights_median);
        out << j << "\t" << stats.at(i).calc << "\t" << weightedMedian(data) << std::endl;
    }

    return 0;
}