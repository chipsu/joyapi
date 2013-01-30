#ifndef _JOYAPI_H_
#define _JOYAPI_H_

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <malloc.h>
#include <memory.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/joystick.h>

#ifndef JOYAPI_MODE_FF
#	define JOYAPI_MODE_FF 1
#endif

namespace JoyAPI {

/**
 * Simple Linux Joystick Example
 *  
 * @todo GetNumDevices() 
 * @todo Device disconnects?
 * @todo EnumDevices() (with names and caps)
 */
struct Device {
    
    struct Event {
        enum Type {
            EVENT_AXIS,
            EVENT_BUTTON,
        };
        uint32_t time;
        int16_t value;
        Type type;
        uint8_t number;
    };
    
    static int GetNumDevices();
    static int GetDefaultDeviceId();
    Device();
    virtual ~Device();
    int GetId() const;
    const char *GetName() const;
    int GetNumAxes() const;
    int GetNumButtons() const;
    bool IsOpen() const;
    bool Open(int id = -1);
    bool ReOpen(int id = -1);
    void Close();
    bool Poll();

protected:
    
    virtual void OnOpened() {}
    virtual void OnClosed() {}
    virtual void OnEvent(const Event &e) {}
    
protected:
    static int OpenDevice(int id);

    struct Data {
        int fd;
        int id;
        char name[255];
        int buttons;
        int axes;
    };
    
    Data m_data;
};

struct SimpleDevice : public Device {
    SimpleDevice();
    virtual ~SimpleDevice();
    int GetAxis(int id) const;
    int GetButton(int id) const;

protected:
    virtual void OnOpened();
    virtual void OnClosed();
    virtual void OnEvent(const Event &e);
    
private:
    int *m_axes;
    int *m_buttons;
};

} // Namespace JoyAPI

#endif

