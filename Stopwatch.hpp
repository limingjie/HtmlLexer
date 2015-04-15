//
// Stopwatch - measure accurate time
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
// stopwatch - Measure execution time of function or any piece of code.
//
// Prerequisite - c++11 library <chrono>
//
// - Measure function execution time
//   void function()
//   {
//       stopwatch<double> time("name"); // stopwatch starts
//       [Some Code]
//   } // stopwatch stops since out of scope.
//
// - Measure execution time of certain code
//   // stopwatch declared and paused
//   stopwatch<> *time = new stopwatch<>("name");
//   while (true)
//   {
//       time->start();
//       [Some Logic]
//       time->pause();
//       [Console Output]
//       time->resume();
//       [Other Logic]
//       time->stop();
//       [Console Output]
//   }
//   delete time; // stopwatch stops by explicitly calling destructor.
//
template <typename T = int, typename R = std::milli>
class stopwatch
{
private:
    typedef std::chrono::high_resolution_clock              hr_clock;
    typedef std::chrono::high_resolution_clock::duration    hr_duration;
    typedef std::chrono::high_resolution_clock::time_point  hr_time_point;

    enum state_type
    {
        state_start,
        state_pause,
        state_resume,
        state_stop
    };

    std::string    m_name;
    bool           m_ticking;
    hr_duration    m_duration;   // sum(pause - resume)
    hr_time_point  m_time_point; // Last time that starts ticking

public:
    stopwatch(std::string name = "stopwatch")
    {
        m_name    = name;
        m_ticking = false;
        reset();
    }

    virtual ~stopwatch()
    {
        stop();
    }

    void reset()
    {
        if (m_ticking)
        {
            stop();
        }

        m_duration = hr_duration::zero();
    }

    void start()
    {
        reset();

        print(state_start);
        m_ticking = true;
        m_time_point = hr_clock::now();
    }

    void pause()
    {
        if (m_ticking)
        {
            m_duration += hr_clock::now() - m_time_point;
            m_ticking = false;
            print(state_pause);
        }
    }

    void resume()
    {
        if (!m_ticking)
        {
            print(state_resume);
            m_ticking = true;
            m_time_point = hr_clock::now();
        }
    }

    void stop()
    {
        if (m_ticking)
        {
            m_duration += hr_clock::now() - m_time_point;
            m_ticking = false;
            print(state_stop);
        }
    }

private:
    void print(state_type state)
    {
        using namespace std;
        using namespace std::chrono;

        ios::fmtflags fmt(cerr.flags()); // keep cerr format

        // Print stopwatch name, e.g.
        // stopwatch [name            ]
        cerr << "[" << left << setw(16) << m_name << right << "]";

        if (state == state_start)
        {
            cerr << " starts...";
        }
        else if (state == state_resume)
        {
            cerr << " resume...";
        }
        else
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
            if (state == state_pause)
            {
                cerr << " paused. ";
            }
            else if (state == state_stop)
            {
                cerr << " done.   ";
            }

            cerr << setw(3) << min << "m"
                 << setfill('0')
                 << setw(2) << sec << "."
                 << setw(3) << msec << "s / "
                 << setfill(' ')
                 << setprecision(3) << fixed
                 << setw(12) << ticks << " x "
                 << R::num << '/' << R::den << "s";
        }

        cerr << endl;

        cerr.flags(fmt); // restore cerr format
    }
};
