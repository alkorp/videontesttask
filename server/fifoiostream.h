#ifndef FIFOIOSTREAM_H
#define FIFOIOSTREAM_H

#include <streambuf>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

#include <cstring>
#include <stdexcept>

class FifoOStreamBuf : public std::streambuf
{
public:
    FifoOStreamBuf(const std::string &name, int timeout, int ctrlFd = -1):
        _timeout(timeout),
        _fd(-1),
        _ctrlFd(ctrlFd)
    {
        setp (_buffer, _buffer + (bufferSize - 1));
        for (int attempt = 0; attempt < timeout; ++attempt) {
            _fd = ::open(name.c_str(), O_WRONLY | O_NONBLOCK);
            if (_fd != -1 || errno != ENXIO) {
                break;
            }
            sleep(1);
        }
        if (_fd == -1) {
            throw std::runtime_error("Failed to open " + name);
        }
    }
    virtual ~FifoOStreamBuf()
    {
        sync();
        ::close(_fd);
    }
protected:
    virtual int_type overflow(int_type c)
    {
        if (c != EOF) {
            *pptr() = c;
            pbump(1);
        }
        if (flushBuffer() == EOF) {
            return EOF;
        }
        return c;
    }
    virtual int sync()
    {
        if (flushBuffer() == EOF) {
            return -1;
        }
        return 0;
    }
private:
    int flushBuffer()
    {
        int num = pptr() - pbase();
        struct pollfd pfd[2] = { { _fd, POLLOUT, 0}, { _ctrlFd, POLLIN, 0} };
        int r = ::poll(pfd, 2, _timeout);
        if (r > 0 && pfd[1].revents & POLLIN) {
            throw std::runtime_error("Aborted");
        }
        if (r > 0 && pfd[0].revents & POLLOUT) {
            if (::write (_fd, _buffer, num) != num) {
                return EOF;
            }
        }
        pbump(-num);
        return num;
    }
    void sleep(int seconds)
    {
        if (_ctrlFd != -1) {
            struct pollfd pfd = { _fd, POLLIN, 0};
            if (::poll(&pfd, 1, seconds*1000)) {
                throw std::runtime_error("Aborted");
            }
        } else {
            ::sleep(seconds);
        }
    }

    static const int bufferSize = 512;
    char _buffer[bufferSize];
    int _timeout;
    int _fd;
    int _ctrlFd;
};

class FifoIStreamBuf : public std::streambuf
{
public:
    FifoIStreamBuf(const std::string &name, int timeout, int ctrlFd = -1):
        _timeout(timeout),
        _fd(-1),
        _ctrlFd(ctrlFd)
    {
        setg(_buffer + ungetSize, _buffer + ungetSize, _buffer + ungetSize);
        _fd = ::open(name.c_str(), O_RDONLY | O_NONBLOCK);
        if (_fd == -1) {
            throw std::runtime_error("Failed to open " + name);
        }
    }
    virtual ~FifoIStreamBuf()
    {
        ::close(_fd);
    }
protected:
    virtual int_type underflow()
    {
        if (gptr() < egptr()) {
            return traits_type::to_int_type(*gptr());
        }
        int numPutback;
        numPutback = gptr() - eback();
        if (numPutback > ungetSize) {
            numPutback = ungetSize;
        }
        std::memmove(_buffer + (ungetSize - numPutback), gptr() - numPutback, numPutback);

        // read new characters
        int num;
        struct pollfd pfd[2] = { { _fd, POLLIN | POLLHUP, 0}, { _ctrlFd, POLLIN, 0} };
        int r = ::poll(pfd, 2, 1000*_timeout);
        if (r > 0 && pfd[1].revents & POLLIN) {
            throw std::runtime_error("Aborted");
        }
        if (r > 0 && pfd[0].revents & POLLIN) {
            num = ::read (_fd, _buffer + ungetSize, bufferSize - ungetSize);
            if (num <= 0) {
                return EOF; // ERROR or EOF
            }
        } else {
            return EOF;
        }

        // reset buffer pointers
        setg (_buffer + (ungetSize - numPutback), _buffer + ungetSize, _buffer + ungetSize + num);
        // return next character
        return traits_type::to_int_type(*gptr());
    }
private:
    static const int bufferSize = 512;
    static const int ungetSize = 4;
    char _buffer[bufferSize];
    int _timeout;
    int _fd;
    int _ctrlFd;
};

#endif // FIFOIOSTREAM_H
