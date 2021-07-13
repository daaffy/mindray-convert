#ifndef GUI_OPENCL_DEVICE
#define GUI_OPENCL_DEVICE

#include <map>
#include <string>

#include <CL/cl2.hpp>

#include "Program.hh"

class Device
{
private:
    cl::Platform platform;
    cl::Device device;
    cl::Buffer   outBuffer;
    cl::Buffer invMVTransposed;
    cl::CommandQueue cQueue;

    unsigned int width = 0;
    unsigned int height = 0;

public:
    cl_GLuint pixelBuffer;

    std::map<std::string, Program> programs;
    cl::Context context;
    cl_device_type type = CL_DEVICE_TYPE_CPU;

    Device(unsigned int width = 512, unsigned int height = 512);
    ~Device();

    void render(float *inv);
    void createDisplay(unsigned int w, unsigned int h);
    void prepareVolume(unsigned int d, unsigned int l, unsigned int w, const cl::Buffer &v);

    void selectDevice();


};

#endif