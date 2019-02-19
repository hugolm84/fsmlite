#include <cassert>
#include <iostream>
#include <string>

#include "fsm.h"

class player: public fsmlite::fsm<player> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Stopped, Open, Empty, Playing, Paused };

    player(state_type init_state = Empty) : fsm(init_state) { }

    void autoplay(bool f) {
        m_autoplay = f;
    }

    struct play {};
    struct open_close {};
    struct cd_detected { std::string title; };
    struct stop {};
    struct pause {};

private:
    bool is_autoplay(const cd_detected&) const {
        return m_autoplay;
    }

    bool is_bad_cd(const cd_detected& cd) const {
        return cd.title.empty();
    }

    void start_playback(const play&);
    void start_playback(const cd_detected& cd);
    void open_drawer(const open_close&);
    void open_drawer(const cd_detected& cd);
    void close_drawer(const open_close&);
    void store_cd_info(const cd_detected& cd);
    void stop_playback(const stop&);
    void pause_playback(const pause&);
    void stop_and_open(const open_close&);
    void resume_playback(const play&);

private:
    using m = player;  // for brevity

    using transition_table = table<
//              Start    Event        Target   Action              Guard (optional)
//  -----------+--------+------------+--------+-------------------+---------------+-
    mem_fn_row< Stopped, play,        Playing, &m::start_playback                  >,
    mem_fn_row< Stopped, open_close,  Open,    &m::open_drawer                     >,
    mem_fn_row< Open,    open_close,  Empty,   &m::close_drawer                    >,
    mem_fn_row< Empty,   open_close,  Open,    &m::open_drawer                     >,
    mem_fn_row< Empty,   cd_detected, Open,    &m::open_drawer,    &m::is_bad_cd   >,
    mem_fn_row< Empty,   cd_detected, Playing, &m::start_playback, &m::is_autoplay >,
    mem_fn_row< Empty,   cd_detected, Stopped, &m::store_cd_info   /* fallback */  >,
    mem_fn_row< Playing, stop,        Stopped, &m::stop_playback                   >,
    mem_fn_row< Playing, pause,       Paused,  &m::pause_playback                  >,
    mem_fn_row< Playing, open_close,  Open,    &m::stop_and_open                   >,
    mem_fn_row< Paused,  play,        Playing, &m::resume_playback                 >,
    mem_fn_row< Paused,  stop,        Stopped, &m::stop_playback                   >,
    mem_fn_row< Paused,  open_close,  Open,    &m::stop_and_open                   >
//  -----------+--------+------------+--------+-------------------+---------------+-
    >;

private:
    bool m_autoplay = false;
};

void player::start_playback(const play&)
{
    std::cout << "Starting playback\n";
}

void player::start_playback(const cd_detected& cd)
{
    std::cout << "Starting playback of '" << cd.title << "'\n";
}

void player::open_drawer(const open_close&)
{
    std::cout << "Opening drawer\n";
}

void player::open_drawer(const cd_detected&)
{
    std::cout << "Ejecting bad CD\n";
}

void player::close_drawer(const open_close&)
{
    std::cout << "Closing drawer\n";
}

void player::store_cd_info(const cd_detected& cd)
{
    std::cout << "Detected CD '" << cd.title << "'\n";
}

void player::stop_playback(const stop&)
{
    std::cout << "Stopping playback\n";
}

void player::pause_playback(const pause&)
{
    std::cout << "Pausing playback\n";
}

void player::stop_and_open(const open_close&)
{
    std::cout << "Stopping and opening drawer\n";
}

void player::resume_playback(const play&)
{
    std::cout << "Resuming playback\n";
}

void test_player()
{
    player p;
    assert(p.current_state() == player::Empty);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Open);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Empty);
    p.process_event(player::cd_detected{"louie, louie"});
    assert(p.current_state() == player::Stopped);
    p.process_event(player::play());
    assert(p.current_state() == player::Playing);
    p.process_event(player::pause());
    assert(p.current_state() == player::Paused);
    p.process_event(player::play());
    assert(p.current_state() == player::Playing);
    p.process_event(player::stop());
    assert(p.current_state() == player::Stopped);
    p.process_event(player::play());
    assert(p.current_state() == player::Playing);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Open);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Empty);
    p.process_event(player::play());
    assert(p.current_state() == player::Empty);
}

void test_bad_cd()
{
    player p;
    assert(p.current_state() == player::Empty);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Open);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Empty);
    p.process_event(player::cd_detected{});
    assert(p.current_state() == player::Open);
}

void test_autoplay()
{
    player p;
    p.autoplay(true);
    assert(p.current_state() == player::Empty);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Open);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Empty);
    p.process_event(player::cd_detected{"louie, louie"});
    assert(p.current_state() == player::Playing);
}

int main()
{
    test_player();
    test_bad_cd();
    test_autoplay();
    return 0;
}
