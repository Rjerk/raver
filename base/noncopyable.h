#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

namespace raver {

class noncopyable {
protected:
    noncopyable() = default;
    ~noncopyable() = default;

private:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

}

#endif
