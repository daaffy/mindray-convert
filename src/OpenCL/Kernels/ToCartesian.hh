#ifndef OPENCL_TOCARTESIAN_HH
#define OPENCL_TOCARTESIAN_HH

#include <cmath>
#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Filter.hh"
#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"
#include "../../GUI/Tree.hh"

namespace opencl
{
    class ToCartesian : public Filter
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;

    public:
        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        ToCartesian(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr) : kernel(ptr), context(c), queue(q)
        {
            Filter::volume = std::make_shared<data::Volume>();
            Filter::input = std::bind(input, this, std::placeholders::_1);
            Filter::execute = std::bind(execute, this);
            Filter::getOptions = std::bind(getOptions, this);
        }

        ~ToCartesian() = default;

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

            // depth = static_cast<cl_uint>(static_cast<float>(v->depth) - static_cast<float>(v->depth) * v->ratio / (v->ratio + 1.0f));
            // length = static_cast<cl_uint>((static_cast<float>(v->length) - 1.0f) / (2.0f * std::tan(v->delta / 2.0f) * static_cast<float>(v->depth)));
            // width = inwidth > 1 ? static_cast<cl_uint>((static_cast<float>(v->width) - 1.0f) / (2.0f * std::tan(v->delta / 2.0f) * static_cast<float>(v->depth))) : inwidth;

            volume->depth = indepth;
            volume->width = inwidth;
            volume->length = inlength;

            // std::cout << indepth << '=' << v->delta << '\n'
            //           << inlength << '=' << length << '\n'
            //           << inwidth << '=' << width << std::endl;

            volume->buffer = cl::Buffer(context, CL_MEM_READ_WRITE, volume->length * volume->depth * volume->width * sizeof(cl_uint));
        }

        void execute()
        {
            kernel->setArg(0, indepth);
            kernel->setArg(1, inlength);
            kernel->setArg(2, inwidth);
            kernel->setArg(3, inBuffer);
            kernel->setArg(4, volume->depth);
            kernel->setArg(5, volume->length);
            kernel->setArg(6, volume->width);
            kernel->setArg(7, volume->buffer);
            kernel->setArg(8, volume->ratio);
            kernel->setArg(9, volume->delta);

            kernel->global = cl::NDRange(volume->depth, volume->length, volume->width);
            kernel->execute(queue);
        }
        std::shared_ptr<gui::Tree> getOptions();
    };

} // namespace opencl

#endif