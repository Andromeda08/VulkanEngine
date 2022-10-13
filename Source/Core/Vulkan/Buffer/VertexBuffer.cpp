#include "VertexBuffer.hpp"

VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices,
                           const CommandBuffers& cmd_buffers,
                           const Device& device)
: m_device(device)
, m_vertices(vertices)
{
    vk::DeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

    auto staging = Buffer::make_staging_buffer(buffer_size, m_device);

    void* data;
    vkMapMemory(m_device.handle(), staging.memory(), 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), (size_t) buffer_size);
    vkUnmapMemory(m_device.handle(), staging.memory());

    m_buffer = std::make_unique<Buffer>(buffer_size,
                                        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                                        vk::MemoryPropertyFlagBits::eDeviceLocal,
                                        m_device);

    Buffer::copy_buffer(cmd_buffers, m_buffer->handle(), staging.handle(), buffer_size);

    m_device.handle().destroyBuffer(staging.handle(), nullptr);
    m_device.handle().freeMemory(staging.memory(), nullptr);
}