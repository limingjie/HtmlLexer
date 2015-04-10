//
// Algorithm Practice - Stopwatch
//
// Mingjie Li (limingjie0719@gmail.com)
// Mar 14, 2014
//
// Compiled with
// - MinGW g++ 4.8.2
// - Visual Studio Express 2013
//

#include <chrono>   // clock, duration, time_point
#include <ratio>    // ratio
#include <string>   // string
#include <iostream> // cerr
#include <iomanip>  // fmt

//
// Stopwatch - Measure execution time of function or any piece of code.
//
// Prerequisite - c++11 library <chrono>
//
// - Measure function execution time
//   void function()
//   {
//       Stopwatch<double> time("name", true); // Stopwatch starts
//       [Some Code]
//   } // Stopwatch stops since out of scope.
//
// - Measure execution time of certain code
//   // Stopwatch declared and paused
//   Stopwatch<> *time = new Stopwatch<>("name");
//   while (true)
//   {
//       time->start();
//       [Core Logic]
//       time->pause();
//       [Console Output]
//   }
//   delete time; // Stopwatch stops by explicitly calling destructor.
//
template <typename T = int, typename R = std::milli>
class Stopwatch
{
    typedef std::chrono::high_resolution_clock              hr_clock;
    typedef std::chrono::high_resolution_clock::duration    hr_duration;
    typedef std::chrono::high_resolution_clock::time_point  hr_time_point;

public:
    Stopwatch(std::string name = "", bool ticking = false)
    {
        m_name     = name;
        m_ticking  = ticking;
        m_duration = hr_duration::zero();

        if (m_ticking)
        {
            print();
            m_time_point = hr_clock::now();
        }
    }

    virtual ~Stopwatch()
    {
        pause();
        print(true);
    }

    void pause()
    {
        if (m_ticking)
        {
            m_duration += hr_clock::now() - m_time_point;
            m_ticking = false;
        }
    }

    void start()
    {
        if (!m_ticking)
        {
            print();
            m_ticking = true;
            m_time_point = hr_clock::now();
        }
    }

    inline void resume() { start(); }
    inline void stop()   { pause(); }

private:
    std::string    m_name;
    bool           m_ticking;
    hr_duration    m_duration;   // sum(pause - resume)
    hr_time_point  m_time_point; // Last time that starts ticking

    void print(bool isPaused = false)
    {
        using namespace std;
        using namespace std::chrono;

        ios::fmtflags fmt(cerr.flags()); // keep cerr format

        // Print Stopwatch name, e.g.
        // Stopwatch [name            ]
        cerr << "[" << left << setw(16) << m_name << right << "]";

        if (isPaused)
        {
            unsigned long long min, sec, msec, usec;

            // Default duration
            usec = duration_cast<std::chrono::microseconds>(m_duration).count();

            // Roundup usec to msec
            usec += 500ULL;

            // 0m00.000s
            min  =  usec / 60000000ULL;
            sec  = (usec /  1000000ULL) %   60ULL;
            msec = (usec /     1000ULL) % 1000ULL;

            // Duration specified by template.
            T ticks = duration_cast<duration<T, R>>(m_duration).count();

            // Print execution time, e.g.
            // elapsed   0m11.621s /    11620.665 ticks of 1/1000s
            cerr << " paused. "
                << setw(3) << min << "m"
                << setfill('0')
                << setw(2) << sec << "."
                << setw(3) << msec << "s / "
                << setfill(' ')
                << setprecision(3) << fixed
                << setw(12) << ticks << " ticks of "
                << R::num << '/' << R::den << "s";
        }
        else
        {
            cerr << " starts...";
        }

        cerr << endl;

        cerr.flags(fmt); // restore cerr format
    }
};
