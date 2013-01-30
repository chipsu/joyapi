#include "joyapi.h"

namespace JoyAPI {

int Device::GetNumDevices() {
    return 1;
}

int Device::GetDefaultDeviceId() {
    return GetNumDevices() - 1;
}

Device::Device() {
    memset(&m_data, 0, sizeof(m_data));
    m_data.id = -1;
    m_data.fd = -1;
}

Device::~Device() {
    Close();
}

int Device::GetId() const {
    return m_data.id;
}

const char *Device::GetName() const {
    return m_data.name;
}

int Device::GetNumAxes() const {
    return m_data.axes;
}

int Device::GetNumButtons() const {
    return m_data.buttons;
}

bool Device::IsOpen() const {
    return m_data.fd != -1;
}

bool Device::Open(int id) {
    if(IsOpen()) {
        fprintf(stderr, "Device is already open!\n");
        return false;
    }
    if(id < 0) {
        id = GetDefaultDeviceId();
    }
    if(id < 0) {
        fprintf(stderr, "Could not find any devices!\n");
        return false;
    }
    m_data.fd = OpenDevice(id);
    if(m_data.fd == -1) {
        fprintf(stderr, "Could not open device %d!\n", id);
        return false;
    }
    m_data.id = id;
    ioctl(m_data.fd, JSIOCGAXES, &m_data.axes);
    ioctl(m_data.fd, JSIOCGBUTTONS, &m_data.buttons);
    ioctl(m_data.fd, JSIOCGNAME(255), &m_data.name);
    OnOpened();
    printf("Device %d (%s) opened!\n", id, m_data.name);
    return true;
}

bool Device::ReOpen(int id) {
    if(IsOpen()) {
        Close();
    }
    return Open(id);
}

void Device::Close() {
    if(m_data.fd != -1) {
        close(m_data.fd);
        OnClosed();
        printf("Device %d closed!\n", m_data.id);
        m_data.fd = -1;
        m_data.id = -1;
        m_data.name[0] = 0;
    }
}

bool Device::Poll() {
    js_event js;

    if(!IsOpen()) {
        fprintf(stderr, "Device is not open!\n");
        return false;
    }

    int result = read(m_data.fd, &js, sizeof(js));

    if(result == -1) {
        return true;
    }

    if(result != sizeof(js_event)) {
        fprintf(stderr, "Read error %d\n", result);
        return false;
    }

    Event::Type type;
    
    switch(js.type & ~JS_EVENT_INIT) {
    case JS_EVENT_AXIS:
        if(js.number < 0 || js.number >= m_data.axes) {
            fprintf(stderr, "Out of range number: %d\n", js.number);
            return false;
        }
        type = Event::EVENT_AXIS;
        break;
    case JS_EVENT_BUTTON:
        if(js.number < 0 || js.number >= m_data.buttons) {
            fprintf(stderr, "Out of range number: %d\n", js.number);
            return false;
        }
        type = Event::EVENT_BUTTON;
        break;
    default:
        // FIXME: Other events?
        fprintf(stderr, "Unknown event 0x%x", js.type);
        return true;
    }
    
    Event e;
    e.type = type;
    e.time = js.time;
    e.value = js.value;
    e.number = js.number;

    OnEvent(e);

    return true;
}

int Device::OpenDevice(int id) {
    char buffer[1024];
    sprintf(buffer, "/dev/input/js%d", id);
    int result = open(buffer, O_RDONLY | O_NONBLOCK);
    if(result == -1) {
        sprintf(buffer, "/dev/js%d", id);
        result = open(buffer, O_RDONLY | O_NONBLOCK);
    }
    return result;
}

SimpleDevice::SimpleDevice() {
    m_axes = NULL;
    m_buttons = NULL;
}

SimpleDevice::~SimpleDevice() {
}

int SimpleDevice::GetAxis(int id) const {
    if(id < 0 || id > m_data.axes) {
        return -1;
    }
    return m_axes[id];
}

int SimpleDevice::GetButton(int id) const {
    if(id < 0 || id > m_data.buttons) {
        return -1;
    }
    return m_buttons[id] != 0;
}

void SimpleDevice::OnOpened() {
    m_axes = new int[m_data.axes];
    m_buttons = new int[m_data.buttons];
}

void SimpleDevice::OnClosed() {
    delete m_axes;
    delete m_buttons;
}

void SimpleDevice::OnEvent(const Event &e) {
    switch(e.type) {
    case Event::EVENT_AXIS:
        m_axes[e.number] = e.value;
        break;
    case Event::EVENT_BUTTON:
        m_buttons[e.number] = e.value;
        break;
    }
}

}
