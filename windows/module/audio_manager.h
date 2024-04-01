#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <memory>
#include "audio_decoder.h"
#include "audio_out.h"
#include "audio_cache.h"
#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <variant>

enum AudioEnum {  
    AUDIO_CTL_PLAY,
    AUDIO_CTL_PAUSE,
    AUDIO_CTL_RESUME,
    AUDIO_CTL_STOP,
    AUDIO_CTL_DECODE,
    AUDIO_CTL_OUTPUT,
    AUDIO_CTL_EXIT,
    AUDIO_CTL_SEEK,
    
    AUDIO_STATA_STOPED,
    AUDIO_STATA_PLAYED,
    AUDIO_STATA_PAUSED,
};   

class AudioManager
{

public:

    bool initialize(const std::function<void(AudioEnum)> &callback, const std::function<void(double, double)> &playPrcessBK); // 初始化解码器
    void metadata(const std::string file_path, const std::function<void(std::string, std::string)> &callback);
    void play(const std::string file_path);
    void play();
    void pause();
    void stop();
    void resume();
    void decode();
    void output();
    void exit();
    void seek(double value);
    void setStatus(AudioEnum stata); 
    // 提供一个公共的静态方法来获取Foo的单例对象
    static AudioManager &getInstance()
    {
        return Singleton::getInstance();
    };

private:
    // std::unique_ptr<IAudioOutput> output_;

    std::function<void(AudioEnum)> callback;
    std::function<void(double, double)> playProcessCallBack;
    std::mutex taskMutex;
    std::mutex taskOutMutex;    // 播放锁
    std::condition_variable condition;
    std::condition_variable _out_condition;  // 播放条件变量
    std::future<void> _thread;
    std::future<void> _thread_out;
    bool isStoped = true;
    bool isPaused = false;
    bool waitPlay = false;
    bool waitStata= false;
    std::string file_path;
    PcmFormatInfo speakerInfo;
    double start_pcm_sec = 0.0; 
    double new_pcm_sec = 0.0; 
    std::atomic<bool> out_pause;
    std::atomic<bool> out_stop;

    using AudioVariant = std::variant<std::unique_ptr<double>, std::unique_ptr<IAudioDecoder>, std::unique_ptr<IAudioOutput>>;

    class TaskObj
    {
    public:
        AudioEnum obj_msg;
        std::unique_ptr<AudioVariant> obj_variant;

        TaskObj(AudioEnum msg) : obj_msg(msg), obj_variant(nullptr) {}

        // 直接接受 std::unique_ptr<AudioVariant>，而不是引用
        TaskObj(AudioEnum msg, std::unique_ptr<AudioVariant> variant)
            : obj_msg(msg), obj_variant(std::move(variant)) {}
    };
    using VariantQueue = std::queue<std::unique_ptr<TaskObj>>;
    VariantQueue taskMsgQueue;
    VariantQueue taskOutMsgQueue; // 播放 任务队列

    void loop();  // 解码循环
    void loop_out(); // 播放循环

    AudioManager();
    ~AudioManager();
    AudioManager(const AudioManager &) = delete;
    AudioManager &operator=(const AudioManager &) = delete;

    // 单例类，声明为AudioManager的友元
    class Singleton
    {
    private:
        static AudioManager instance;
        static std::mutex mtx;

        Singleton() = delete;
        Singleton(const Singleton &) = delete;
        Singleton &operator=(const Singleton &) = delete;

    public:
        static AudioManager &getInstance()
        {
            std::lock_guard<std::mutex> lock(mtx);
            return instance;
        }

        // 友元声明，允许Singleton访问AudioManager的私有构造函数
        friend class AudioManager;
    };
};

#endif // AUDIOMANAGER_H
