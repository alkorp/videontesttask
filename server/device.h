#ifndef DEVICE_H
#define DEVICE_H

#include <mutex>
#include <vector>
#include <map>
#include <string>

/**
 * @brief The LED class
 * All methods are thread-safe
 */
class LED {
public:
    enum Color
    {
        Red,
        Green,
        Blue,
        Invalid
    };

    bool isEnabled() const;
    Color color() const;
    int rate() const;

    void setEnabled(bool enabled = true);
    void setColor(Color color);
    void setRate(int rate);
private:
    LED();
    bool _enabled;
    Color _color;
    int _rate;
    mutable std::mutex _mutex;
    friend LED& led();
};

std::string to_string(const LED::Color& color);
LED::Color from_string(const std::string& str);

LED& led();

extern const std::map<std::string, std::string(*)(const std::vector<std::string>&)> commands;

#endif // DEVICE_H
