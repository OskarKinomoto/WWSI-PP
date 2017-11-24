#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <numeric>

/*
Witam,

Do algorytmu ważonych median powinien wziąć Pan liczby które są obliczone z plików ze statystykami .
Liczby te należy obliczać jako suma różnic : receive timestamp i origin timestamp oraz destination timestamp i transmit timestamp. W ten sposób każdy wiersz da Panu jedną liczbę.
W dokumentacji pod linkiem: https://www.eecis.udel.edu/~mills/ntp/html/monopt.html znajdzie Pan opis poszczególnych kolumn (w tym origin, receive, transmit i destination timestamp).

W razie problemów proszę o e-mail.

 */

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

std::istream &operator>>(std::istream& stream, RawStat& rawstat) {
    stream >> rawstat.date >> rawstat.s >> rawstat.ip_source >> rawstat.ip_destination
           >> rawstat.ntp_s_origin_timestamp >> rawstat.ntp_s_receive_timestamp >> rawstat.ntp_s_transmit_timestamp
           >> rawstat.ntp_s_destination_timestamp;

    return stream;
}

typedef std::vector<RawStat> RawStats;
typedef std::map<double, size_t, std::greater<double>> WeightedMedianData;


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

/*
 *
Odnośnie Pana pytania - powinien Pan liczyć ważoną medianę w sposób kroczący tzn. pobierać kolejne wartości z plików z pomiarami tzn. (z tych które przesłałem Panu), wykonywać obliczenia i zapisywać wynik do pliku. W obliczenia ważonej mediany proszę przyjąć długość pamięci wstecz tzn. ile poprzednich próbek branych jest do wyliczenia ważonej mediany w każdym kroku. Parametry: długość pamięci wstecz oraz wagi (dla każdej poprzedniej próbki aż do maksymalnej pamięci wstecz) powinny być podawane przez użytkownika (nie powinny być zaszyte na stałe w algorytmie).
 *
 */

struct Params {
    size_t median_back_log;
};

int main() {
    std::ifstream file("rawstats");
    auto& input = file;

    RawStats stats;
    Params p{.median_back_log = 10};

    while (input.good()) {
        RawStat rawStat;
        input >> rawStat;
        rawStat.calculate();
        stats.push_back(rawStat);
    }

    auto weightedMedianData = getWeightedMedianData(stats.begin(), stats.end(), std::vector<double>(stats.size(), 1));

    std::cout << stats.size() << std::endl;
    std::cout << weightedMedian(weightedMedianData) << std::endl;

    for (int i = p.median_back_log, j = 0; i < stats.size(); ++i, ++j) {
        auto data = getWeightedMedianData(stats.begin() + j, stats.begin() + i, std::vector<double>(p.median_back_log, 1));
        std::cout << j << "\t" << stats.at(i).calc << "\t" << weightedMedian(data) << std::endl;
    }

    return 0;
}