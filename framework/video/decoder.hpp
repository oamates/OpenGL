#ifndef _decoder_included_09881451047604113453140164151374851682451024651048325
#define _decoder_included_09881451047604113453140164151374851682451024651048325

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <atomic>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>

struct player_t;

enum struct playback_state_t : unsigned int
    {NOT_LOADED, LOADING, LOADED, PAUSED, PLAYING, ERROR};

struct frame_handle_t
{
    AVFrame* frame = 0;
    uint8_t* frame_buffer = 0;
};

struct player_control_t
{
    uint32_t m_frame_position = 0;                      // current position [frame index]
    int64_t m_duration = 0;                             // ms
    int64_t m_frame_count = 0;                          // not possible or seek to file end [frames count]
    int timer_interval = 0;

    mutable std::recursive_mutex lock;

    playback_state_t playback_state;

    const char* video;

    std::atomic<bool> quit;
    std::map<int64_t, frame_handle_t> buffer;

    std::thread* thread = 0;
    std::condition_variable_any condWait;

    int num_bytes = 0;
    uint8_t* pictBuffer = 0;

    player_control_t();
    ~player_control_t();

    void init(const char*, player_t *p);
    void deinit();

    void play();
    void pause();
    void set_frame(uint32_t index);
    void next_frame();

    frame_handle_t current_frame() const;

    playback_state_t state() const;

    int64_t duration() const;
    int64_t frame_count() const;
    uint32_t frame_position() const;
    const char* source() const { return video; }


    friend void decoder_thread(player_control_t*);
    friend bool init_decoder(player_control_t*);

    void setState(playback_state_t state);
    void setFrameCount(int64_t frameCount);
    void setDuration(int64_t ms);
    void clearBuffer();

};

#endif // _decoder_included_09881451047604113453140164151374851682451024651048325
