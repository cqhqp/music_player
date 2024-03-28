#include "audio_manager.h"
#include "audio_metadata.h"
#include <iostream>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>

#include <glog/export.h>
#include <glog/logging.h>

// 在类外部定义 Singleton 的静态成员
std::mutex AudioManager::Singleton::mtx;        // 静态成员 mtx 的定义
AudioManager AudioManager::Singleton::instance; // 静态成员 instance 的定义

bool AudioManager::initialize(const std::function<void(AudioEnum)> &callback, const std::function<void(double, double)> &playPrcessBK)
{
    LOG(INFO) << "initialize.";
    this->callback = callback;
    this->playProcessCallBack = playPrcessBK;
    _thread = std::async(std::launch::async, &AudioManager::loop, this);
    isStoped = true;
    isPaused = false;
    waitPlay = false;
    return true;
}

AudioManager::AudioManager()
{
}

AudioManager::~AudioManager()
{
    exit();
    // 等待解码线程完成
    if (_thread.valid())
    {
        _thread.get();
    }
}

void AudioManager::metadata(const std::string file_path, const std::function<void(std::string, std::string)> &callback)
{
    LOG(INFO) << "metadata.";
    AudioMetadata metadata(file_path);
    metadata.parse(callback);
}

// 结束前面的解码线程，播放线程，清空并初始化buf
void AudioManager::play(const std::string file_path)
{
    LOG(INFO) << "play.";
    this->file_path = file_path;
    if (!isStoped)
    {
        stop();
        waitPlay = true;
    }
    else
    {
        play();
    }
}

void AudioManager::play()
{
    if (waitStata)
    {
        return;
    }

    waitStata = true;
    std::unique_ptr<AudioDecoder> decoder_;
    decoder_ = std::make_unique<Mp3Decoder>();
    bool init_ok = decoder_->initialize(file_path, playProcessCallBack);
    if (init_ok)
    {
        LOG(INFO) << "[1] play lock.";
        std::unique_ptr<AudioVariant> audioVariant = std::make_unique<AudioVariant>(std::move(decoder_));

        std::unique_ptr<TaskObj> taskObj = std::make_unique<TaskObj>(AUDIO_CTL_PLAY, std::move(audioVariant));
        std::lock_guard<std::mutex> lock(taskMutex);
        taskMsgQueue.push(std::move(taskObj));
        condition.notify_one(); // 通知线程
        LOG(INFO) << "[1] play unlock.";
    }
    else
    {
        setStatus(AUDIO_STATA_STOPED);
    }
}
void AudioManager::pause()
{
    if (waitStata)
    {
        return;
    }

    if (!isStoped)
    {
        waitStata = true;

        LOG(INFO) << "pause.";
        LOG(INFO) << "[2] pause lock.";
        std::unique_ptr<TaskObj> taskObj = std::make_unique<TaskObj>(AUDIO_CTL_PAUSE);
        std::lock_guard<std::mutex> lock(taskMutex);
        taskMsgQueue.push(std::move(taskObj));
        condition.notify_one(); // 通知线程
        LOG(INFO) << "[2] pause unlock.";
    }
}

void AudioManager::resume()
{
    if (waitStata)
    {
        return;
    }

    if (!isStoped)
    {
        waitStata = true;
        LOG(INFO) << "resume.";
        LOG(INFO) << "[3] resume lock.";
        std::unique_ptr<TaskObj> taskObj = std::make_unique<TaskObj>(AUDIO_CTL_RESUME);
        std::lock_guard<std::mutex> lock(taskMutex);
        taskMsgQueue.push(std::move(taskObj));
        condition.notify_one(); // 通知线程
        LOG(INFO) << "[3] resume unlock.";
    }
}
void AudioManager::stop()
{
    if (waitStata)
    {
        return;
    }

    if (!isStoped)
    {
        waitStata = true;

        LOG(INFO) << "stop.";
        LOG(INFO) << "[4] stop lock.";
        std::unique_ptr<TaskObj> taskObj = std::make_unique<TaskObj>(AUDIO_CTL_STOP);
        std::lock_guard<std::mutex> lock(taskMutex);
        taskMsgQueue.push(std::move(taskObj));
        condition.notify_one(); // 通知线程
        LOG(INFO) << "[4] stop unlock.";
    }
}
void AudioManager::decode()
{
    std::unique_ptr<TaskObj> taskObj = std::make_unique<TaskObj>(AUDIO_CTL_DECODE);
    std::lock_guard<std::mutex> lock(taskMutex);
    taskMsgQueue.push(std::move(taskObj));
    condition.notify_one(); // 通知线程
}

void AudioManager::exit()
{
    LOG(INFO) << "exit.";
    LOG(INFO) << "[6] exit lock.";
    std::unique_ptr<TaskObj> taskObj = std::make_unique<TaskObj>(AUDIO_CTL_EXIT);
    std::lock_guard<std::mutex> lock(taskMutex);
    taskMsgQueue.push(std::move(taskObj));
    condition.notify_one(); // 通知线程
    LOG(INFO) << "[6] exit unlock.";
}

void AudioManager::setStatus(AudioEnum stata)
{
    waitStata = false;
    if (stata == AUDIO_STATA_STOPED)
    {
        isStoped = true;
        isPaused = false;
        LOG(INFO) << "Audio STATA is stoped.";
    }
    else if (stata == AUDIO_STATA_PAUSED)
    {
        isPaused = true;
        LOG(INFO) << "Audio STATA is paused.";
    }
    else if (stata == AUDIO_STATA_PLAYED)
    {
        isStoped = false;
        isPaused = false;

        LOG(INFO) << "Audio STATA is played.";
    }

    if (waitPlay && isStoped)
    {
        LOG(INFO) << "Audio STOPED, will play.";
        waitPlay = false;
        play();
    }
}

void AudioManager::loop()
{
    LOG(INFO) << "loop.";
    std::unique_ptr<AudioDecoder> decoder;
    bool exit = false;
    std::unique_ptr<TaskObj> msgObj;
    while (!exit)
    {
        {
            /* code */
            std::unique_lock<std::mutex> lock(taskMutex);
            // 等待任务
            condition.wait(lock, [this]
                           { return !taskMsgQueue.empty(); });
            msgObj = std::move(taskMsgQueue.front());
            taskMsgQueue.pop();
        }

        if (msgObj->obj_msg == AUDIO_CTL_STOP)
        {
            LOG(INFO) << "AUDIO_CTL_STOP";
            decoder = nullptr;
            {
                std::unique_lock<std::mutex> lock(taskMutex);
                // 需要清空队列
                while (!taskMsgQueue.empty())
                {
                    taskMsgQueue.pop();
                }
            }
            this->callback(AUDIO_STATA_STOPED);
        }
        else if (msgObj->obj_msg == AUDIO_CTL_PLAY)
        {
            LOG(INFO) << "AUDIO_CTL_PLAY";
            decoder = nullptr;
            std::unique_ptr<AudioVariant> obj = std::move(msgObj->obj_variant);

            // 使用 std::get_if 来尝试获取 std::unique_ptr<AudioDecoder>
            if (auto audioDecoderPtr = std::get_if<std::unique_ptr<AudioDecoder>>(obj.get()))
            {
                // 在这里，audioDecoderPtr 是一个指向 std::unique_ptr<AudioDecoder> 的指针
                decoder = std::move(*audioDecoderPtr);
                // 你可以通过 *audioDecoderPtr 来访问它指向的 AudioDecoder 对象
                std::cout << "Found std::unique_ptr<AudioDecoder>!" << std::endl;
                // 示例：你可以在这里对 *audioDecoderPtr 进行操作
            }
            else
            {
                std::cout << "std::unique_ptr<AudioDecoder> not found in variant!" << std::endl;
            }
            this->callback(AUDIO_STATA_PLAYED);
            decode();
        }
        else if (msgObj->obj_msg == AUDIO_CTL_DECODE)
        {
            // LOG(INFO) << "AUDIO_CTL_DECODE";
            if (decoder)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                bool ret = decoder->decode();
                if(ret)
                    decode();
                else
                    this->callback(AUDIO_STATA_STOPED);
            }
            else
            {
                LOG(ERROR) << "No decoder";
            }
        }
        else if (msgObj->obj_msg == AUDIO_CTL_PAUSE)
        {
            LOG(INFO) << "AUDIO_CTL_PAUSE";
            if (decoder)
            {
                {
                    std::unique_lock<std::mutex> lock(taskMutex);
                    // 需要清空队列
                    while (!taskMsgQueue.empty())
                    {
                        taskMsgQueue.pop();
                    }
                }
                this->callback(AUDIO_STATA_PAUSED);
                // 暂停渲染。
            }
        }
        else if (msgObj->obj_msg == AUDIO_CTL_RESUME)
        {
            LOG(INFO) << "AUDIO_CTL_RESUME";
            if (decoder)
            {
                this->callback(AUDIO_STATA_PLAYED);
                decode();
            }
        }
        else if (msgObj->obj_msg == AUDIO_CTL_EXIT)
        {
            LOG(INFO) << "AUDIO_CTL_EXIT";
            break;
        }
    }
}