#include "device.h"

LED& led()
{
    static LED instance;
    return instance;
}

LED::LED(): _enabled(false), _color(Red), _rate(0)
{
}

bool LED::isEnabled() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _enabled;
}

LED::Color LED::color() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _color;
}

int LED::rate() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _rate;
}

void LED::setEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _enabled = enabled;
}

void LED::setColor(LED::Color color)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _color = color;
}

void LED::setRate(int rate)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _rate = rate;
}

std::string to_string(const LED::Color &color)
{
    switch(color) {
    case LED::Color::Red:
        return "red";
    case LED::Color::Green:
        return "green";
    case LED::Color::Blue:
        return "blue";
    default:
        return "";
    }
}

LED::Color from_string(const std::string &str)
{
    return  (str == "red")? LED::Color::Red:
            (str == "green")? LED::Color::Green:
            (str == "blue")? LED::Color::Blue:
                             LED::Color::Invalid;
}

typedef const std::vector<std::string>& Args;
const std::map<std::string, std::string(*)(Args)> commands = {
{
        "get-led-state",
        [](Args args) { return std::string((led().isEnabled()? "OK on": "OK off")); }
},
{
        "get-led-color",
        [](Args args) { return std::string("OK " + to_string(led().color())); }
},
{
        "get-led-rate",
        [](Args args) { return std::string("OK " + std::to_string(led().rate())); }
},
{
        "set-led-state",
        [](Args args) {
            if (args.size() < 1 || (args.front() != "on" && args.front() != "off"))
                return std::string("FAILED");
            else {
                led().setEnabled(args.front() == "on");
                return std::string("OK");
            }
        }
},
{
        "set-led-color",
        [](Args args) {
            LED::Color c;
            if (args.size() < 1 || (c = from_string(args.front())) == LED::Color::Invalid)
                return std::string("FAILED");
            else {
                led().setColor(c);
                return std::string("OK");
            }
        }
},
{
        "set-led-rate",
        [](Args args) {
            if (args.size() < 1 || args.front() < "0" || args.front() > "5")
                return std::string("FAILED");
            else {
                int rate = atoi(args.front().c_str());
                led().setRate(rate);
                return std::string("OK");
            }
        }
}
};
