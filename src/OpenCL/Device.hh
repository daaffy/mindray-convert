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

public:
    cl::Context context;
    cl_device_type type = CL_DEVICE_TYPE_CPU;
    
    std::map<std::string, Program> programs;

    Device();
    ~Device();
    

    void render(float *inv, cl_GLuint glPixelBuffer = 0);
    void createDisplay(unsigned int w, unsigned int h, cl_GLuint glPixelBuffer = 0);
    void prepareVolume(unsigned int d, unsigned int l, unsigned int w, const cl::Buffer &v);

    void selectDevice();


};

#endif