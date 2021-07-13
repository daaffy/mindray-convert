#include "Volume.hh"

#include <numbers>

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Events/GUI.hh"

namespace data
{

    Volume::Volume(unsigned int d, unsigned int l, unsigned int w, const std::vector<uint8_t> &data) : depth(d), length(l), width(w)
    {
        raw.reserve(width * depth * length);
        for (unsigned int z = 0; z < width; ++z)
        {
            auto zyx = z * length * depth;
            for (unsigned int y = 0; y < length; ++y)
            {
                auto yx = y * depth;
                for (unsigned int x = 0; x < depth; ++x)
                {
                    cl_uchar bnw = data.at(x + yx + zyx);
                    cl_uchar4 arr = {bnw, bnw, bnw, 0xFF};
                    raw.push_back(arr);
                }
            }
        }

        // eventManager.addCallback(SDL_MOUSEWHEEL, events::scaleEvent<Volume>, *this);
        // eventManager.addCallback(SDL_MOUSEMOTION, events::rotateEvent<Volume>, *this);
    }

    Volume::~Volume()
    {
    }

    void Volume::sendToCl(const cl::Context &context)
    {
        buffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(raw[0]) * raw.size(), raw.data());
    }

    void Volume::update()
    {
        if (!modified)
        {
            return;
        }

        float maxEdge = static_cast<float>(std::max(std::max(depth, length), std::max(depth, width)));

        glm::mat4 id(1.0f);

        // MODEL
        id = glm::scale(id, {maxEdge / static_cast<float>(depth), maxEdge / static_cast<float>(length), maxEdge / static_cast<float>(width)});
        id = glm::scale(id, scale);
        id = glm::rotate(id, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        id = glm::translate(id, {translation.x, translation.y, translation.z});

        // VIEW
        id = glm::translate(id, {0.0f, 0.0f, 2.0f});
        id = glm::rotate(id, glm::radians(rotation.x), {1.0f, 0.0f, 0.0f});
        id = glm::rotate(id, glm::radians(rotation.y), {0.0f, 1.0f, 0.0f});

        GLfloat *modelView = glm::value_ptr(id);

        invMVTransposed[0] = modelView[0];
        invMVTransposed[1] = modelView[4];
        invMVTransposed[2] = modelView[8];
        invMVTransposed[3] = modelView[12];
        invMVTransposed[4] = modelView[1];
        invMVTransposed[5] = modelView[5];
        invMVTransposed[6] = modelView[9];
        invMVTransposed[7] = modelView[13];
        invMVTransposed[8] = modelView[2];
        invMVTransposed[9] = modelView[6];
        invMVTransposed[10] = modelView[10];
        invMVTransposed[11] = modelView[14];
    }

} // namespace data