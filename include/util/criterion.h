#ifndef IZENELIB_UTIL_CRITERION_H_
#define IZENELIB_UTIL_CRITERION_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

#include <boost/function.hpp>

namespace izenelib{namespace util{namespace criterion{

class cmdline
{
public:
    cmdline()
    {
        init();
    }

    const std::vector<std::string> &args() const
    {
        return args_;
    }

private:
    void init()
    {
        std::string arg = get_cmdline();
        for (size_t i = 0; i < arg.size(); ++i)
        {
            size_t st = i;
            while (i < arg.size() && arg[i] != '\0') ++i;
            args_.push_back(arg.substr(st, i - st));
        }
    }

    static std::string get_cmdline()
    {
        pid_t pid = getpid();

        char fname[32] = {};
        sprintf(fname, "/proc/%d/cmdline", pid);
        std::ifstream ifs(fname);
        if (!ifs)
            throw std::runtime_error("cannot get cmdline");

        std::stringstream ss;
        ifs >> ss.rdbuf();
        return ss.str();
    }

    std::vector<std::string> args_;
};

class clock_time
{
public:
    clock_time(uint64_t sec, int32_t usec)
        : sec(sec)
        , usec(usec)
    {
    }

    operator double() const
    {
        return sec + (double)usec*1e-6;
    }

    clock_time &operator+=(const clock_time &ct)
    {
        sec += ct.sec;
        usec += ct.usec;
        while(usec >= 1000000)
        {
            sec += 1;
            usec -= 1000000;
        }
        while(usec < 0)
        {
            sec -= 1;
            usec += 1000000;
        }
        return *this;
    }

    clock_time &operator-=(const clock_time &ct)
    {
        sec -= ct.sec;
        usec -= ct.usec;
        while(usec >= 1000000)
        {
            sec+=1;
            usec-=1000000;
        }
        while(usec<0)
        {
            sec-=1;
            usec+=1000000;
        }
        return *this;
    }

    const clock_time operator+(const clock_time &ct) const
    {
        clock_time ret(*this);
        ret += ct;
        return ret;
    }

    const clock_time operator-(const clock_time &ct) const
    {
        clock_time ret(*this);
        ret -= ct;
        return ret;
    }

    bool operator<(const clock_time &ct) const
    {
        if (sec != ct.sec) return sec < ct.sec;
        return usec < ct.usec;
    }

    uint64_t sec;
    int32_t usec;
};

inline clock_time get_clock_time()
{
    timeval tv = {};
    int res = gettimeofday(&tv, NULL);
    if (res!=0) return clock_time(0,0);
    return clock_time(tv.tv_sec, tv.tv_usec);
}

inline std::string format_time(double span)
{
    char buf[32] = {};

    if (span >= 1.0)
    {
        sprintf(buf, "%.3f s", span);
        return std::string(buf);
    }

    span *= 1000;
    if (span >= 1.0)
    {
        sprintf(buf, "%.3f ms", span);
        return std::string(buf);
    }

    span *= 1000;
    if (span >= 1.0)
    {
        sprintf(buf, "%.3f us", span);
        return std::string(buf);
    }

    span *= 1000;
    sprintf(buf, "%.3f ns", span);
    return std::string(buf);
}

class estimate
{
public:
    estimate()
        : point(0), lower_bound(0), upper_bound(0), confidence_level(0) {}

    estimate(double p, double l, double u, double ci)
        : point(p), lower_bound(l), upper_bound(u), confidence_level(ci) {}

    double point;
    double lower_bound;
    double upper_bound;
    double confidence_level;
};

typedef std::vector<double> sample;
typedef boost::function<double(sample)> estimator;

class mean
{
public:
    double operator()(const sample &samp) const
    {
        double m = 0, n = 0;
        for (size_t i = 0; i < samp.size(); ++i)
        {
            ++n;
            m = m + (samp[i] - m) / n;
        }
        return m;
    }
};

inline double sqr(double d)
{
    return d * d;
}

inline double robust_sum_var(double m, const sample &samp)
{
    double ret = 0;
    for (size_t i = 0; i < samp.size(); ++i)
    {
        ret += sqr(samp[i] - m);
    }
    return ret;
}

inline double variance_unbiased(const sample &samp)
{
    if (samp.empty()) return 0;
    return robust_sum_var(mean()(samp), samp) / (samp.size() - 1);
}

class std_dev
{
public:
    double operator()(const sample &samp) const
    {
        return sqrt(variance_unbiased(samp));
    }
};

class normal_distribution
{
public:
    normal_distribution(double m, double v, double p, double c)
        : mean(m), variance(v), pdf_denom(p), cdf_denom(c) {}

    double mean;
    double variance;
    double pdf_denom;
    double cdf_denom;
};

const normal_distribution &standard()
{
    static const double m_pi = atan(1.0) * 4;
    static const double m_sqrt_2 = sqrt(2.0);
    static const double m_sqrt_2_pi = sqrt(2.0 * m_pi);

    static const normal_distribution
    nd(0.0, 1.0, m_sqrt_2_pi, m_sqrt_2);

    return nd;
}

inline double cumlative_normal(const normal_distribution &d, double x);
inline double density_normal(const normal_distribution &d, double p);
inline double quantile_normal(const normal_distribution &d, double p);

inline double find_root(const normal_distribution &d,
                        double prob,
                        double x,
                        double lo,
                        double hi)
{
    const int max_iters = 150;
    const double accuracy = 1e-15;

    double dx = 1;
    for (int i = 0; abs(dx) > accuracy && i < max_iters; ++i)
    {
        double err = cumlative_normal(d, x) - prob;
        if (err < 0) lo = x;
        else hi = x;
        double pdf = density_normal(d, x);
        if (pdf != 0) dx = err / pdf, x -= dx;
        if (x < lo || x > hi || pdf == 0)
        {
            double y = (lo + hi) / 2;
            dx = y-x;
            x = y;
        }
    }
    return x;
}

inline double density_normal(const normal_distribution &d, double x)
{
    double xm = x - d.mean;
    return exp(-xm * xm / (2 * d.variance)) / d.pdf_denom;
}

inline double cumlative_normal(const normal_distribution &d, double x)
{
    return erfc((d.mean - x) / d.cdf_denom) / 2;
}

inline double quantile_normal(const normal_distribution &d, double p)
{
    if (p < 0 || p > 1) return INFINITY/INFINITY;
    if (p == 0) return -INFINITY;
    if (p == 1) return INFINITY;
    if (p == 0.5) return d.mean;

    double x = find_root(standard(), p, 0, -100, 100);
    return x * sqrt(d.variance) + d.mean;
}

class group
{
public:
    group(const std::string &id)
        : identifier_(id)
    {
    }
    ~group()
    {
    }

    operator bool() const
    {
        return false;
    }

    const std::string &identifier() const
    {
        return identifier_;
    }

private:
    const std::string identifier_;
};

class bootstrapping
{
public:
    static void resample(std::vector<estimator> &ests,
                         size_t num_resamples,
                         const sample &s,
                         std::vector<sample> &resamples)
    {
        resamples.resize(ests.size());
        sample re(s.size());

        for (size_t i = 0; i < num_resamples; ++i)
        {
            for (size_t j = 0; j < s.size(); ++j)
                re[j] = s[rand() % s.size()];

            for (size_t j = 0; j < ests.size(); ++j)
            {
                resamples[j].push_back(ests[j](re));
            }
        }

        for (size_t i = 0; i < resamples.size(); ++i)
        {
            std::sort(resamples[i].begin(), resamples[i].end());
        }
    }

    static void jackknife(estimator &est,
                          const sample &samp,
                          std::vector<double> &ret)
    {
        ret.resize(samp.size());
        for (size_t i = 0; i < samp.size(); ++i)
        {
            sample tmp;
            tmp.reserve(samp.size() - 1);
            for (size_t j = 0; j < samp.size(); ++j)
            {
                if (i != j) tmp.push_back(samp[j]);
            }
            ret[i] = est(tmp);
        }
    }

    static void bca(double confidence_level,
                    const sample &samp,
                    std::vector<estimator> &ests,
                    const std::vector<sample> &res,
                    std::vector<estimate> &result)
    {

        assert(confidence_level > 0 && confidence_level < 1);
        assert(ests.size() == res.size());

        for (size_t i = 0; i < ests.size(); ++i)
        {
            const sample &resample = res[i];
            double pt = ests[i](samp);

            if (samp.size() == 1)
            {
                result.push_back(estimate(pt, pt, pt, confidence_level));
                continue;
            }

            double z1 = quantile_normal(standard(), (1 - confidence_level) / 2);

            int prob_n = 0;
            for (size_t j = 0; j < resample.size(); ++j)
                if (resample[j] < pt)
                    prob_n++;

            double bias = quantile_normal(standard(), (double)prob_n / resample.size());

            std::vector<double> jack;
            jackknife(ests[i], samp, jack);

            double jack_mean = mean()(jack);

            double sum_squares = 0, sum_cubes = 0;
            for (size_t j = 0; j < jack.size(); ++j)
            {
                double d = jack_mean - jack[j];
                sum_squares += d * d;
                sum_cubes += d * d * d;
            }

            double accel = sum_cubes / (6 * pow(sum_squares, 1.5));

            double b1 = bias + z1;
            double a1 = bias + b1 / (1 - accel * b1);
            int lo = std::max(0,
                              (int)round(cumlative_normal(standard(), a1) * resample.size()));

            double b2 = bias - z1;
            double a2 = bias + b2 / (1 - accel * b2);
            int hi = std::min((int)resample.size() - 1,
                              (int)round(cumlative_normal(standard(), a2) * resample.size()));

            result.push_back(estimate(pt, resample[lo], resample[hi], confidence_level));
        }
    }
};

class statistics
{
public:
    statistics(const std::string &id, group &gr, int t = 100)
        : identifier_(id), gr_(gr), times_(t)
        , count_(0), iters_(0), icnt_(0)
        , start_(0, 0), end_(0, 0)
    {
        std::cout << "benchmarking "
                  << gr.identifier()
                  << "/"
                  << id
                  << std::endl;
        start_ = get_clock_time();
    }

    ~statistics()
    {
        std::cout << "collecting " << ((int)lap_time_.size() - 1) << " samples, "
                  << iters_ << " iterations each, "
                  << "in estimated " << format_time(end_ - start_)
                  << std::endl;

        sample samp(lap_time_.size() - 0.1);
        for (size_t i = 0; i + 1 < lap_time_.size(); ++i)
            samp[i] = std::max(0.0, (double)(lap_time_[i + 1] - lap_time_[i]) / iters_);

        analyze(samp);

        std::cout << std::endl;
    }

    bool lap()
    {
        ++icnt_;

        if (iters_ == 0)
        {
            clock_time cur = get_clock_time();

            if (cur - start_ >= 0.1)
            {
                iters_ = std::max(icnt_, 2) - 1;
                icnt_ = 0;
                lap_time_.reserve(times_ + 1);
                start_ = get_clock_time();
                lap_time_.push_back(get_clock_time());
            }

            return true;
        }

        if (icnt_ < iters_)
            return true;

        icnt_ = 0;
        ++count_;

        lap_time_.push_back(get_clock_time());
        if (count_ >= times_)
        {
            end_ = get_clock_time();
            return false;
        }
        return true;
    }

private:

    void analyze(const sample & samp)
    {
        assert(samp.size() > 0);

        size_t num_resamples = 100000;
        std::cout << "bootstrapping with " << num_resamples
                  << " resamples" << std::endl;

        std::vector<estimator> ests;
        ests.push_back(mean());
        ests.push_back(std_dev());

        const char *names[] =
        {
            "mean",
            "std dev",
        };

        std::vector<sample> res;
        bootstrapping::resample(ests, num_resamples, samp, res);

        double ci = 0.95;
        std::vector<estimate> es;
        bootstrapping::bca(ci, samp, ests, res, es);

        for (size_t i = 0; i < es.size(); ++i)
        {
            std::cout << names[i] << ": "
                      << format_time(es[i].point) <<", "
                      << "lb " << format_time(es[i].lower_bound) << ", "
                      << "ub " << format_time(es[i].upper_bound) << ", "
                      << "ci " << es[i].confidence_level
                      << std::endl;
        }
    }

    const std::string identifier_;
    group gr_;
    const int times_;
    int count_;
    int iters_;
    int icnt_;
    clock_time start_, end_;
    std::vector<clock_time> lap_time_;

    static const int resamples_ = 100000;
};

#define bgroup(group_name)                                              \
  if (izenelib::util::criterion::group BENCH_MUST_BE_IN_BGROUP_ = izenelib::util::criterion::group(group_name)) {} \
  else

#define bench(bench_name)                                               \
  for (izenelib::util::criterion::statistics BENCH_STAT_ = izenelib::util::criterion::statistics(bench_name, BENCH_MUST_BE_IN_BGROUP_); \
       BENCH_STAT_.lap(); )

}}}

#endif /* IZENELIB_UTIL_CRITERION_H_ */
