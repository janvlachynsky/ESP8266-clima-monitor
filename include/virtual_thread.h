#include <functional>

class SimpleVirtualThread {
public:
    SimpleVirtualThread(uint32_t parentLoopInterval = 1000, uint32_t loopInterval = 1000, bool initRun = false, std::function<void()> func = nullptr)
        : parentLoopInterval(parentLoopInterval)
        , loopInterval(loopInterval)
        , initRun(initRun)
        , func(func) {}

    bool run() {
        auto timeFromLastRun = parentLoopInterval * parentLoopCount;
        if ((initRun && timeFromLastRun == 0) || timeFromLastRun >= loopInterval) {
            // run func
            if (func != nullptr) {
                func();
                parentLoopCount = 0;
                parentLoopCount++;
                return true;
            }
        }
        parentLoopCount++;
        return false;
    }

    void reset() {
        parentLoopCount = 0;
    }

private:
    uint32_t parentLoopCount = 0;
    uint32_t parentLoopInterval;
    uint32_t loopInterval;
    // If true, run func at the beginning (with parentLoopCount == 0)
    bool initRun;
    std::function<void()> func;
};
