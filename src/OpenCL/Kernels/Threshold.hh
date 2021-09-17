#ifndef OPENCL_KERNELS_THRESHOLD_HH
#define OPENCL_KERNELS_THRESHOLD_HH

#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Filter.hh"
#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"
#include "../../GUI/Tree.hh"
#include "../../GUI/Slider.hh"

namespace opencl
{
    class Threshold : public Filter
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;
        std::shared_ptr<gui::Slider> thresholdSlider;

    public:
        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        Threshold(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
        {
            Filter::volume = std::make_shared<data::Volume>();
            Filter::input = std::bind(input, this, std::placeholders::_1);
            Filter::execute = std::bind(execute, this);
            Filter::getOptions = std::bind(getOptions, this);
            
            thresholdSlider = gui::Slider::build(0.0f, 0.0f, 0.0f, 10.0f);
        }

        Threshold(const Threshold &th)
        {
            kernel = th.kernel;
            context = th.context;
            queue = th.queue;
            kernel = th.kernel;

            Filter::volume = std::make_shared<data::Volume>();
            Filter::input = std::bind(input, this, std::placeholders::_1);
            Filter::execute = std::bind(execute, this);
            Filter::getOptions = std::bind(getOptions, this);

            thresholdSlider = gui::Slider::build(0.0f, 0.0f, 0.0f, 10.0f);
        }

        ~Threshold() = default;

        void input(const std::weak_ptr<data::Volume> &wv)
        {
            auto v = wv.lock();
            if (!v)
                return;

            volume->min = v->min;
            volume->max = v->max;
            inlength = v->length;
            inwidth = v->width;
            indepth = v->depth;
            inBuffer = v->buffer;
            volume->ratio = v->ratio;
            volume->delta = v->delta;

            volume->length = inlength;
            volume->width = inwidth;
            volume->depth = indepth;
            volume->buffer = cl::Buffer(context, CL_MEM_READ_WRITE, volume->length * volume->depth * volume->width * sizeof(cl_uint));
        }

        void execute()
        {
            kernel->setArg(0, indepth);
            kernel->setArg(1, inlength);
            kernel->setArg(2, inwidth);
            kernel->setArg(3, inBuffer);
            kernel->setArg(4, volume->buffer);
            kernel->setArg(5, static_cast<cl_uchar>(thresholdSlider->value*255.0f));

            kernel->global = cl::NDRange(volume->depth, volume->length, volume->width);
            kernel->execute(queue);
        }
        std::shared_ptr<gui::Tree> getOptions();
    };

} // namespace opencl

#endif